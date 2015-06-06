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


#define _GNU_SOURCE

#include <dirent.h>
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "logger.h"
#include "gslist.h"

LZGSList *fds = NULL;
LZGSList *files = NULL;
LZGSList *dirs = NULL;
LZGSList *shms = NULL;

//TODO dbus API?

static void log_event(const char *syscall_text,
                     const char *file,
                     const int dirfd,
                     const char *event_interpretation)
{
  if (!file || !syscall_text || !event_interpretation)
    return;

  LZGLog *log = lzg_log_get_default(0);
  if (!log) return;

  LZGEvent *event = lzg_event_new ();
  if (!event) return;
  
  lzg_event_set_interpretation (event, event_interpretation);
  char *actor = _zg_get_actor_from_pid (getpid ());
  lzg_event_set_actor (event, actor);
  lzg_event_set_pid (event, getpid());
  free (actor);
  
  LZGSubject *subject = lzg_subject_new ();
  if (!subject) {
    lzg_event_free (event);
    return;
  }
  lzg_subject_set_uri (subject, file);
  lzg_subject_set_text (subject, syscall_text);

  if (file && file[0] != '/') {
    char *origin = NULL;

    if (dirfd < 0) /* Includes AT_FDCWD */ {
      origin = get_current_dir_name();
    } else {
      origin = malloc (100 * sizeof(char));
      if (origin)
        snprintf (origin, 100, "fd: %d", dirfd);
    }
    lzg_subject_set_origin(subject, origin);
    free (origin);
  }

  lzg_event_add_subject(event, subject);
  lzg_log_insert_event(log, event);
}

static void log_old_new_event(const char *oldsubjecttext, const char *oldfile, const int olddirfd,
                              const char *newsubjecttext, const char *newfile, const int newdirfd,
                              const char *event_interpretation)
{
  if (!oldfile || !newfile || !oldsubjecttext || !newsubjecttext || !event_interpretation)
    return;

  LZGLog *log = lzg_log_get_default(0);
  if (!log) return;

  LZGEvent *event = lzg_event_new ();
  if (!event) return;

  lzg_event_set_interpretation (event, event_interpretation);
  char *actor = _zg_get_actor_from_pid (getpid ());
  lzg_event_set_actor (event, actor);
  lzg_event_set_pid (event, getpid());
  free (actor);
  
  LZGSubject *subject = lzg_subject_new ();
  if (!subject) {
    lzg_event_free (event);
    return;
  }
  lzg_subject_set_uri (subject, oldfile);
  lzg_subject_set_text (subject, oldsubjecttext);

  if (oldfile && oldfile[0] != '/') {
    char *origin = NULL;
    
    if (olddirfd < 0) /* Includes AT_FDCWD */ {
      origin = get_current_dir_name();
    } else {
      origin = malloc (100 * sizeof(char));
      if (origin)
        snprintf (origin, 100, "fd: %d", olddirfd);
    }
    lzg_subject_set_origin(subject, origin);
    free (origin);
  }
  lzg_event_add_subject(event, subject);
  
  subject = lzg_subject_new ();
  if (!subject) {
    lzg_event_free (event); // will clear the added subject
    return;
  }
  lzg_subject_set_uri (subject, newfile);
  lzg_subject_set_text (subject, newsubjecttext);

  if (newfile && newfile[0] != '/') {
    char *origin = NULL;
    
    if (newdirfd < 0) /* Includes AT_FDCWD */ {
      origin = get_current_dir_name();
    } else {
      origin = malloc (100 * sizeof(char));
      if (origin)
        snprintf (origin, 100, "fd: %d", newdirfd);
    }
    lzg_subject_set_origin(subject, origin);
    free (origin);
  }
  lzg_event_add_subject(event, subject);
  lzg_log_insert_event(log, event);
}

int starts_with(const char *string, const char *prefix)
{
  size_t lenpre = strlen(prefix), lenstr = strlen(string);
  return lenstr < lenpre ? 0 : strncmp (prefix, string, lenpre) == 0;
}

