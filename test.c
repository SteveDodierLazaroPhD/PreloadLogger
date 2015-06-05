/* 
    2015 (c) Steve Dodier-Lazaro <sidnioulz@gmail.com>
    This file is part of PreloadLogger.

    PreloadLogger is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    PreloadLogger is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with PreloadLogger.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <dirent.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main(void)
{
    
    int fd = open("/home/study/Preload/lib.c", O_RDONLY);
    if (fd < 0) {
        printf("open() failed\n");
    }
    
    fd = open("/home/study/Preload/lib.c", O_RDONLY, S_IRWXU);
    if (fd < 0) {
        printf("open() failed\n");
    }
    
    fd = open64("/home/study/Preload/lib.c", O_RDWR);
    if (fd < 0) {
        printf("open64() failed\n");
    } else close(fd);
    
    fd = open("/home/study/.local/share/zeitgeist/activity.sqlite-wal", O_RDWR);
    if (fd < 0) {
        printf("open() failed\n");
    } else close(fd);
    
    fd = open("./lib.c", O_RDONLY);
    if (fd < 0) {
        printf("open() failed\n");
    } else close(fd);
    
    fd = open("./dsfsdf.c", O_RDONLY);
    if (fd < 0) {
        printf("open() failed\n");
    } else close(fd);
    
    fd = open("/tmp/fsdfiop", O_RDWR | O_APPEND);
    if (fd < 0) {
        printf("open() failed\n");
    } else close(fd);
    
    fd = open("/var/lib", O_RDONLY);
    if (fd < 0) {
        printf("open() failed\n");
    } else close(fd);
    
    fd = open("/usr/Preload/test.c", O_WRONLY | O_APPEND);
    if (fd < 0) {
        printf("open() failed\n");
    } else close(fd);
    
    fd = openat(1, "Preload/test.c", O_WRONLY | O_APPEND);
    if (fd < 0) {
        printf("openat() failed\n");
    } else close(fd);
    
    unlink ("/tmp/LKFEKJW{F");
    unlink ("/tmp/PreloadTest");
    link("/home/study/Downloads", "/tmp/dl");
    link("lib.so", "/tmp/dl");

    FILE *gkldfjg = fopen("lib.so", "r");
    FILE *two = freopen("lib.so", "a+", gkldfjg);


    int ngkdjpg = open("/tmp", O_RDONLY);
    fdopen(ngkdjpg, "r");
    fdopen(ngkdjpg, "w+");

    int pipefd[2];
    pipe(pipefd);
    fd = creat("./blublu.c", S_IRWXU);
    if (fd < 0) {
        printf("creat() failed\n");
    } else close(fd);
    
    fd = creat("/opt/lol", S_IRWXU);
    if (fd < 0) {
        printf("creat() failed\n");
    } else close(fd);
    
    fd = creat("./lol", S_IRWXU);
    if (fd < 0) {
        printf("creat() failed\n");
    } else close(fd);
    
    int foo = unlink("./lol");
    if (foo) {
        printf("unlink() failed\n");
    }
    
    foo = remove("./lol");
    if (foo) {
        printf("remove() failed\n");
    }
    
    fd = dup(2);
    if (fd < 0) {
        printf("dup() failed\n");
    }
    
    fd = dup2(2, 30284);
    if (fd < 0) {
        printf("dup2() failed\n");
    }
    
    fd = dup3(2, 3543, S_IRWXU);
    if (fd < 0) {
        printf("dup3() failed\n");
    } 
    link("/usr/Preload/", "/tmp/PreloadTest");
    open("lib.so", O_RDWR);

    fork();
    int sv[2];
    socketpair(AF_LOCAL, SOCK_DGRAM, 0, sv);
    close(sv[0]);
    close(sv[1]);

    shm_open("foo", O_RDWR, 0700);
    rename("foo", "bar");  
    shm_unlink("bar");


    DIR *dir = opendir("/tmp/test");

    int chaussette = socket(AF_INET, SOCK_DGRAM, 0);
    close(chaussette);
    chaussette = socket(AF_LOCAL, SOCK_DGRAM, 0);
    close(chaussette);
    mkdir("/tmp/test", 1770);
    dir = opendir("/tmp/test");
    closedir(dir);
    rmdir("/tmp/test");


    return 0;
}
