#include <types.h>
#include <fs/stat.h>
#include "user/user.h"
#include <fs/fcntl.h>

int main(int argc, char *argv[]) {
    int i;

    if (argc < 2) {
        fprintf(2, "Usage: mkdir files...\n");
        exit(1);
    }

    for (i = 1; i < argc; i++) {
        if (mkdirat(AT_FDCWD, argv[i], 0666) < 0) {
            fprintf(2, "mkdir: %s failed to create\n", argv[i]);
            break;
        }
    }

    exit(0);
}