static int is_forbidden_file(const char *file)
{
  if(!file)
    return 1;

  const char *home = getenv("HOME");
  if(!home)
    home = "/usr";
  
  char *banned[] = {
    LZG_TARGET_DIR,
    ".cache/",
    NULL
  };

  int forbidden = 0, i;
  for (i=0; banned[i] && !forbidden; ++i) {
    size_t len = strlen(home) + 1 + strlen(banned[i]) + 1;
    char *full_path = malloc (sizeof (char) * len);
    if (full_path) {
      snprintf (full_path, len, "%s/%s", home, banned[i]);
      forbidden |= starts_with(file, full_path);
      free (full_path);
    }
  }

  return forbidden;
}

static int is_home(const char *file)
{
  return (file && file[0]=='/' && file[1]=='h' && file[2]=='o' && file[3]=='m' && file[4]=='e' && file[5]=='/');
}

static int is_tmp(const char *file)
{
  return (file && file[0]=='/' && file[1]=='t' && file[2]=='m' && file[3]=='p' && file[4]=='/');
}

static int is_relative(const char *file)
{
  return (file && file[0] != '\0' && file[0] != '/');
}

static int is_existent(const int ret)
{
  return ret != ENOENT;
}

static int is_open_for_writing(const int oflag)
{
  return oflag & (O_WRONLY | O_RDWR);
}

void _open (const int ret, const char *interpretation, int creates, int dirfd, const char *file, int oflag)
{
  if (
         (geteuid() >= 1000)                                                                     /* Limit the performance hit on service processes */
      && (creates || is_existent (ret))                                                          /* Filter out vain searches in PATH and LD_LIBRARY_PATH */
      && (is_open_for_writing (oflag) || is_home (file) || is_tmp (file) || is_relative (file) ) /* We don't care about /etc, /usr... */
      && (!is_forbidden_file (file))                                                             /* Our log files in ~/.local/share/... are off-limits */
     )
  {
    char error[1024], *error_str = NULL;
    if (errno) {
      error_str = strerror_r (errno, error, 1024);
    }

    size_t len = 24 /* oflag and ret */ + 200 /* snprintf format string + some extra for safety */ + 1024 /* error_str string */;
    char *open_txt = malloc (sizeof (char) * len);
    if (open_txt) {
      snprintf (open_txt, len, "fd %d: with flag %d, returned %s", ret, oflag, (ret<0? error_str:"no error"));
      log_event(open_txt, file, dirfd, interpretation);
      free (open_txt);
    }
    fds = lzg_slist_prepend(fds, INT_TO_POINTER(ret));
  }
}

int open (const char *file, int oflag, ...)
{
  va_list list;
  va_start(list, oflag);
  mode_t momo = va_arg(list, int);
  va_end(list);

  typeof(open) *original_open = dlsym(RTLD_NEXT, "open");
  int ret = (*original_open)(file, oflag, momo);
  _open(ret, OPEN_SCI, O_CREAT & oflag, -1, file, oflag);
  return ret;
}

int open64 (const char *file, int oflag, ...)
{
  va_list list;
  va_start(list, oflag);
  mode_t momo = va_arg(list, int);
  va_end(list);

  typeof(open64) *original_open = dlsym(RTLD_NEXT, "open64");
  int ret = (*original_open)(file, oflag, momo);
   _open(ret, OPEN64_SCI, O_CREAT & oflag, -1, file, oflag | O_LARGEFILE);
  va_end(list);
  return ret;
}

int openat (int dirfd, const char *file, int oflag, ...)
{
  va_list list;
  va_start(list, oflag);
  mode_t momo = va_arg(list, int);
  va_end(list);

  typeof(openat) *original_open = dlsym(RTLD_NEXT, "openat");
  int ret = (*original_open)(dirfd, file, oflag, momo);
  _open(ret, OPENAT_SCI, O_CREAT & oflag, dirfd, file, oflag);
  return ret;
}

int openat64 (int dirfd, const char *file, int oflag, ...)
{
  va_list list;
  va_start(list, oflag);
  mode_t momo = va_arg(list, int);
  va_end(list);

  typeof(openat64) *original_open = dlsym(RTLD_NEXT, "openat64");
  int ret = (*original_open)(dirfd, file, oflag, momo);
  _open(ret, OPENAT64_SCI, O_CREAT & oflag, dirfd, file, oflag | O_LARGEFILE);
  return ret;
}

