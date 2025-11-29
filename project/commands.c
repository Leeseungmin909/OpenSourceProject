#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include "commands.h"

int confirm_action(const char *msg, const char *filename) {
    printf("%s '%s'? (y/n): ", msg, filename);
    char ans = getchar();
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
    
    if (ans == 'y' || ans == 'Y') return 1;
    return 0;
}

void cmd_pwd(int argc, char *argv[]) {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL)
        printf("%s\n", cwd);
    else
        perror("pwd error");
}

void cmd_cd(int argc, char *argv[]) {
    char *path = (argc > 1) ? argv[1] : getenv("HOME");
    if (chdir(path) != 0) perror("cd error");
}

void cmd_ls(int argc, char *argv[]) {
    DIR *dp;
    struct dirent *entry;
    struct stat st;
    struct passwd *pw;
    struct group *gr;
    struct tm *t;
    char timebuf[64];
    
    int a_flag = 0, l_flag = 0;
    char *path = ".";

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (strstr(argv[i], "a")) a_flag = 1;
            if (strstr(argv[i], "l")) l_flag = 1;
        } else {
            path = argv[i];
        }
    }

    if ((dp = opendir(path)) == NULL) {
        perror("ls error");
        return;
    }

    while ((entry = readdir(dp)) != NULL) {
        if (!a_flag && entry->d_name[0] == '.') continue;

        if (l_flag) {
            char fullpath[1024];
            sprintf(fullpath, "%s/%s", path, entry->d_name);
            
            if (lstat(fullpath, &st) == -1) continue;

            if (S_ISDIR(st.st_mode)) printf("d");
            else if (S_ISLNK(st.st_mode)) printf("l"); 
            else printf("-");

            printf( (st.st_mode & S_IRUSR) ? "r" : "-");
            printf( (st.st_mode & S_IWUSR) ? "w" : "-");
            printf( (st.st_mode & S_IXUSR) ? "x" : "-");
            printf( (st.st_mode & S_IRGRP) ? "r" : "-");
            printf( (st.st_mode & S_IWGRP) ? "w" : "-");
            printf( (st.st_mode & S_IXGRP) ? "x" : "-");
            printf( (st.st_mode & S_IROTH) ? "r" : "-");
            printf( (st.st_mode & S_IWOTH) ? "w" : "-");
            printf( (st.st_mode & S_IXOTH) ? "x" : "-");

            pw = getpwuid(st.st_uid);
            gr = getgrgid(st.st_gid);
            t = localtime(&st.st_mtime);
            strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", t);
            
            printf(" %ld %s %s %5ld %s %s", 
                st.st_nlink, 
                (pw) ? pw->pw_name : "unknown", 
                (gr) ? gr->gr_name : "unknown", 
                st.st_size, 
                timebuf, 
                entry->d_name);
            
            if (S_ISLNK(st.st_mode)) {
                char link_target[1024];
                ssize_t len = readlink(fullpath, link_target, sizeof(link_target)-1);
                if (len != -1) {
                    link_target[len] = '\0';
                    printf(" -> %s", link_target);
                }
            }
            printf("\n");

        } else {
            printf("%s  ", entry->d_name);
        }
    }
    if (!l_flag) printf("\n");
    closedir(dp);
}

void cmd_mkdir(int argc, char *argv[]) {
    if (argc < 2) { printf("usage: mkdir dirname\n"); return; }
    if (mkdir(argv[1], 0755) != 0) perror("mkdir failed");
}

void cmd_rmdir(int argc, char *argv[]) {
    if (argc < 2) { printf("usage: rmdir dirname\n"); return; }
    if (rmdir(argv[1]) != 0) perror("rmdir failed");
}

void cmd_ln(int argc, char *argv[]) {
    int s_flag = 0;
    char *target = NULL, *linkname = NULL;

    for(int i=1; i<argc; i++) {
        if(strcmp(argv[i], "-s") == 0) s_flag = 1;
        else if(!target) target = argv[i];
        else if(!linkname) linkname = argv[i];
    }

    if (!target || !linkname) { printf("usage: ln [-s] target linkname\n"); return; }

    if (s_flag) {
        if (symlink(target, linkname) != 0) perror("symlink failed");
        else printf("Symbolic link created: %s -> %s\n", linkname, target);
    } else {
        if (link(target, linkname) != 0) perror("link failed");
        else printf("Hard link created: %s -> %s\n", linkname, target);
    }
}

