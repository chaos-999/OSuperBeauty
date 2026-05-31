#ifndef _POLL_H
#define _POLL_H

#include "types.h"

typedef unsigned long nfds_t;

struct pollfd {
    int fd;        /* 文件描述符 */
    short events;  /* 请求的事件 */
    short revents; /* 返回的事件 */
};

/* poll事件类型 */
#define POLLIN 0x001   /* 可读 */
#define POLLPRI 0x002  /* 有紧急数据可读 */
#define POLLOUT 0x004  /* 可写 */
#define POLLERR 0x008  /* 错误条件 */
#define POLLHUP 0x010  /* 挂起 */
#define POLLNVAL 0x020 /* 无效请求 */

#endif