int creat (const char *pathname, mode_t mode)
{
  typeof(creat) *original_open = dlsym(RTLD_NEXT, "creat");
  int ret = (*original_open)(pathname, mode);
  _open(ret, CREAT_SCI, 1, -1, pathname, O_CREAT|O_WRONLY|O_TRUNC);
  return ret;
}





void _dup (const int ret, const char *interpretation, int oldfd, int newfd, mode_t mode)
{
  if(lzg_slist_find(fds, INT_TO_POINTER(oldfd))) {
    char error[1024], *error_str = NULL;
    if (errno) {
      error_str = strerror_r (errno, error, 1024);
    }

    size_t plen = 40, dup_len = 200+1024;
    char *oldpath = malloc (sizeof(char)*plen), *newpath = malloc (sizeof(char)*plen), *dup_txt = malloc (sizeof(char)*dup_len);
    snprintf (oldpath, plen, "fd: %d", oldfd);
    snprintf (newpath, plen, "fd: %d", newfd);
    snprintf (dup_txt, dup_len, "New fd: returned %s", (ret? error_str:"no error"));
    log_old_new_event ("Old fd", oldpath, -1, dup_txt, newpath, -1, interpretation);
    free (oldpath);
    free (newpath);
    free (dup_txt);
  }
}

int dup(int oldfd)
{
  typeof(dup) *original_dup = dlsym(RTLD_NEXT, "dup");
  int ret = (*original_dup)(oldfd);

  int newfd = ret;
  _dup (ret, DUP_SCI, oldfd, newfd, 0);

  return ret;
}

int dup2(int oldfd, int newfd)
{
  typeof(dup2) *original_dup = dlsym(RTLD_NEXT, "dup2");
  int ret = (*original_dup)(oldfd, newfd);
  _dup (ret, DUP2_SCI, oldfd, newfd, 0);

  return ret;
}

int dup3(int oldfd, int newfd, int flags)
{
  typeof(dup3) *original_dup = dlsym(RTLD_NEXT, "dup3");
  int ret = (*original_dup)(oldfd, newfd, flags);
  _dup (ret, DUP3_SCI, oldfd, newfd, flags);

  return ret;
}

void _link (const int ret, const char *interpretation,
            const char *oldpath, const int olddirfd,
            const char *newpath, const int newdirfd, int flags)
{
  if (
         (geteuid() >= 1000)                                               /* Limit the performance hit on service processes */
      && (is_home (oldpath) || is_tmp (oldpath) || is_relative (oldpath) ||
          is_home (newpath) || is_tmp (newpath) || is_relative (newpath))  /* We don't care about /etc, /usr... */
     )
  {
    char error[1024], *error_str = NULL;
    if (errno) {
      error_str = strerror_r (errno, error, 1024);
    }

    size_t new_len = 200+1024;
    char *new_txt = malloc (sizeof(char)*new_len);
    snprintf (new_txt, new_len, "with flag %d, returned %s", flags, (ret<0? error_str:"no error"));
    log_old_new_event ("", oldpath, olddirfd, new_txt, newpath, newdirfd, interpretation);
    free (new_txt);
  }
}

int link(const char *oldpath, const char *newpath)
{
  typeof(link) *original_link = dlsym(RTLD_NEXT, "link");
  int ret = (*original_link)(oldpath, newpath);
  _link (ret, LINK_SCI, oldpath, -1, newpath, -1, 0);
  return ret;
}

int linkat(int olddirfd, const char *oldpath,
           int newdirfd, const char *newpath, int flags)
{
  typeof(linkat) *original_link = dlsym(RTLD_NEXT, "linkat");
  int ret = (*original_link)(olddirfd, oldpath, newdirfd, newpath, flags);
  _link (ret, LINKAT_SCI, oldpath, -1, newpath, -1, flags);
  return ret;
}

int symlink(const char *target, const char *newpath)
{
  typeof(symlink) *original_symlink = dlsym(RTLD_NEXT, "symlink");
  int ret = (*original_symlink)(target, newpath);
  _link (ret, SYMLINK_SCI, target, -1, newpath, -1, 0);
  return ret;
}

