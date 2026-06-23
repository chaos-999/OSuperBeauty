#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "platform.h"
#include "lock/lock.h"  // 假设锁的定义在这里
#include "proc/proc.h"
#include "defs.h"
#include "lib/elf.h"
#include "fs/vfs/inode.h"
#include "fs/vfs/ops.h"
#include "errno.h"

int flags2perm(int flags) {
#ifdef RISCV
    int perm = 0;
    if (flags & 0x1) perm = PTE_X;
    if (flags & 0x2) perm |= PTE_W;
    return perm;
#elif defined(LOONGARCH)
    int perm = PTE_P | PTE_PLV | PTE_MAT | PTE_D;
    if ((flags & 0x1) == 0) {
        perm |= PTE_NX;
    }
    if (flags & 0x2) {
        perm |= PTE_W;
    }
    return perm;
#endif
}

#define AUXV(id, val)       \
    do {                    \
        aux[index++] = id;  \
        aux[index++] = val; \
    } while (0);

static int loadseg(pde_t *, uint64, struct inode *, uint, uint);

#ifdef RISCV
int execve(char *path, char **argv, char **envp) {
    char *s, *last;
    int i, off, ret = -1;
    uint64 envc, argc, sz = 0, sp, ustack[MAXARG], estack[MAXENV], stackbase;
    struct elfhdr elf;
    struct inode *ip;
    struct proghdr ph;
    pagetable_t pagetable = 0, oldpagetable;
    struct proc *p = myproc();
    int has_setuid = 0, has_setgid = 0;
    uint file_uid = 0, file_gid = 0;

    // printf("VF2DBG: execve path=%s\n", path);

    if ((ip = namei(path)) == 0) {
        // printf("VF2DBG: execve namei failed for %s\n", path);
        return -ENOENT;
    }
    ip->i_op->lock(ip);

    // 检查文件的setuid/setgid位
    if (ip->i_mode & S_ISUID) {
        has_setuid = 1;
        file_uid = ip->i_uid;
    }
    if (ip->i_mode & S_ISGID) {
        has_setgid = 1;
        file_gid = ip->i_gid;
    }

    // Check ELF header
    if (ip->i_op->read(ip, 0, (uint64)&elf, 0, sizeof(elf)) != sizeof(elf)) goto bad;

    if (elf.magic != ELF_MAGIC) {
        // printf("VF2DBG: execve bad ELF magic for %s (magic=0x%x)\n", path, elf.magic);
        ret = -ENOEXEC;  // let musl/glibc know it's a script, not an ELF
        goto bad;
    }

    if ((pagetable = proc_pagetable(p)) == 0) goto bad;

    // printf("execve: Loading %s, ELF entry point: 0x%lx\n", path, elf.entry);

    // Load program into memory.
    uint64 first_load_vaddr = 0;
    for (i = 0, off = elf.phoff; i < elf.phnum; i++, off += sizeof(ph)) {
        if (ip->i_op->read(ip, 0, (uint64)&ph, off, sizeof(ph)) != sizeof(ph)) goto bad;
        if (ph.type != ELF_PROG_LOAD) continue;
        if (first_load_vaddr == 0 || ph.vaddr < first_load_vaddr) first_load_vaddr = ph.vaddr;
        if (ph.memsz < ph.filesz) goto bad;
        if (ph.vaddr + ph.memsz < ph.vaddr) goto bad;
        uint64 sz1;
        if ((sz1 = uvmalloc(pagetable, sz, ph.vaddr + ph.memsz, PTE_W | PTE_X | PTE_R | PTE_U)) ==
            0)
            goto bad;
        sz = sz1;

        uint margin_size = 0;
        if ((ph.vaddr % PGSIZE) != 0) {
            margin_size = ph.vaddr % PGSIZE;
        }

        if (loadseg(pagetable, PGROUNDDOWN(ph.vaddr), ip, PGROUNDDOWN(ph.off),
                    ph.filesz + margin_size) < 0) {
            // printf("VF2DBG: execve loadseg failed vaddr=0x%lx off=0x%lx filesz=0x%lx\n",
            // ph.vaddr, ph.off, ph.filesz);
            goto bad;
        }
    }
    ip->i_op->unlock(ip);
    ip = 0;

    p = myproc();
    uint64 oldsz = p->sz;

    // Allocate stack at fixed high address (separate from program segments)
    // Use USTACK definitions from memlayout.h for separated layout
    sz = PGROUNDUP(sz);  // Only program segments, no stack
    uint64 sz1;

    // Allocate stack at fixed high address
    if ((sz1 = uvmalloc(pagetable, USTACK, USTACK_TOP, PTE_W | PTE_R | PTE_U)) == 0) goto bad;
    uvmclear(pagetable, USTACK);  // Guard page

    sp = USTACK_TOP;
    stackbase = USTACK;
    sp -= sizeof(uint64);

    for (envc = 0; envp[envc]; envc++) {
        if (envc >= MAXENV) {
            goto bad;
        }
        sp -= strlen(envp[envc]) + 1;
        sp -= sp % 16;
        if (sp < stackbase) {
            goto bad;
        }
        if (copyout(pagetable, sp, envp[envc], strlen(envp[envc]) + 1) < 0) {
            goto bad;
        }
        estack[envc] = sp;
    }
    estack[envc] = 0;

    // Push argument strings, prepare rest of stack in ustack.
    for (argc = 0; argv[argc]; argc++) {
        if (argc >= MAXARG) goto bad;
        sp -= strlen(argv[argc]) + 1;
        sp -= sp % 16;  // riscv sp must be 16-byte aligned
        if (sp < stackbase) goto bad;
        if (copyout(pagetable, sp, argv[argc], strlen(argv[argc]) + 1) < 0) goto bad;
        ustack[argc] = sp;
    }
    ustack[argc] = 0;

    // Build auxv first (we will lay out as: argc | argv[] | envp[] | auxv[])
    uint64 aux[MAX_AT * 2];
    int index = 0;
    AUXV(AT_HWCAP, 0);
    AUXV(AT_PAGESZ, PGSIZE);
    AUXV(AT_PHDR, first_load_vaddr + elf.phoff);
    AUXV(AT_PHENT, elf.phentsize);
    AUXV(AT_PHNUM, elf.phnum);
    AUXV(AT_BASE, 0);
    AUXV(AT_ENTRY, elf.entry);
    AUXV(AT_UID, p->uid);
    AUXV(AT_EUID, has_setuid ? file_uid : p->euid);
    AUXV(AT_GID, p->gid);
    AUXV(AT_EGID, has_setgid ? file_gid : p->egid);
    AUXV(AT_SECURE, (has_setuid || has_setgid) ? 1 : 0);
    AUXV(AT_RANDOM, sp);
    AUXV(AT_NULL, 0);

    uint64 aux_bytes = (uint64)index * sizeof(uint64);
    uint64 argv_bytes = (argc + 1) * sizeof(uint64);
    uint64 env_bytes = (envc + 1) * sizeof(uint64);

    // Keep stack 16-byte aligned before laying out pointer tables
    sp -= sp % 16;

    // Place auxv (highest among the tables)
    sp -= aux_bytes;
    if (sp < stackbase) goto bad;
    if (copyout(pagetable, sp, (char *)aux, aux_bytes) < 0) goto bad;

    // Place envp[] just below auxv
    sp -= env_bytes;
    if (sp < stackbase) goto bad;
    if (copyout(pagetable, sp, (char *)estack, env_bytes) < 0) goto bad;
    uint64 envp_ptr = sp;

    // Place argv[] below envp[]
    sp -= argv_bytes;
    if (sp < stackbase) goto bad;
    if (copyout(pagetable, sp, (char *)ustack, argv_bytes) < 0) goto bad;
    uint64 argv_ptr = sp;

    // Finally place argc at the bottom
    sp -= sizeof(uint64);
    if (sp < stackbase) goto bad;
    if (copyout(pagetable, sp, (char *)&argc, sizeof(uint64)) < 0) goto bad;

    // Set registers to conventional values (argv/envp pointers)
    p->trapframe->a1 = argv_ptr;
    p->trapframe->a2 = envp_ptr;

    // Save program name for debugging.
    for (last = s = path; *s; s++)
        if (*s == '/') last = s + 1;
    safestrcpy(p->name, last, sizeof(p->name));

    // 获取进程锁
    acquire(&p->lock);

    // 处理setuid/setgid
    if (has_setuid) {
        p->euid = file_uid;
        p->suid = file_uid;
    }
    if (has_setgid) {
        p->egid = file_gid;
        p->sgid = file_gid;
    }

    // Commit to the user image.
    oldpagetable = p->pagetable;
    p->pagetable = pagetable;
    p->sz = sz;                     // Process size only includes program segments, not stack
    p->trapframe->epc = elf.entry;  // initial program counter = main
    p->trapframe->sp = sp;          // initial stack pointer
    safestrcpy(p->exec_path, path, MAXPATH);  // for /proc/self/exe

    release(&p->lock);

    proc_freepagetable(oldpagetable, oldsz);
    return 0;  // this ends up in a0, the first argument to main(argc, argv)

bad:
    // printf("VF2DBG: execve failed for %s\n", path);
    return ret;
}
#endif

