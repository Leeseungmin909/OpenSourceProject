#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("사용법: %s <원본파일> <복사본파일>\n", argv[0]);
        exit(1);
    }

    int src_fd, dst_fd;
    char *src_ptr, *dst_ptr;
    struct stat sb;

    src_fd = open(argv[1], O_RDONLY);
    fstat(src_fd, &sb); 

    dst_fd = open(argv[2], O_RDWR | O_CREAT | O_TRUNC, 0644);
    ftruncate(dst_fd, sb.st_size);

    src_ptr = mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, src_fd, 0);
    dst_ptr = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, dst_fd, 0);

    memcpy(dst_ptr, src_ptr, sb.st_size);
    munmap(src_ptr, sb.st_size);
    munmap(dst_ptr, sb.st_size);
    
    close(src_fd); 
    close(dst_fd);

    printf("복사가 완료되었습니다! (%ld bytes)\n", sb.st_size);

    return 0;
}