int symlinkat(const char *target, int newdirfd, const char *linkpath)
{
  typeof(symlinkat) *original_symlink = dlsym(RTLD_NEXT, "symlinkat");
  int ret = (*original_symlink)(target, newdirfd, linkpath);
  _link (ret, SYMLINKAT_SCI, target, -1, linkpath, newdirfd, 0);
  return ret;
}

static int _translate_fopen_mode(const char *mode)
{
  const char *c = mode;
  int flag = 0, rw = 0, ro = 0, wo = 0;
  while (*c) {
    if (*c=='+') {
      rw = 1;
    } else if (*c=='w') {
      flag |= O_CREAT | O_TRUNC;
      wo = 1;
    } else if (*c=='a') {
      flag |= O_CREAT | O_APPEND;
      wo = 1;
    } else if (*c=='r') {
      ro = 1;
    }
    ++c;
  }

  if (rw) flag |= O_RDWR;
  else if (ro) flag |= O_RDONLY;
  else if (wo) flag |= O_WRONLY;

  return flag;
}

static void _fopen(FILE *ret, const char *path, const char *mode, const char *interpretation, int is_command)
{
  int flag = _translate_fopen_mode(mode);

  if((geteuid() >= 1000) 
     && (is_command || ((is_open_for_writing (flag) || is_home (path) || is_tmp (path) || is_relative (path))
                        && !is_forbidden_file (path) )
        )
    ) {
    char error[1024], *error_str = NULL;
    if (errno) {
      error_str = strerror_r (errno, error, 1024);
    }

    size_t len = 12 /* flag */ + 200 /* snprintf format string + some extra for safety */ + 1024 /* error_str string */;
    char *open_txt = malloc (sizeof (char) * len);
    if (open_txt) {
      snprintf (open_txt, len, "FILE %p: with flag %d, returned %s", ret, flag, (ret? "no error":error_str));
      files = lzg_slist_prepend(files, ret);
      log_event(open_txt, path, -1, interpretation);
      free (open_txt);
    }
  }
}

FILE *fopen(const char *path, const char *mode)
{
  typeof(fopen) *original_open = dlsym(RTLD_NEXT, "fopen");
  FILE *ret = (*original_open)(path, mode);
  _fopen (ret, path, mode, FOPEN_SCI, 0);
  return ret;
}

FILE *freopen(const char *path, const char *mode, FILE *stream)
{
  typeof(freopen) *original_open = dlsym(RTLD_NEXT, "freopen");
  FILE *ret = (*original_open)(path, mode, stream);
  _fopen (ret, path, mode, FREOPEN_SCI, 0);
  return ret;
}

FILE *fdopen(int fd, const char *mode)
{
  typeof(fdopen) *original_open = dlsym(RTLD_NEXT, "fdopen");
  FILE *ret = (*original_open)(fd, mode);

  if(lzg_slist_find(fds, INT_TO_POINTER(fd))) {
    char error[1024], *error_str = NULL;
    if (errno) {
      error_str = strerror_r (errno, error, 1024);
    }

    int flag = _translate_fopen_mode(mode);

    size_t len = 12 /* flag */ + 200 /* snprintf format string + some extra for safety */ + 1024 /* error_str string */;
    size_t plen = 80;
    char *open_txt = malloc (sizeof (char) * len);
    char *file = malloc (sizeof(char)*plen);
    char *old_fd = malloc (sizeof(char)*plen);
    if (open_txt && file && old_fd) {
      snprintf (old_fd, plen, "fd: %d", fd);
      snprintf (file, plen, "FILE %p", ret);
      snprintf (open_txt, len, "with flag %d, returned %s", flag, (ret? "no error":error_str));
      files = lzg_slist_prepend(files, ret);
      log_old_new_event ("", old_fd, -1, open_txt, file, -1, FDOPEN_SCI);
      free (open_txt);
      free (old_fd);
      free (file);
    }
  }

  return ret;
}

int mkfifo(const char *pathname, mode_t mode)
{
  typeof(mkfifo) *original_mkfifo = dlsym(RTLD_NEXT, "mkfifo");
  int ret = (*original_mkfifo)(pathname, mode);
  _open(ret, MKFIFO_SCI, 1, -1, pathname, 0);
  return ret;
}