void cmd_cp(int argc, char *argv[]) {
    int i_flag = 0;
    char *src = NULL, *dest = NULL;

    for(int i=1; i<argc; i++) {
        if(strcmp(argv[i], "-i") == 0) i_flag = 1;
        else if(!src) src = argv[i];
        else if(!dest) dest = argv[i];
    }

    if(!src || !dest) { printf("usage: cp [-i] src dest\n"); return; }

    if (i_flag && access(dest, F_OK) == 0) {
        if (!confirm_action("overwrite", dest)) return;
    }

    int sfd = open(src, O_RDONLY);
    if(sfd < 0) { perror("src open error"); return; }
    
    int dfd = open(dest, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if(dfd < 0) { perror("dest open error"); close(sfd); return; }

    char buf[4096];
    ssize_t n;
    while ((n = read(sfd, buf, sizeof(buf))) > 0)
        write(dfd, buf, n);

    close(sfd); close(dfd);
}

void cmd_rm(int argc, char *argv[]) {
    int i_flag = 0;
    char *file = NULL;

    for(int i=1; i<argc; i++) {
        if(strcmp(argv[i], "-i") == 0) i_flag = 1;
        else file = argv[i];
    }
    
    if(!file) { printf("usage: rm [-i] file\n"); return; }

    if(i_flag) {
        if (!confirm_action("remove", file)) return;
    }

    if (unlink(file) != 0) perror("rm failed");
}

void cmd_mv(int argc, char *argv[]) {
    int i_flag = 0;
    char *src = NULL, *dest = NULL;

    for(int i=1; i<argc; i++) {
        if(strcmp(argv[i], "-i") == 0) i_flag = 1;
        else if(!src) src = argv[i];
        else if(!dest) dest = argv[i];
    }

    if(!src || !dest) { printf("usage: mv [-i] src dest\n"); return; }

    if(i_flag && access(dest, F_OK) == 0) {
        if (!confirm_action("overwrite", dest)) return;
    }

    if (rename(src, dest) != 0) perror("mv failed");
}

void cmd_cat(int argc, char *argv[]) {
    int n_flag = 0;
    char *file = NULL;

    for(int i=1; i<argc; i++) {
        if(strcmp(argv[i], "-n") == 0) n_flag = 1;
        else file = argv[i];
    }
    
    if(!file) { printf("usage: cat [-n] file\n"); return; }

    FILE *fp = fopen(file, "r");
    if(!fp) { perror("cat file open error"); return; }

    char line[1024];
    int line_num = 1;
    while(fgets(line, sizeof(line), fp)) {
        if(n_flag) printf("%6d  ", line_num++);
        printf("%s", line);
    }
    fclose(fp);
}

void cmd_grep(int argc, char *argv[]) {
    int n_flag = 0, i_flag = 0;
    char *pattern = NULL, *file = NULL;

    for(int k=1; k<argc; k++) {
        if(strcmp(argv[k], "-n") == 0) n_flag = 1;
        else if(strcmp(argv[k], "-i") == 0) i_flag = 1;
        else if(!pattern) pattern = argv[k];
        else if(!file) file = argv[k];
    }

    if(!pattern || !file) { printf("usage: grep [-n] [-i] pattern file\n"); return; }

    FILE *fp = fopen(file, "r");
    if(!fp) { perror("grep file open error"); return; }

    char line[1024];
    int line_num = 0;
    while(fgets(line, sizeof(line), fp)) {
        line_num++;
        char *ptr;
        if(i_flag) ptr = strcasestr(line, pattern); 
        else ptr = strstr(line, pattern);

        if(ptr) {
            if(n_flag) printf("%d: ", line_num);
            printf("%s", line);
        }
    }
    fclose(fp);
}