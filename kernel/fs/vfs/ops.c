#include "fs/vfs/ops.h"

#include <fs/fcntl.h>

#include "fs/vfs/file.h"
#include "fs/vfs/fs.h"
#include "types.h"
#include "platform.h"
#include "defs.h"
#include "fs/vfs_ext4_ext.h"

void get_absolute_path(const char *path, const char *cwd, char *absolute_path) {
    // debug
    if (path == NULL) {
        strcpy(absolute_path, cwd);
    } else if (path[0] == '/') {
        strcpy(absolute_path, path);
    } else {
        strcpy(absolute_path, cwd);
        strcat(absolute_path, "/");
        strcat(absolute_path, path);
    }
    // handle ./ and ../
    char *p = absolute_path;
    while (*p != '\0') {
        if (*p == '.' && *(p + 1) != '\0' && *(p + 1) == '/') {
            strcpy(p, p + 2);
        } else if (*p == '.' && *(p + 1) != '\0' && *(p + 1) == '.' && *(p + 2) != '\0' &&
                   *(p + 2) == '/') {
            char *q = p - 2;
            while (q >= absolute_path && *q != '/') {
                q--;
            }
            if (q >= absolute_path) {
                strcpy(q + 1, p + 3);
                p = q;
            } else {
                strcpy(absolute_path, p + 3);
                p = absolute_path;
            }
        } else {
            p++;
        }
    }

    /* handle trailing . and .. */
    size_t path_len = strlen(absolute_path);
    // remove trailing "/."
    if (path_len >= 2) {
        if (absolute_path[path_len - 1] == '.' && absolute_path[path_len - 2] == '/') {
            // "/." at end
            absolute_path[path_len - 2] = '\0';
            path_len -= 2;
        }
    }
    if (path_len >= 3) {
        p = absolute_path + path_len - 2;
        if ((*p == '/' || *p == '.') && *(p + 1) == '.' && *(p + 2) == '\0') {
            *p = '\0';
        }
    }

    path_len = strlen(absolute_path);
    if (path_len >= 4) {
        p = absolute_path + path_len - 3;
        if (*p == '/' && *(p + 1) == '.' && *(p + 2) == '.' && *(p + 3) == '\0') {
            if (p == absolute_path) {
                strcpy(absolute_path, "/");
                return;
            }
            char *q = p - 1;
            while (q >= absolute_path && *q != '/') {
                q--;
            }
            if (q >= absolute_path) {
                *(q + 1) = '\0';
            } else {
                strcpy(absolute_path, "/");
            }
        }
    }

    while (strlen(absolute_path) >= 2 && absolute_path[0] == '/' && absolute_path[1] == '/') {
        strcpy(absolute_path, absolute_path + 1);
    }
    size_t len = strlen(absolute_path);
    if (len > 1 && absolute_path[len - 1] == '/') {
        absolute_path[len - 1] = '\0';
        --len;
    }
    if (strlen(absolute_path) == 0) {
        strcpy(absolute_path, "/");
    } else if (absolute_path[0] != '/') {
        size_t len2 = strlen(absolute_path);
        char *x = absolute_path + len2;
        *(x + 1) = 0;
        while (x > absolute_path) {
            *x = *(x - 1);
            x--;
        }
        *x = '/';
    }
}

// Paths

// Copy the next path element from path into name.
// Return a pointer to the element following the copied one.
// The returned path has no leading slashes,
// so the caller can check *path=='\0' to see if the name is the last one.
// If no name to remove, return 0.
//
// Examples:
//   skipelem("a/bb/c", name) = "bb/c", setting name = "a"
//   skipelem("///a//bb", name) = "bb", setting name = "a"
//   skipelem("a", name) = "", setting name = "a"
//   skipelem("", name) = skipelem("////", name) = 0
//
// static char*
// skipelem(char *path, char *name)
// {
//     char *s;
//     int len;

//     while(*path == '/')
//         path++;
//     if(*path == 0)
//         return 0;
//     s = path;
//     while(*path != '/' && *path != 0)
//         path++;
//     len = path - s;
//     if(len >= DIRSIZ)
//         memmove(name, s, DIRSIZ);
//     else {
//         memmove(name, s, len);
//         name[len] = 0;
//     }
//     while(*path == '/')
//         path++;
//     return path;
// }

// Look up and return the inode for a path name.
// If parent != 0, return the inode for the parent and copy the final
// path element into name, which must have room for DIRSIZ bytes.
// Must be called inside a transaction since it calls iput().
static struct inode *namex(char *path, int nameiparent, char *name) {
    // struct inode *ip, *next;
    //
    // // if(*path == '/') {
    // //
    // // }
    // // else
    // //     ip = idup(myproc()->cwd);
    //
    // while((path = skipelem(path, name)) != 0){
    //     ilock(ip);
    //     if(ip->i_type != T_DIR){
    //         iunlockput(ip);
    //         return 0;
    //     }
    //     if(nameiparent && *path == '\0'){
    //         // Stop one level early.
    //         iunlock(ip);
    //         return ip;
    //     }
    //     if((next = dirlookup(ip, name, 0)) == 0){
    //         iunlockput(ip);
    //         return 0;
    //     }
    //     iunlockput(ip);
    //     ip = next;
    // }
    // if(nameiparent){
    //     iput(ip);
    //     return 0;
    // }
    // return ip;
    return NULL;
}

struct inode *namei(char *path) {
    char name[EXT4_PATH_LONG_MAX];
    get_absolute_path(path, myproc()->cwd.path, name);
    return vfs_ext_namei(name);
}

struct inode *nameiparent(char *path, char *name) { return namex(path, 1, name); }