int mkfifoat(int dirfd, const char *pathname, mode_t mode)
{
  typeof(mkfifoat) *original_mkfifo = dlsym(RTLD_NEXT, "mkfifoat");
  int ret = (*original_mkfifo)(dirfd, pathname, mode);
  _open(ret, MKFIFOAT_SCI, 1, dirfd, pathname, 0);
  return ret;
}

void _pipe(int ret, int pipefd[2], int flags, const char *interpretation)
{
  if (!ret) // don't fool around with a potentially NULL pipefd, we don't care about the types of errors that might occur anyway
    return;

  if ((geteuid() >= 1000)) {
    size_t len = 50;
    char *p0_txt = malloc (sizeof (char) * len);
    char *p1_txt = malloc (sizeof (char) * len);
    if (p0_txt && p1_txt) {
      snprintf (p0_txt, len, "read fd %d", pipefd[0]);
      snprintf (p1_txt, len, "write fd %d", pipefd[1]);
      log_old_new_event("", p0_txt, -1, "", p1_txt, -1, interpretation);
      free (p0_txt);
      free (p1_txt);
    }
    fds = lzg_slist_prepend(fds, INT_TO_POINTER(pipefd[0]));
    fds = lzg_slist_prepend(fds, INT_TO_POINTER(pipefd[1]));
  }
}

int pipe2(int pipefd[2], int flags)
{
  typeof(pipe2) *original_pipe2 = dlsym(RTLD_NEXT, "pipe2");
  int ret = (*original_pipe2)(pipefd, flags);
  _pipe(ret, pipefd, flags, PIPE2_SCI);
  return ret;
}

int pipe(int pipefd[2])
{
  typeof(pipe) *original_pipe = dlsym(RTLD_NEXT, "pipe");
  int ret = (*original_pipe)(pipefd);
  _pipe(ret, pipefd, 0, PIPE_SCI);
  return ret;
}

int socketpair(int domain, int type, int protocol, int sv[2])
{
  typeof(socketpair) *original_socket = dlsym(RTLD_NEXT, "socketpair");
  int ret = (*original_socket)(domain, type, protocol, sv);

  if ((geteuid() >= 1000) && (domain == AF_UNIX || domain == AF_LOCAL) && (errno!=EFAULT)) {
    char error[1024], *error_str = NULL;
    if (errno) {
      error_str = strerror_r (errno, error, 1024);
    }
    
    size_t len = 50;
    size_t txtlen = 600 + 1024 /* error_str string */;
    char *p0_txt = malloc (sizeof (char) * len);
    char *open_txt = malloc (sizeof (char) * txtlen);
    if (p0_txt && open_txt) {
      snprintf (p0_txt, len, "socket %d", sv[0]);
      snprintf (open_txt, txtlen, "socket %d: with domain %d, type %d, protocol %d, returned %s", sv[1], domain, type, protocol, (ret<0? error_str:"no error"));
      log_old_new_event("", p0_txt, -1, "", open_txt, -1, SOCKETPAIR_SCI);
      free (p0_txt);
      free (open_txt);
    }
    fds = lzg_slist_prepend(fds, INT_TO_POINTER(sv[0]));
    fds = lzg_slist_prepend(fds, INT_TO_POINTER(sv[1]));
  }

  return ret;
}

FILE *popen(const char *command, const char *type)
{
  typeof(popen) *original_open = dlsym(RTLD_NEXT, "popen");
  FILE *ret = (*original_open)(command, type);
  _fopen (ret, command, type, POPEN_SCI, 1);
  return ret;
}