#ifdef LOONGARCH
// For loognarch
uint64 user_push(pagetable_t pt, uint64 *ustack, char *str, uint64 sp) {
    uint64 argc = ++ustack[0];
    if (argc > MAXARG + 1) {
        return -1;
    }
    sp -= strlen(str) + 1;
    sp -= sp % 16;  // riscv sp must be 16-byte aligned

    if (copyout(pt, sp, str, strlen(str) + 1) < 0) {
        printf("copyout failed\n");
        return -1;
    }
    ustack[argc] = sp;

    ustack[argc + 1] = 0;
    return sp;
}

void alloc_aux(uint64 *aux, uint64 atid, uint64 value) {
    // printf("aux[%d] = %p\n",atid,value);
    uint64 argc = aux[0];
    aux[argc * 2 + 1] = atid;
    aux[argc * 2 + 2] = value;
    aux[argc * 2 + 3] = 0;
    aux[argc * 2 + 4] = 0;
    aux[0]++;
}

uint64 loadaux(pagetable_t pagetable, uint64 sp, uint64 *aux) {
    int argc = aux[0];
    if (!argc) return sp;
    sp -= (2 * argc + 2) * sizeof(uint64);
    aux[0] = 0;
    if (copyout(pagetable, sp, (char *)(aux + 1), (2 * argc + 2) * sizeof(uint64)) < 0) {
        return -1;
    }
    return sp;
}

