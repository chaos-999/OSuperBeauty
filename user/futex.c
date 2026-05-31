#include <types.h>
#include <fs/stat.h>
#include <syscall/syscall.h>
#include "user/user.h"

#define FUTEX_WAIT 0
#define FUTEX_WAKE 1

static uint32 futex_var = 0;

int main(void) {
    printf("Testing futex implementation...\n");

    int pid = fork();
    if (pid == 0) {
        // Child process - wait on futex
        printf("Child: waiting on futex (expecting value 0)...\n");

        int ret = futex((uint64)&futex_var, FUTEX_WAIT, 0, 0, 0, 0);

        if (ret == 0) {
            printf("Child: futex wait returned successfully\n");
        } else {
            printf("Child: futex wait failed\n");
        }

        printf("Child: current futex value is %u\n", futex_var);
        exit(0);
    } else if (pid > 0) {
        // Parent process - wake up child after a delay
        sleep(1);  // Give child time to start waiting

        printf("Parent: changing futex value and waking child...\n");
        futex_var = 1;

        int ret = futex((uint64)&futex_var, FUTEX_WAKE, 1, 0, 0, 0);
        printf("Parent: futex wake returned %d\n", ret);

        wait(0);  // Wait for child to exit
        printf("Test completed\n");
    } else {
        printf("Fork failed\n");
        exit(1);
    }

    exit(0);
}