pid_t fork(void)
{
  typeof(fork) *original_fork = dlsym(RTLD_NEXT, "fork");
  pid_t ret = (*original_fork)();

  // Reset the log for the child
  if (!ret)
    lzg_log_get_default(1);

  if (ret)
  {
    char error[1024], *error_str = NULL;
    if (errno) {
      error_str = strerror_r (errno, error, 1024);
    }
    
    size_t len = 24;
    size_t txtlen = 100 + 1024 /* error_str string */;
    char *pid_txt = malloc (sizeof (char) * len);
    char *err_txt = malloc (sizeof (char) * txtlen);
    if (pid_txt && err_txt) {
      snprintf (pid_txt, len, "pid %d", ret);
      snprintf (err_txt, txtlen, "returned %s", (ret<0? error_str:"no error"));
      log_event(err_txt, pid_txt, -1, FORK_SCI);
      free (pid_txt);
      free (err_txt);
    }
  }

  return ret;
}

DIR *opendir(const char *name)
{
  typeof(opendir) *original_open = dlsym(RTLD_NEXT, "opendir");
  DIR *ret = (*original_open)(name);

  if((geteuid() >= 1000) && (is_home (name) || is_tmp (name) || is_relative (name))) {
    char error[1024], *error_str = NULL;
    if (errno) {
      error_str = strerror_r (errno, error, 1024);
    }

    size_t len = 12 /* flag */ + 200 /* snprintf format string + some extra for safety */ + 1024 /* error_str string */;
    char *open_txt = malloc (sizeof (char) * len);
    if (open_txt) {
      snprintf (open_txt, len, "DIR %p: returned %s", ret, (ret? "no error":error_str));
      dirs = lzg_slist_prepend(dirs, ret);
      log_event(open_txt, name, -1, OPENDIR_SCI);
      free (open_txt);
    }
  }

  return ret;
}

DIR *fdopendir(int fd)
{
  typeof(fdopendir) *original_open = dlsym(RTLD_NEXT, "fdopendir");
  DIR *ret = (*original_open)(fd);

  if(lzg_slist_find(fds, INT_TO_POINTER(fd))) {
    char error[1024], *error_str = NULL;
    if (errno) {
      error_str = strerror_r (errno, error, 1024);
    }

    size_t len = 200 /* snprintf format string + some extra for safety */ + 1024 /* error_str string */;
    size_t plen = 80;
    char *open_txt = malloc (sizeof (char) * len);
    char *file = malloc (sizeof(char)*plen);
    char *old_fd = malloc (sizeof(char)*plen);
    if (open_txt && file && old_fd) {
      snprintf (old_fd, plen, "fd: %d", fd);
      snprintf (file, plen, "DIR %p", ret);
      snprintf (open_txt, len, "returned %s", (ret? "no error":error_str));
      dirs = lzg_slist_prepend(dirs, ret);
      log_old_new_event ("", old_fd, -1, open_txt, file, -1, FDOPENDIR_SCI);
      free (open_txt);
      free (old_fd);
      free (file);
    }
  }

  return ret;
}

int shm_open(const char *name, int oflag, mode_t mode)
{
  typeof(shm_open) *original_shm_open = dlsym(RTLD_NEXT, "shm_open");
  int ret = (*original_shm_open)(name, oflag, mode);

  if (geteuid() >= 1000)
  {
    char error[1024], *error_str = NULL;
    if (errno) {
      error_str = strerror_r (errno, error, 1024);
    }

    size_t len = 300 + 1024 /* error_str string */;
    char *open_txt = malloc (sizeof (char) * len);
    if (open_txt) {
      snprintf (open_txt, len, "shm %d: with flag %d and mode %d returned %s", ret, oflag, mode, (ret<0? error_str:"no error"));
      log_event(open_txt, name, -1, SHM_OPEN_SCI);
      free (open_txt);
    }
  }
  
  return ret;
}

int shm_unlink(const char *name)
{
 typeof(shm_unlink) *original_shm_unlink = dlsym(RTLD_NEXT, "shm_unlink");
  int ret = (*original_shm_unlink)(name);

  if (geteuid() >= 1000)
  {
    char error[1024], *error_str = NULL;
    if (errno) {
      error_str = strerror_r (errno, error, 1024);
    }

    size_t len = 300 + 1024 /* error_str string */;
    char *open_txt = malloc (sizeof (char) * len);
    if (open_txt) {
      snprintf (open_txt, len, "shm: returned %s", (ret<0? error_str:"no error"));
      log_event(open_txt, name, -1, SHM_UNLINK_SCI);
      free (open_txt);
    }
  }
  
  return ret;
}

