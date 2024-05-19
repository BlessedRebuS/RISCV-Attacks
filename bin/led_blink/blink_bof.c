#include <stdlib.h>

#include <stdio.h>

#include <string.h>

#include <unistd.h>

#include <fcntl.h>



#define SYSFS_GPIO_EXPORT_GREEN "/sys/class/gpio/export"

#define SYSFS_GPIO_UNEXPORT_GREEN "/sys/class/gpio/unexport"

#define SYSFS_GPIO_RST_PIN_VAL_GREEN "466"

#define SYSFS_GPIO_RST_DIR_GREEN "/sys/class/gpio/gpio466/direction"

#define SYSFS_GPIO_RST_DIR_VAL_GREEN "OUT"

#define SYSFS_GPIO_RST_VAL_GREEN "/sys/class/gpio/gpio466/value"

#define SYSFS_GPIO_RST_VAL_H_GREEN "1"

#define SYSFS_GPIO_RST_VAL_L_GREEN "0"



#define SYSFS_GPIO_EXPORT_RED "/sys/class/gpio/export"

#define SYSFS_GPIO_UNEXPORT_RED "/sys/class/gpio/unexport"

#define SYSFS_GPIO_RST_PIN_VAL_RED "459"

#define SYSFS_GPIO_RST_DIR_RED "/sys/class/gpio/gpio459/direction"

#define SYSFS_GPIO_RST_DIR_VAL_RED "OUT"

#define SYSFS_GPIO_RST_VAL_RED "/sys/class/gpio/gpio459/value"

#define SYSFS_GPIO_RST_VAL_H_RED "1"

#define SYSFS_GPIO_RST_VAL_L_RED "0"



int green(){



    int fd;



    fd = open(SYSFS_GPIO_EXPORT_GREEN, O_WRONLY);

    if (fd == -1)

    {

        printf("ERR: export open error.\n");

        return EXIT_FAILURE;

    }



    write(fd, SYSFS_GPIO_RST_PIN_VAL_GREEN, sizeof(SYSFS_GPIO_RST_PIN_VAL_GREEN));

    close(fd);



    fd = open(SYSFS_GPIO_RST_DIR_GREEN, O_WRONLY);



    if (fd == -1)

    {

        printf("ERR: direction open error.\n");

        return EXIT_FAILURE;

    }



    write(fd, SYSFS_GPIO_RST_DIR_VAL_GREEN, sizeof(SYSFS_GPIO_RST_DIR_VAL_GREEN));

    close(fd);



    fd = open(SYSFS_GPIO_RST_VAL_GREEN, O_RDWR);

    if (fd == -1)

    {

        printf("ERR: gpio open error.\n");

        return EXIT_FAILURE;

    }





    write(fd, SYSFS_GPIO_RST_VAL_H_GREEN, sizeof(SYSFS_GPIO_RST_VAL_H_GREEN));

    usleep(1000000);

    write(fd, SYSFS_GPIO_RST_VAL_L_GREEN, sizeof(SYSFS_GPIO_RST_VAL_L_GREEN));

    usleep(1000000);



    close(fd);



    fd = open(SYSFS_GPIO_UNEXPORT_GREEN, O_WRONLY);

    if (fd == -1)

    {

        printf("ERR: unexport open error.\n");

        return EXIT_FAILURE;

    }



    write(fd, SYSFS_GPIO_RST_PIN_VAL_GREEN, sizeof(SYSFS_GPIO_RST_PIN_VAL_GREEN));

    close(fd);

    

    return 0;



}



int red(){



    int fd;



    fd = open(SYSFS_GPIO_EXPORT_RED, O_WRONLY);

    if (fd == -1)

    {

        printf("ERR: export open error.\n");

        return EXIT_FAILURE;

    }



    write(fd, SYSFS_GPIO_RST_PIN_VAL_RED, sizeof(SYSFS_GPIO_RST_PIN_VAL_RED));

    close(fd);



    fd = open(SYSFS_GPIO_RST_DIR_RED, O_WRONLY);



    if (fd == -1)

    {

        printf("ERR: direction open error.\n");

        return EXIT_FAILURE;

    }



    write(fd, SYSFS_GPIO_RST_DIR_VAL_RED, sizeof(SYSFS_GPIO_RST_DIR_VAL_RED));

    close(fd);



    fd = open(SYSFS_GPIO_RST_VAL_RED, O_RDWR);

    if (fd == -1)

    {

        printf("ERR: gpio open error.\n");

        return EXIT_FAILURE;

    }





    write(fd, SYSFS_GPIO_RST_VAL_H_RED, sizeof(SYSFS_GPIO_RST_VAL_H_RED));

    usleep(1000000);

    write(fd, SYSFS_GPIO_RST_VAL_L_RED, sizeof(SYSFS_GPIO_RST_VAL_L_RED));

    usleep(1000000);



    close(fd);



    fd = open(SYSFS_GPIO_UNEXPORT_RED, O_WRONLY);

    if (fd == -1)

    {

        printf("ERR: unexport open error.\n");

        return EXIT_FAILURE;

    }



    write(fd, SYSFS_GPIO_RST_PIN_VAL_RED, sizeof(SYSFS_GPIO_RST_PIN_VAL_RED));

    close(fd);

    

    return 0;



}



void not_called() {

    printf("Enjoy your shell!\n");

    system("/bin/bash");

}



int test_empty() {

    printf("Empty function\n");

    return 1;

}

void vulnerable_function(char* string) {

    char buffer[100];

    test_empty();

    strcpy(buffer, string);

}



int main(int argc, char** argv) {

    vulnerable_function(argv[1]);

    green();

    return 0;



}