enum redir {
    REDIR_OUT,
    REDIR_APPEND,
};

__attribute__((aligned(64)))
int execve(char *path, char **argv, char **envp) {
    char *s, *last;
    int i, off;
    uint64 argc, envc, sz = 0, sp, ustack[MAXARG], estack[MAXENV], stackbase;
    struct elfhdr elf;
    struct inode *ip;
    struct proghdr ph;
    pagetable_t pagetable = 0, oldpagetable;
    struct proc *p = myproc();
    int has_setuid = 0, has_setgid = 0;
    uint file_uid = 0, file_gid = 0;

    if ((ip = namei(path)) == 0) {
        printf("namei failed\n");
        return -1;
    }
    ip->i_op->lock(ip);

    // 检查文件的setuid/setgid位
    if (ip->i_mode & S_ISUID) {
        has_setuid = 1;
        file_uid = ip->i_uid;
    }
    if (ip->i_mode & S_ISGID) {
        has_setgid = 1;
        file_gid = ip->i_gid;
    }

    if (ip->i_op->read(ip, 0, (uint64)&elf, 0, sizeof(elf)) != sizeof(elf)) goto bad;
    if (elf.magic != ELF_MAGIC) goto bad;
    if ((pagetable = proc_pagetable(p)) == 0) goto bad;

    uint64 start_vaddr = 0;
    char interp_buf[MAXPATH] = {0};
    uint64 interp_off = 0, interp_len = 0;

    for (i = 0, off = elf.phoff; i < elf.phnum; i++, off += sizeof(ph)) {
        if (ip->i_op->read(ip, 0, (uint64)&ph, off, sizeof(ph)) != sizeof(ph)) goto bad;

        if (ph.type == ELF_PROG_INTERP) {
            if (ph.filesz < MAXPATH) {
                interp_off = ph.off;
                interp_len = ph.filesz;
            }
            continue;
        }

        if (ph.type == ELF_PROG_LOAD) {
            if (ph.memsz < ph.filesz) goto bad;
            if (ph.vaddr + ph.memsz < ph.vaddr) goto bad;
            if (start_vaddr == 0) start_vaddr = ph.vaddr;

            uint64 sz1;
#ifdef LA2K1000
            if ((sz1 = uvmalloc(pagetable, PGROUNDDOWN(start_vaddr), PGROUNDUP(ph.vaddr + ph.memsz),
                                PTE_P | PTE_W | PTE_PLV | PTE_MAT | PTE_D | PTE_U)) == 0)
#else
            if ((sz1 = uvmalloc(pagetable, PGROUNDDOWN(start_vaddr), PGROUNDUP(ph.vaddr + ph.memsz),
                                PTE_P | PTE_W | PTE_PLV | PTE_MAT | PTE_D)) == 0)
#endif
                goto bad;
            start_vaddr = PGROUNDUP(ph.vaddr + ph.memsz);
            sz = sz1;

            uint margin_size = 0;
            if ((ph.vaddr % PGSIZE) != 0) margin_size = ph.vaddr % PGSIZE;

            if (loadseg(pagetable, PGROUNDDOWN(ph.vaddr), ip, PGROUNDDOWN(ph.off),
                        ph.filesz + margin_size) < 0)
                goto bad;
        }
    }

    // Read PT_INTERP path if present
    if (interp_len > 0) {
        if (ip->i_op->read(ip, 0, (uint64)interp_buf, interp_off, interp_len) != (int)interp_len)
            goto bad;
        interp_buf[interp_len] = '\0';
    }

    ip->i_op->unlock(ip);
    ip = 0;

    // ---- Load dynamic linker (PT_INTERP) ----
    uint64 interp_base = 0;
    uint64 interp_entry = elf.entry;
    int has_interp = 1;  // force

    if (has_interp) {
        struct inode *lip = namei(interp_buf);

        // Try fallback path: /glibc/lib/<basename>
        if (lip == 0) {
            char *bn = interp_buf;
            for (char *p = interp_buf; *p; p++)
                if (*p == '/') bn = p + 1;
            char alt[MAXPATH];
            safestrcpy(alt, "/glibc/lib/", MAXPATH);
            safestrcpy(alt + 11, bn, MAXPATH - 11);
            lip = namei(alt);
        }
        if (lip == 0) goto bad;

        lip->i_op->lock(lip);
        struct elfhdr lelf;
        if (lip->i_op->read(lip, 0, (uint64)&lelf, 0, sizeof(lelf)) != sizeof(lelf))
            goto bad_interp;
        if (lelf.magic != ELF_MAGIC) goto bad_interp;

        interp_base = 0x40000000;  // 1GB base, well above main binary

        for (i = 0, off = lelf.phoff; i < lelf.phnum; i++, off += sizeof(ph)) {
            if (lip->i_op->read(lip, 0, (uint64)&ph, off, sizeof(ph)) != sizeof(ph))
                goto bad_interp;
            if (ph.type != ELF_PROG_LOAD) continue;
            if (ph.memsz < ph.filesz) goto bad_interp;

            uint64 va = interp_base + ph.vaddr;
            uint64 vend = va + ph.memsz;

            // Map pages manually (no uvmalloc → p->sz untouched)
            for (uint64 pg = PGROUNDDOWN(va); pg < PGROUNDUP(vend); pg += PGSIZE) {
                char *mem = kalloc();
                if (mem == 0) goto bad_interp;
                memset(mem, 0, PGSIZE);
                if (mappages(pagetable, pg, PGSIZE, (uint64)mem,
                            PTE_P | PTE_W | PTE_PLV | PTE_MAT | PTE_D) < 0) {
                    kfree(mem);
                    goto bad_interp;
                }
            }

            // Copy file data
            uint64 fend = va + ph.filesz;
            for (uint64 pg = PGROUNDDOWN(va); pg < PGROUNDUP(fend); pg += PGSIZE) {
                uint64 pa = walkaddr(pagetable, pg);
                if (pa == 0) goto bad_interp;
                uint64 skip = (pg < va) ? (va - pg) : 0;
                uint64 foff = ph.off + (pg - PGROUNDDOWN(va));
                uint n = PGSIZE - skip;
                if (pg + skip + n > fend) n = fend - (pg + skip);
                if (lip->i_op->read(lip, 0, (uint64)pa + skip, foff, n) != (int)n)
                    goto bad_interp;
            }

            // Zero BSS
            if (ph.memsz > ph.filesz) {
                uint64 bss = va + ph.filesz;
                for (uint64 pg = PGROUNDDOWN(bss); pg < vend; pg += PGSIZE) {
                    uint64 pa = walkaddr(pagetable, pg);
                    if (pa == 0) continue;
                    uint64 off = (pg < bss) ? (bss - pg) : 0;
                    uint64 len = PGSIZE - off;
                    if (pg + len > vend) len = vend - pg;
                    memset((void*)((uint64)pa + off), 0, len);
                }
            }
        }

        interp_entry = interp_base + lelf.entry;
        lip->i_op->unlock(lip);
        lip = 0;
        goto interp_done;

    bad_interp:
        if (lip) lip->i_op->unlock(lip);
        goto bad;
    }
interp_done:

    p = myproc();
    uint64 oldsz = p->sz;
    sz = PGROUNDUP(sz);

    sp = USTACK_TOP;
    stackbase = USTACK;
    uint64 sz1;
#ifdef LA2K1000
    if ((sz1 = uvmalloc(pagetable, USTACK, USTACK + USTACK_PAGE * PGSIZE,
                        PTE_P | PTE_W | PTE_PLV | PTE_MAT | PTE_D | PTE_U)) == 0)
#else
    if ((sz1 = uvmalloc(pagetable, USTACK, USTACK + USTACK_PAGE * PGSIZE,
                        PTE_P | PTE_W | PTE_PLV | PTE_MAT | PTE_D)) == 0)
#endif
        goto bad;
    uvmclear(pagetable, USTACK);

    // 预留8字节，与 RISC-V 分支保持一致，有助于最终 sp 对齐
    sp -= sizeof(uint64);

    // 推送环境变量字符串
    for (envc = 0; envp && envp[envc]; envc++) {
        if (envc >= MAXENV) goto bad;
        sp -= strlen(envp[envc]) + 1;
        sp -= sp % 16;
        if (sp < stackbase) goto bad;
        if (copyout(pagetable, sp, envp[envc], strlen(envp[envc]) + 1) < 0) goto bad;
        estack[envc] = sp;
    }
    estack[envc] = 0;

    // 推送参数字符串
    for (argc = 0; argv[argc]; argc++) {
        if (argc >= MAXARG) goto bad;
        sp -= strlen(argv[argc]) + 1;
        sp -= sp % 16;
        if (sp < stackbase) goto bad;
        if (copyout(pagetable, sp, argv[argc], strlen(argv[argc]) + 1) < 0) goto bad;
        ustack[argc] = sp;
    }
    ustack[argc] = 0;

    sp -= sp % 16;

    // 准备辅助向量，提供常见项并包含 AT_RANDOM 指向16字节缓冲
    uint64 aux[MAX_AT * 2];
    aux[0] = 0;
    aux[1] = 0;
    aux[2] = 0;  // 初始化
    alloc_aux(aux, AT_PAGESZ, PGSIZE);
    alloc_aux(aux, AT_ENTRY, elf.entry);
    if (has_interp) {
        alloc_aux(aux, AT_BASE, interp_base);
        alloc_aux(aux, AT_PHDR, elf.phoff);
        alloc_aux(aux, AT_PHENT, elf.phentsize);
        alloc_aux(aux, AT_PHNUM, elf.phnum);
    }
    alloc_aux(aux, AT_UID, p->uid);
    alloc_aux(aux, AT_EUID, has_setuid ? file_uid : p->euid);
    alloc_aux(aux, AT_GID, p->gid);
    alloc_aux(aux, AT_EGID, has_setgid ? file_gid : p->egid);
    alloc_aux(aux, AT_SECURE, (has_setuid || has_setgid) ? 1 : 0);
    // 为 AT_RANDOM 预留16字节
    {
        uint8 randbuf[16];
        // 简化：填零，不影响大多数程序（如需真随机可接入时钟）
        memset(randbuf, 0, sizeof(randbuf));
        sp -= sizeof(randbuf);
        if (sp < stackbase) goto bad;
        if (copyout(pagetable, sp, (char *)randbuf, sizeof(randbuf)) < 0) goto bad;
        alloc_aux(aux, AT_RANDOM, sp);
    }
    // 标准终止项
    alloc_aux(aux, AT_NULL, 0);
    // 先保持 16 字节对齐，然后顺序布局为：auxv -> envp[] -> argv[] -> argc
    // 为保证最终 sp 仍保持 16 字节对齐，预先插入必要的 8 字节填充
    sp -= sp % 16;
    {
        int argc_aux = aux[0];
        uint64 aux_bytes = (uint64)(argc_aux * 2) * sizeof(uint64) + 2 * sizeof(uint64);
        uint64 env_bytes = (envc + 1) * sizeof(uint64);
        uint64 argv_bytes = (argc + 1) * sizeof(uint64);
        uint64 total_tables = aux_bytes + env_bytes + argv_bytes + sizeof(uint64);  // +argc
        if (((sp - total_tables) & 0xf) != 0) {
            sp -= sizeof(uint64);  // 插入 8 字节填充
        }
        // 放置 auxv
        sp -= aux_bytes;
        if (sp < stackbase) goto bad;
        if (copyout(pagetable, sp, (char *)(aux + 1), aux_bytes) < 0) goto bad;
    }

    // 推送环境变量指针表（紧随 argv[] 的 NULL 之后，不再额外对齐以保证紧邻）
    sp -= (envc + 1) * sizeof(uint64);
    if (sp < stackbase) goto bad;
    if (copyout(pagetable, sp, (char *)estack, (envc + 1) * sizeof(uint64)) < 0) goto bad;
    uint64 envp_ptr = sp;

    // 推送参数指针
    sp -= (argc + 1) * sizeof(uint64);
    if (sp < stackbase) goto bad;
    if (copyout(pagetable, sp, (char *)ustack, (argc + 1) * sizeof(uint64)) < 0) goto bad;
    uint64 argv_ptr = sp;

    // 推送argc（栈最低地址处）：形成 [argc][argv[]][NULL][envp[]][NULL][auxv[]] 的顺序
    sp -= sizeof(uint64);
    if (sp < stackbase) goto bad;
    if (copyout(pagetable, sp, (char *)&argc, sizeof(uint64)) < 0) goto bad;

    uint64 final_sp = sp;

    // 保存程序名
    for (last = s = path; *s; s++)
        if (*s == '/') last = s + 1;
    safestrcpy(p->name, last, sizeof(p->name));

    // 获取进程锁
    acquire(&p->lock);

    // 处理setuid/setgid
    if (has_setuid) {
        p->euid = file_uid;
        p->suid = file_uid;
    }
    if (has_setgid) {
        p->egid = file_gid;
        p->sgid = file_gid;
    }

    oldpagetable = p->pagetable;
    p->pagetable = pagetable;
    p->sz = sz;

    // 设置寄存器：设置 PC 与 SP；同时填充 a0/a1/a2 以满足 LA/Linux 约定
    p->trapframe->era = interp_entry;
    p->trapframe->sp = final_sp;
    p->trapframe->a0 = argc;
    p->trapframe->a1 = argv_ptr;
    p->trapframe->a2 = envp_ptr;

    release(&p->lock);

    proc_freepagetable(oldpagetable, oldsz);
    return 0;

bad:
    printf("execve: Failed for path %s\n", path);
    if (pagetable) proc_freepagetable(pagetable, sz);
    if (ip) ip->i_op->unlock(ip);
    return -1;
}

#endif

// Load a program segment into pagetable at virtual address va.
// va must be page-aligned
// and the pages from va to va+sz must already be mapped.
// Returns 0 on success, -1 on failure.
static int loadseg(pagetable_t pagetable, uint64 va, struct inode *ip, uint offset, uint sz) {
    uint i, n;
    uint64 pa;

    for (i = 0; i < sz; i += PGSIZE) {
        pa = walkaddr(pagetable, va + i);
        if (pa == 0) panic("loadseg: address should exist");
        if (sz - i < PGSIZE)
            n = sz - i;
        else
            n = PGSIZE;
        // LA2K1000: walkaddr() 返回可访问的直映地址；其他平台保持原样
#if defined(LOONGARCH)
#ifdef LA2K1000
        if (ip->i_op->read(ip, 0, pa, offset + i, n) != n) return -1;
#else
        if (ip->i_op->read(ip, 0, (uint64)pa, offset + i, n) != n) return -1;
#endif
#else
        if (ip->i_op->read(ip, 0, (uint64)pa, offset + i, n) != n) return -1;
#endif
    }

    return 0;
}