int mkdir(const char *pathname, mode_t mode)
{
  typeof(mkdir) *original_mkdir = dlsym(RTLD_NEXT, "mkdir");
  int ret = (*original_mkdir)(pathname, mode);
  
  _open(ret, MKDIR_SCI, 1, -1, pathname, O_CREAT);
  return ret;
}

int mkdirat(int dirfd, const char *pathname, mode_t mode)
{
  typeof(mkdirat) *original_mkdir = dlsym(RTLD_NEXT, "mkdirat");
  int ret = (*original_mkdir)(dirfd, pathname, mode);
  
  _open(ret, MKDIRAT_SCI, 1, dirfd, pathname, O_CREAT);
  return ret;
}

void _rename(int ret, const char *oldpath, const int olddirfd, const char *newpath, const int newdirfd, const int flags, const char *interpretation)
{
  if(( is_home (oldpath) || is_tmp (oldpath) || is_relative (oldpath) ||  is_home (newpath) || is_tmp (newpath) || is_relative (newpath) ) /* We don't care about /etc, /usr... */
      && !(is_forbidden_file (oldpath) || is_forbidden_file (newpath)) ) {
    char error[1024], *error_str = NULL;
    if (errno) {
      error_str = strerror_r (errno, error, 1024);
    }

    size_t newlen = 100 + 1024;
    char *newtxt = malloc (sizeof(char)*newlen);
    snprintf (newtxt, newlen, "New file: with flags %d, returned %s", flags, (ret? error_str:"no error"));
    log_old_new_event ("Old file", oldpath, olddirfd, newtxt, newpath, newdirfd, interpretation);
    free (newtxt);
  }
}

int rename(const char *oldpath, const char *newpath)
{
  typeof(rename) *original_rename = dlsym(RTLD_NEXT, "rename");
  int ret = (*original_rename)(oldpath, newpath);
  
  _rename(ret, oldpath, -1, newpath, -1, 0, RENAME_SCI);
  return ret;
}

int renameat(int olddirfd, const char *oldpath,
            int newdirfd, const char *newpath)
{
  typeof(renameat) *original_rename = dlsym(RTLD_NEXT, "renameat");
  int ret = (*original_rename)(olddirfd, oldpath, newdirfd, newpath);
  
  _rename(ret, oldpath, olddirfd, newpath, newdirfd, 0, RENAMEAT_SCI);
  return ret;
}

int renameat2(int olddirfd, const char *oldpath,
             int newdirfd, const char *newpath, unsigned int flags)
{
  typeof(renameat2) *original_rename = dlsym(RTLD_NEXT, "renameat2");
  int ret = (*original_rename)(olddirfd, oldpath, newdirfd, newpath, flags);
  
  _rename(ret, oldpath, olddirfd, newpath, newdirfd, flags, RENAMEAT2_SCI);
  return ret;
}



int close (int fd)
{
  typeof(close) *original_close = dlsym(RTLD_NEXT, "close");
  int ret = (*original_close)(fd);
  
  if(lzg_slist_find(fds, INT_TO_POINTER(fd))) {
    char error[1024], *error_str = NULL;
    if (errno) {
      error_str = strerror_r (errno, error, 1024);
    }
    
    char *path = malloc(sizeof(char) * 200);
    snprintf(path, 200, "fd: %d", fd);
    size_t close_len = 200 + 1024;
    char *close_txt = malloc (sizeof(char) * close_len);
    snprintf (close_txt, close_len, "returned %s", (ret? error_str:"no error"));
    log_event (close_txt, path, -1, CLOSE_SCI);
    free (close_txt);
    free (path);

    fds = lzg_slist_remove(fds, INT_TO_POINTER(fd));
  }
  
  return ret;
}

void _fclose (int ret, FILE *fp, const char *interpretation)
{
  if(lzg_slist_find(files, fp)) {
    char error[1024], *error_str = NULL;
    if (errno) {
      error_str = strerror_r (errno, error, 1024);
    }
    
    char *path = malloc(sizeof(char) * 200);
    snprintf(path, 200, "FILE: %p", fp);
    size_t close_len = 200 + 1024;
    char *close_txt = malloc (sizeof(char) * close_len);
    snprintf (close_txt, close_len, "returned %s", (ret? error_str:"no error"));
    log_event (close_txt, path, -1, interpretation);
    free (close_txt);
    free (path);

    files = lzg_slist_remove(files, fp);
  }
}

