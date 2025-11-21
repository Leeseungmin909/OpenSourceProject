#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

void list_recursive(const char *base_path) {
    DIR *dir;
    struct dirent *entry;
    struct stat statbuf;
    char path[1024];

    if ((dir = opendir(base_path)) == NULL) {
        return;
    }

    printf("\n%s:\n", base_path); 

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        printf("%s  ", entry->d_name); 
    }
    printf("\n"); 

    rewinddir(dir);

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        sprintf(path, "%s/%s", base_path, entry->d_name);
        
        if (lstat(path, &statbuf) == -1) {
            continue;
        }

        if (S_ISDIR(statbuf.st_mode)) {
            list_recursive(path);
        }
    }

    closedir(dir);
}

int main() {
    list_recursive(".");
    return 0;
}