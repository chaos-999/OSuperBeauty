#define O_RDONLY 0x000
#define O_WRONLY 0x001
#define O_RDWR 0x002
#define O_CREATE 0x40  // 修改为与测试程序一致
#define O_CREAT 0x40   // Alias for O_CREATE
#define O_TRUNC 0x200
#define O_DIRECTORY 0x004
#define O_CLOEXEC 0x008
#define O_APPEND 0x400
#define O_NONBLOCK 0x800

#define AT_FDCWD -100
#define AT_REMOVEDIR 0x200

#define UTIME_NOW ((1l << 30) - 1l)
#define UTIME_OMIT ((1l << 30) - 2l)

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#define F_DUPFD 0
#define F_GETFD 1
#define F_SETFD 2
#define F_GETFL 3
#define F_SETFL 4
#define F_DUPFD_CLOEXEC 1030