int fclose (FILE *fp)
{
  typeof(fclose) *original_fclose = dlsym(RTLD_NEXT, "fclose");
  int ret = (*original_fclose)(fp);
  _fclose (ret, fp, FCLOSE_SCI);
  return ret;
}

int pclose (FILE *fp)
{
  typeof(pclose) *original_pclose = dlsym(RTLD_NEXT, "pclose");
  int ret = (*original_pclose)(fp);
  _fclose (ret /* not actual fs error... */, fp, PCLOSE_SCI);
  return ret;
}

int closedir(DIR *dirp)
{
  typeof(closedir) *original_closedir = dlsym(RTLD_NEXT, "closedir");
  int ret = (*original_closedir)(dirp);
  if(lzg_slist_find(dirs, dirp)) {
    char error[1024], *error_str = NULL;
    if (errno) {
      error_str = strerror_r (errno, error, 1024);
    }

    char *path = malloc(sizeof(char) * 200);
    snprintf(path, 200, "DIR: %p", dirp);
    size_t close_len = 200 + 1024;
    char *close_txt = malloc (sizeof(char) * close_len);
    snprintf (close_txt, close_len, "returned %s", (ret? error_str:"no error"));
    log_event (close_txt, path, -1, CLOSEDIR_SCI);
    free (close_txt);
    free (path);

    dirs = lzg_slist_remove(dirs, dirp);
  }

  return ret;
}


int socket(int domain, int type, int protocol)
{
  typeof(socket) *original_socket = dlsym(RTLD_NEXT, "socket");
  int ret = (*original_socket)(domain, type, protocol);

  if ((geteuid() >= 1000) && (domain == AF_UNIX || domain == AF_LOCAL)) {
    char error[1024], *error_str = NULL;
    if (errno) {
      error_str = strerror_r (errno, error, 1024);
    }

    size_t len = 600 + 1024 /* error_str string */;
    char *open_txt = malloc (sizeof (char) * len);
    if (open_txt) {
      snprintf (open_txt, len, "socket %d: with domain %d, type %d, protocol %d, returned %s", ret, domain, type, protocol, (ret<0? error_str:"no error"));
      log_event(open_txt, "socket", -1, SOCKET_SCI);
      free (open_txt);
    }
    fds = lzg_slist_prepend(fds, INT_TO_POINTER(ret));
  }

  return ret;
}




void _rm (int ret, const char *pathname, const char *interpretation)
{
  if (
         (geteuid() >= 1000)               /* Limit the performance hit on service processes */
      && (!is_forbidden_file (pathname))   /* Our log files in ~/.local/share/... are off-limits */
     )
  {
    char error[1024], *error_str = NULL;
    if (errno) {
      error_str = strerror_r (errno, error, 1024);
    }

    size_t len = 200 /* snprintf format string + some extra for safety */ + 1024 /* error_str string */;
    char *rm_txt = malloc (sizeof (char) * len);
    if (rm_txt) {
      snprintf (rm_txt, len, "returned %s", (ret<0? error_str:"no error"));
      log_event(rm_txt, pathname, -1, interpretation);
      free (rm_txt);
    }
  }
}

int remove (const char *pathname)
{
  typeof(remove) *original_remove = dlsym(RTLD_NEXT, "remove");
  int ret = (*original_remove)(pathname);

  _rm (ret, pathname, REMOVE_SCI);
  return ret;
}

int rmdir (const char *pathname)
{
  typeof(rmdir) *original_rmdir = dlsym(RTLD_NEXT, "rmdir");
  int ret = (*original_rmdir)(pathname);

  _rm (ret, pathname, RMDIR_SCI);
  return ret;
}

int unlink (const char *pathname)
{
  typeof(unlink) *original_unlink = dlsym(RTLD_NEXT, "unlink");
  int ret = (*original_unlink)(pathname);

  _rm (ret, pathname, UNLINK_SCI);
  return ret;
}

