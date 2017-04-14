#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
//needed for IO things. Attention that this is different from kernel mode int lcd;
#define SCULL_IOC_MAGIC 'k'
#define SCULL_HELLO _IO(SCULL_IOC_MAGIC, 1)
#define SCULL_WRITE _IOW(SCULL_IOC_MAGIC, 2, char*)
#define SCULL_READ _IOR(SCULL_IOC_MAGIC, 3, char*)
#define SCULL_EDIT _IOWR(SCULL_IOC_MAGIC, 4, char*)
int lcd;
void test() {
    int k, i, sum;
    char s[3];
    memset(s, '2', sizeof(s)); printf("test begin!\n");
    k = write(lcd, s, sizeof(s)); printf("written = %d\n", k);
    k = ioctl(lcd, SCULL_HELLO); printf("result = %d\n", k);
}
void mytest() {
    int k;
    char s[11], user_meg[11] = {0};
    memset(s, 'm', sizeof(s));
    s[10] = 0;
    printf("Test by Zhendong\n");

    // Write in a 10 bytes message by ioctl.
    k = ioctl(lcd, SCULL_WRITE, s);
    printf("ioctl write length = %d, content = %s\n", k, s);

    // Read a 10 bytes message by ioctl.
    k = ioctl(lcd, SCULL_READ, user_meg);
    printf("ioctl read result = %s\n", user_meg);
}
void mytest2() {
    // Firstly we write in some values to the driver.
    mytest();

    // Begin to test the EDIT function using ioctl _IOWR.
    int k;
    char user_meg[11] = {0};
    memset(user_meg, 'Q', sizeof(user_meg));
    user_meg[10] = 0;
    printf("\nTest _IOWR by Zhendong\n");
    printf("Before the edit, the user_meg = %s\n", user_meg);

    k = ioctl(lcd, SCULL_EDIT, user_meg);
    printf("After the edit, the user_meg = %s\n", user_meg);
}
int main(int argc, char **argv)
{
    lcd = open("/dev/one", O_RDWR); if (lcd == -1) {
        perror("unable to open lcd"); exit(EXIT_FAILURE);
    }
    //test();
    mytest2();
    close(lcd);
    return 0;
}
