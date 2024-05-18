#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define SYSFS_GPIO_EXPORT "/sys/class/gpio/export"
#define SYSFS_GPIO_UNEXPORT "/sys/class/gpio/unexport"
#define SYSFS_GPIO_RST_PIN_VAL "466"
#define SYSFS_GPIO_RST_DIR "/sys/class/gpio/gpio466/direction"
#define SYSFS_GPIO_RST_DIR_VAL "OUT"
#define SYSFS_GPIO_RST_VAL "/sys/class/gpio/gpio466/value"
#define SYSFS_GPIO_RST_VAL_H "1"
#define SYSFS_GPIO_RST_VAL_L "0"

int main()

{
    int fd;
    int count = 30;

    fd = open(SYSFS_GPIO_EXPORT, O_WRONLY);
    if (fd == -1)
    {
        printf("ERR: export open error.\n");
        return EXIT_FAILURE;
    }

    write(fd, SYSFS_GPIO_RST_PIN_VAL, sizeof(SYSFS_GPIO_RST_PIN_VAL));
    close(fd);

    fd = open(SYSFS_GPIO_RST_DIR, O_WRONLY);

    if (fd == -1)
    {
        printf("ERR: direction open error.\n");
        return EXIT_FAILURE;
    }

    write(fd, SYSFS_GPIO_RST_DIR_VAL, sizeof(SYSFS_GPIO_RST_DIR_VAL));
    close(fd);

    fd = open(SYSFS_GPIO_RST_VAL, O_RDWR);
    if (fd == -1)
    {
        printf("ERR: gpio open error.\n");
        return EXIT_FAILURE;
    }

    while (count)
    {
        count--;
        write(fd, SYSFS_GPIO_RST_VAL_H, sizeof(SYSFS_GPIO_RST_VAL_H));
        usleep(1000000);
        write(fd, SYSFS_GPIO_RST_VAL_L, sizeof(SYSFS_GPIO_RST_VAL_L));
        usleep(1000000);
    }

    close(fd);

    fd = open(SYSFS_GPIO_UNEXPORT, O_WRONLY);
    if (fd == -1)
    {
        printf("ERR: unexport open error.\n");
        return EXIT_FAILURE;
    }

    write(fd, SYSFS_GPIO_RST_PIN_VAL, sizeof(SYSFS_GPIO_RST_PIN_VAL));
    close(fd);
    
    return 0;

}