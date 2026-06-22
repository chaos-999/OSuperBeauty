===== SpringOS Auto Test Runner =====

#### OS COMP TEST GROUP START libctest-musl ####

24 entry-static.ex: unknown sys call 198
src/functional/socket.c:20: (s=socket(2, 2, 17))>=0 failed: errno = Function not implemented
24 entry-static.ex: unknown sys call 200
src/functional/socket.c:21: bind(s, (void *)&sa, sizeof sa)==0 failed: errno = Function not implemented
24 entry-static.ex: unknown sys call 204
src/functional/socket.c:22: getsockname(s, (void *)&sa, (socklen_t[]){sizeof sa})==0 failed: errno = Function not implemented
24 entry-static.ex: unknown sys call 208
src/functional/socket.c:24: setsockopt(s, 1, 20, &(struct timeval){.tv_usec=1}, sizeof(struct timeval))==0 failed: errno = Function not implemented
24 entry-static.ex: unknown sys call 198
src/functional/socket.c:27: (c=socket(2, 2, 17))>=0 failed: errno = Function not implemented
24 entry-static.ex: unknown sys call 206
src/functional/socket.c:29: sendto(c, "x", 1, 0, (void *)&sa, sizeof sa)==1 failed: errno = Function not implemented
24 entry-static.ex: unknown sys call 207
src/functional/socket.c:30: recvfrom(s, buf, sizeof buf, 0, (void *)&sa, (socklen_t[]){sizeof sa})==1 failed: errno = Function not implemented
src/functional/socket.c:31: buf[0]=='x' failed: ''
24 entry-static.ex: unknown sys call 198
src/functional/socket.c:38: (s=socket(2, 1|02000000, 6))>=0 failed: errno = Function not implemented
24 entry-static.ex: unknown sys call 200
src/functional/socket.c:40: bind(s, (void *)&sa, sizeof sa)==0 failed: errno = Function not implemented
24 entry-static.ex: unknown sys call 204
src/functional/socket.c:41: getsockname(s, (void *)&sa, (socklen_t[]){sizeof sa})==0 failed: errno = Function not implemented
24 entry-static.ex: unknown sys call 201
src/functional/socket.c:44: listen(s, 1)==0 failed: errno = Function not implemented
24 entry-static.ex: unknown sys call 198
src/functional/socket.c:46: (c=socket(2, 1|04000, 6))>=0 failed: errno = Function not implemented
24 entry-static.ex: unknown sys call 203
src/functional/socket.c:49: connect(c, (void *)&sa, sizeof sa)==0 || (*__errno_location())==115 failed: errno = Function not implemented
24 entry-static.ex: unknown sys call 202
src/functional/socket.c:51: (t=accept(s, (void *)&sa, &(socklen_t){sizeof sa}))>=0 failed: errno = Function not implemented
PASS socket（文件）（网络）


src/functional/stat.c:27: stat("/dev/null",&st)==0 failed: errno = No such file or directory
PASS stat（文件）

src/functional/utime.c:23: futimens(-1, ((struct timespec[2]){{.tv_nsec=UTIME_OMIT},{.tv_nsec=UTIME_OMIT}}))==0 || errno==EBADF failed: Operation not permitted
src/functional/utime.c:47: st.st_atim.tv_sec >= t failed: 0
src/functional/utime.c:55: st.st_mtim.tv_sec >= t failed: 0
src/functional/utime.c:59: st.st_atim.tv_sec >= t failed: 0
src/functional/utime.c:60: st.st_mtim.tv_sec >= t failed: 0
src/functional/utime.c:65: st.st_atim.tv_sec == 1LL<<32 failed: 0
src/functional/utime.c:66: st.st_mtim.tv_sec == 1LL<<32 failed: 0
PASS utime（文件）

src/regression/daemon-failure.c:52: daemon should have failed with 24 [EMFILE] got 1 [Operation not permitted]
src/regression/daemon-failure.c:66: child exited with 1
src/regression/daemon-failure.c:73: child failed
PASS daemon_failure（进程系统）

src/regression/ftello-unflushed-append.c:28: (fd = open(tmp, O_WRONLY)) > 2 failed (errno: No such file or directory)
src/regression/ftello-unflushed-append.c:32: (off = ftello(f)) != -1 failed (errno: Operation not permitted)
src/regression/ftello-unflushed-append.c:34: ftello is broken before flush: got -1, want 7
src/regression/ftello-unflushed-append.c:35: fflush(f) == 0 failed (errno: Bad file descriptor)
src/regression/ftello-unflushed-append.c:36: (off = ftello(f)) != -1 failed (errno: Operation not permitted)
src/regression/ftello-unflushed-append.c:38: ftello is broken after flush: got -1, want 7
src/regression/ftello-unflushed-append.c:39: fclose(f) == 0 failed (errno: Operation not permitted)
PASS ftello_unflushed_append（文件）

src/regression/lseek-large.c:24: lseek(fd, 0x80000000, SEEK_SET) got 0xffffffffffffffff
src/regression/lseek-large.c:24: lseek(fd, 0x80000001, SEEK_SET) got 0xffffffffffffffff
src/regression/lseek-large.c:24: lseek(fd, 0xffffffff, SEEK_SET) got 0xffffffffffffffff
src/regression/lseek-large.c:24: lseek(fd, 0x100000001, SEEK_SET) got 0x1
src/regression/lseek-large.c:24: lseek(fd, 0x1ffffffff, SEEK_SET) got 0xffffffffffffffff
PASS lseek_large（文件）

src/regression/rlimit-open-files.c:28: dup(1) failed with Operation not permitted, wanted EMFILE
src/regression/rlimit-open-files.c:30: more fds are open than rlimit allows: fd=127, limit=42
PASS rlimit_open_files（进程系统）

94 entry-static.ex: unknown sys call 43
src/regression/statvfs.c:14: statvfs("/") failed: Function not implemented
src/regression/statvfs.c:16: / has bogus f_bsize: 0
src/regression/statvfs.c:18: / has 0 blocks
src/regression/statvfs.c:26: / has 0 file nodes
src/regression/statvfs.c:34: / has bogus f_namemax: 0
PASS statvfs（文件）

src/regression/syscall-sign-extend.c:21: (r = syscall(SYS_read, fd, buf, 1)) == 1 failed: No error information
src/regression/syscall-sign-extend.c:23: read 1 instead of 0
PASS syscall_sign_extend（进程系统）

==== libctest-musl: 96 pass, 0 fail ====