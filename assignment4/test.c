#include<stdio.h>
#include<stdlib.h>
#include<string.h>

void writeFile() {
	int ret;
	FILE *file = fopen("/dev/one", "w");
	char *buf = (char *)malloc(5*1024*1024 + 1); // Malloc 4MB + 1byte to buf.
	memset(buf, 'a', 4*1024*1024);
    memset(buf + 4*1024*1024, 'b', 1*1024*1024); 
	buf[5*1024*1024] = '\0';
    ret = fputs(buf, file);
	printf("%d\n", ret); 
    fclose(file);
}

int main() {
	writeFile();
	return 0;
}
