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
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "logger.h"
#include "gslist.h"

PrelogSList *fds = NULL;
PrelogSList *files = NULL;
PrelogSList *dirs = NULL;

static pthread_mutex_t _prelog_fd_lock = PTHREAD_MUTEX_INITIALIZER;

//TODO dbus API?

static void prelog_log_event(const char *syscall_text,
                     const char *file,
                     const int dirfd,
                     const char *event_interpretation)
{
  if (!file || !syscall_text || !event_interpretation)
    return;

  PrelogLog *log = prelog_log_get_default(PRELOG_LOG_DONT_RESET);
  if (!log) return;

  PrelogEvent *event = prelog_event_new ();
  if (!event) return;
  
  prelog_event_set_interpretation (event, event_interpretation);
  
  PrelogSubject *subject = prelog_subject_new ();
  if (!subject) {
    prelog_event_free (event);
    return;
  }
  prelog_subject_set_uri (subject, file);
  prelog_subject_set_text (subject, syscall_text);

  if (file && file[0] != '/') {
    char *origin = NULL;

    if (dirfd < 0) /* Includes AT_FDCWD */ {
      origin = get_current_dir_name();
    } else {
      origin = malloc (100 * sizeof(char));
      if (origin)
        snprintf (origin, 100, "fd: %d", dirfd);
    }
    prelog_subject_set_origin(subject, origin);
    free (origin);
  }

  prelog_event_add_subject(event, subject);
  prelog_log_insert_event(log, event);
}

static void prelog_log_old_new_event(const char *oldsubjecttext, const char *oldfile, const int olddirfd,
                              const char *newsubjecttext, const char *newfile, const int newdirfd,
                              const char *event_interpretation)
{
  if (!oldfile || !newfile || !oldsubjecttext || !newsubjecttext || !event_interpretation)
    return;

  PrelogLog *log = prelog_log_get_default(0);
  if (!log) return;

  PrelogEvent *event = prelog_event_new ();
  if (!event) return;

  prelog_event_set_interpretation (event, event_interpretation);
  
  PrelogSubject *subject = prelog_subject_new ();
  if (!subject) {
    prelog_event_free (event);
    return;
  }
  prelog_subject_set_uri (subject, oldfile);
  prelog_subject_set_text (subject, oldsubjecttext);

  if (oldfile && oldfile[0] != '/') {
    char *origin = NULL;
    
    if (olddirfd < 0) /* Includes AT_FDCWD */ {
      origin = get_current_dir_name();
    } else {
      origin = malloc (100 * sizeof(char));
      if (origin)
        snprintf (origin, 100, "fd: %d", olddirfd);
    }
    prelog_subject_set_origin(subject, origin);
    free (origin);
  }
  prelog_event_add_subject(event, subject);
  
  subject = prelog_subject_new ();
  if (!subject) {
    prelog_event_free (event); // will clear the added subject
    return;
  }
  prelog_subject_set_uri (subject, newfile);
  prelog_subject_set_text (subject, newsubjecttext);

  if (newfile && newfile[0] != '/') {
    char *origin = NULL;
    
    if (newdirfd < 0) /* Includes AT_FDCWD */ {
      origin = get_current_dir_name();
    } else {
      origin = malloc (100 * sizeof(char));
      if (origin)
        snprintf (origin, 100, "fd: %d", newdirfd);
    }
    prelog_subject_set_origin(subject, origin);
    free (origin);
  }
  prelog_event_add_subject(event, subject);
  prelog_log_insert_event(log, event);
}

int prelog_starts_with(const char *string, const char *prefix)
{
  size_t lenpre = strlen(prefix), lenstr = strlen(string);
  return lenstr < lenpre ? 0 : strncmp (prefix, string, lenpre) == 0;
}

static int prelog_is_forbidden_file(const char *file)
{
  if(!file)
    return 1;

  const char *home = getenv("HOME");
  if(!home)
    home = "/usr";
  
  char *banned[] = {
    PRELOG_TARGET_DIR,
    ".cache/",
    NULL
  };

  int forbidden = 0, i;
  for (i=0; banned[i] && !forbidden; ++i) {
    size_t len = strlen(home) + 1 + strlen(banned[i]) + 1;
    char *full_path = malloc (sizeof (char) * len);
    if (full_path) {
      snprintf (full_path, len, "%s/%s", home, banned[i]);
      forbidden |= prelog_starts_with(file, full_path);
      free (full_path);
    }
  }

  return forbidden;
}

static int prelog_is_home(const char *file)
{
  return (file && file[0]=='/' && file[1]=='h' && file[2]=='o' && file[3]=='m' && file[4]=='e' && file[5]=='/');
}

static int prelog_is_tmp(const char *file)
{
  return (file && file[0]=='/' && file[1]=='t' && file[2]=='m' && file[3]=='p' && file[4]=='/');
}

static int prelog_is_relative(const char *file)
{
  return (file && file[0] != '\0' && file[0] != '/');
}

static int prelog_is_existent(const int ret)
{
  return ret != ENOENT;
}

static int prelog_is_open_for_writing(const int oflag)
{
  return oflag & (O_WRONLY | O_RDWR);
}

void prelog_open (const int ret, const char *interpretation, int creates, int dirfd, const char *file, int oflag)
{
  if (
         (geteuid() >= 1000)                                                                     /* Limit the performance hit on service processes */
      && (creates || prelog_is_existent (ret))                                                          /* Filter out vain searches in PATH and LD_LIBRARY_PATH */
      && (prelog_is_open_for_writing (oflag) || prelog_is_home (file) || prelog_is_tmp (file) || prelog_is_relative (file) ) /* We don't care about /etc, /usr... */
      && (!prelog_is_forbidden_file (file))                                                             /* Our log files in ~/.local/share/... are off-limits */
     )
  {
    char *error_str = NULL;//, error[1024];
    if (errno) {
      //error_str = strerror_r (errno, error, 1024);
      error_str = malloc (26);
      snprintf (error_str, 26, "e%d", errno);
    }

    size_t len = 24 /* oflag and ret */ + 200 /* snprintf format string + some extra for safety */ + 1024 /* error_str string */;
    char *open_txt = malloc (sizeof (char) * len);
    if (open_txt) {
      snprintf (open_txt, len, "fd %d: with flag %d, %s", ret, oflag, (ret<0? error_str:"e0"));
      prelog_log_event(open_txt, file, dirfd, interpretation);
      free (open_txt);
    }
    pthread_mutex_lock(&_prelog_fd_lock);
    fds = prelog_slist_prepend(fds, PRELOG_INT_TO_POINTER(ret));
    pthread_mutex_unlock(&_prelog_fd_lock);
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
  int saved_errno = errno;
  prelog_open(ret, OPEN_SCI, O_CREAT & oflag, -1, file, oflag);
  errno = saved_errno;
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
  int saved_errno = errno;
   prelog_open(ret, OPEN64_SCI, O_CREAT & oflag, -1, file, oflag | O_LARGEFILE);
  errno = saved_errno;
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
  int saved_errno = errno;
  prelog_open(ret, OPENAT_SCI, O_CREAT & oflag, dirfd, file, oflag);
  errno = saved_errno;
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
  int saved_errno = errno;
  prelog_open(ret, OPENAT64_SCI, O_CREAT & oflag, dirfd, file, oflag | O_LARGEFILE);
  errno = saved_errno;
  return ret;
}

int creat (const char *pathname, mode_t mode)
{
  typeof(creat) *original_open = dlsym(RTLD_NEXT, "creat");
  int ret = (*original_open)(pathname, mode);
  int saved_errno = errno;
  prelog_open(ret, CREAT_SCI, 1, -1, pathname, O_CREAT|O_WRONLY|O_TRUNC);
  errno = saved_errno;
  return ret;
}





void prelog_dup (const int ret, const char *interpretation, int oldfd, int newfd, mode_t mode)
{
  pthread_mutex_lock(&_prelog_fd_lock);
  if(prelog_slist_find(fds, PRELOG_INT_TO_POINTER(oldfd))) {
    char *error_str = NULL;//, error[1024];
    if (errno) {
      //error_str = strerror_r (errno, error, 1024);
      error_str = malloc (26);
      snprintf (error_str, 26, "e%d", errno);
    }

    size_t plen = 40, dup_len = 200+1024;
    char *oldpath = malloc (sizeof(char)*plen), *newpath = malloc (sizeof(char)*plen), *dup_txt = malloc (sizeof(char)*dup_len);
    snprintf (oldpath, plen, "fd: %d", oldfd);
    snprintf (newpath, plen, "fd: %d", newfd);
    snprintf (dup_txt, dup_len, "New fd: %s", (ret? error_str:"e0"));
    prelog_log_old_new_event ("Old fd", oldpath, -1, dup_txt, newpath, -1, interpretation);
    free (oldpath);
    free (newpath);
    free (dup_txt);
  }
  pthread_mutex_unlock(&_prelog_fd_lock);
}

int dup(int oldfd)
{
  typeof(dup) *original_dup = dlsym(RTLD_NEXT, "dup");
  int ret = (*original_dup)(oldfd);
  int saved_errno = errno;

  int newfd = ret;
  prelog_dup (ret, DUP_SCI, oldfd, newfd, 0);

  errno = saved_errno;
  return ret;
}

int dup2(int oldfd, int newfd)
{
  typeof(dup2) *original_dup = dlsym(RTLD_NEXT, "dup2");
  int ret = (*original_dup)(oldfd, newfd);
  int saved_errno = errno;
  prelog_dup (ret, DUP2_SCI, oldfd, newfd, 0);

  errno = saved_errno;
  return ret;
}

int dup3(int oldfd, int newfd, int flags)
{
  typeof(dup3) *original_dup = dlsym(RTLD_NEXT, "dup3");
  int ret = (*original_dup)(oldfd, newfd, flags);
  int saved_errno = errno;
  prelog_dup (ret, DUP3_SCI, oldfd, newfd, flags);

  errno = saved_errno;
  return ret;
}

void prelog_link (const int ret, const char *interpretation,
            const char *oldpath, const int olddirfd,
            const char *newpath, const int newdirfd, int flags)
{
  if (
         (geteuid() >= 1000)                                               /* Limit the performance hit on service processes */
      && (prelog_is_home (oldpath) || prelog_is_tmp (oldpath) || prelog_is_relative (oldpath) ||
          prelog_is_home (newpath) || prelog_is_tmp (newpath) || prelog_is_relative (newpath))  /* We don't care about /etc, /usr... */
     )
  {
    char *error_str = NULL;//, error[1024];
    if (errno) {
      //error_str = strerror_r (errno, error, 1024);
      error_str = malloc (26);
      snprintf (error_str, 26, "e%d", errno);
    }

    size_t new_len = 200+1024;
    char *new_txt = malloc (sizeof(char)*new_len);
    snprintf (new_txt, new_len, "with flag %d, %s", flags, (ret<0? error_str:"e0"));
    prelog_log_old_new_event ("", oldpath, olddirfd, new_txt, newpath, newdirfd, interpretation);
    free (new_txt);
  }
}

int link(const char *oldpath, const char *newpath)
{
  typeof(link) *original_link = dlsym(RTLD_NEXT, "link");
  int ret = (*original_link)(oldpath, newpath);
  int saved_errno = errno;
  prelog_link (ret, LINK_SCI, oldpath, -1, newpath, -1, 0);
  errno = saved_errno;
  return ret;
}

int linkat(int olddirfd, const char *oldpath,
           int newdirfd, const char *newpath, int flags)
{
  typeof(linkat) *original_link = dlsym(RTLD_NEXT, "linkat");
  int ret = (*original_link)(olddirfd, oldpath, newdirfd, newpath, flags);
  int saved_errno = errno;
  prelog_link (ret, LINKAT_SCI, oldpath, -1, newpath, -1, flags);
  errno = saved_errno;
  return ret;
}

int symlink(const char *target, const char *newpath)
{
  typeof(symlink) *original_symlink = dlsym(RTLD_NEXT, "symlink");
  int ret = (*original_symlink)(target, newpath);
  int saved_errno = errno;
  prelog_link (ret, SYMLINK_SCI, target, -1, newpath, -1, 0);
  errno = saved_errno;
  return ret;
}

int symlinkat(const char *target, int newdirfd, const char *linkpath)
{
  typeof(symlinkat) *original_symlink = dlsym(RTLD_NEXT, "symlinkat");
  int ret = (*original_symlink)(target, newdirfd, linkpath);
  int saved_errno = errno;
  prelog_link (ret, SYMLINKAT_SCI, target, -1, linkpath, newdirfd, 0);
  errno = saved_errno;
  return ret;
}

static int prelog_translate_fopen_mode(const char *mode)
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

static void prelog_fopen(FILE *ret, const char *path, const char *mode, const char *interpretation, int is_command)
{
  int flag = prelog_translate_fopen_mode(mode);

  if((geteuid() >= 1000) 
     && (is_command || ((prelog_is_open_for_writing (flag) || prelog_is_home (path) || prelog_is_tmp (path) || prelog_is_relative (path))
                        && !prelog_is_forbidden_file (path) )
        )
    ) {
    char *error_str = NULL;//, error[1024];
    if (errno) {
      //error_str = strerror_r (errno, error, 1024);
      error_str = malloc (26);
      snprintf (error_str, 26, "e%d", errno);
    }

    size_t len = 12 /* flag */ + 200 /* snprintf format string + some extra for safety */ + 1024 /* error_str string */;
    char *open_txt = malloc (sizeof (char) * len);
    if (open_txt) {
      snprintf (open_txt, len, "FILE %p: with flag %d, %s", ret, flag, (ret? "e0":error_str));
      pthread_mutex_lock(&_prelog_fd_lock);
      files = prelog_slist_prepend(files, ret);
      pthread_mutex_unlock(&_prelog_fd_lock);
      prelog_log_event(open_txt, path, -1, interpretation);
      free (open_txt);
    }
  }
}

FILE *fopen(const char *path, const char *mode)
{
  typeof(fopen) *original_open = dlsym(RTLD_NEXT, "fopen");
  FILE *ret = (*original_open)(path, mode);
  int saved_errno = errno;
  prelog_fopen (ret, path, mode, FOPEN_SCI, 0);
  errno = saved_errno;
  return ret;
}

FILE *freopen(const char *path, const char *mode, FILE *stream)
{
  typeof(freopen) *original_open = dlsym(RTLD_NEXT, "freopen");
  FILE *ret = (*original_open)(path, mode, stream);
  int saved_errno = errno;
  prelog_fopen (ret, path, mode, FREOPEN_SCI, 0);
  errno = saved_errno;
  return ret;
}

FILE *fdopen(int fd, const char *mode)
{
  typeof(fdopen) *original_open = dlsym(RTLD_NEXT, "fdopen");
  FILE *ret = (*original_open)(fd, mode);
  int saved_errno = errno;
  pthread_mutex_lock(&_prelog_fd_lock);
  if(prelog_slist_find(fds, PRELOG_INT_TO_POINTER(fd))) {
    char *error_str = NULL;//, error[1024];
    if (errno) {
      //error_str = strerror_r (errno, error, 1024);
      error_str = malloc (26);
      snprintf (error_str, 26, "e%d", errno);
    }

    int flag = prelog_translate_fopen_mode(mode);

    size_t len = 12 /* flag */ + 200 /* snprintf format string + some extra for safety */ + 1024 /* error_str string */;
    size_t plen = 80;
    char *open_txt = malloc (sizeof (char) * len);
    char *file = malloc (sizeof(char)*plen);
    char *old_fd = malloc (sizeof(char)*plen);
    if (open_txt && file && old_fd) {
      snprintf (old_fd, plen, "fd: %d", fd);
      snprintf (file, plen, "FILE %p", ret);
      snprintf (open_txt, len, "with flag %d, %s", flag, (ret? "e0":error_str));
      files = prelog_slist_prepend(files, ret);
      prelog_log_old_new_event ("", old_fd, -1, open_txt, file, -1, FDOPEN_SCI);
      free (open_txt);
      free (old_fd);
      free (file);
    }
  }
  pthread_mutex_unlock(&_prelog_fd_lock);

  errno = saved_errno;
  return ret;
}

int mkfifo(const char *pathname, mode_t mode)
{
  typeof(mkfifo) *original_mkfifo = dlsym(RTLD_NEXT, "mkfifo");
  int ret = (*original_mkfifo)(pathname, mode);
  int saved_errno = errno;
  prelog_open(ret, MKFIFO_SCI, 1, -1, pathname, 0);
  errno = saved_errno;
  return ret;
}

int mkfifoat(int dirfd, const char *pathname, mode_t mode)
{
  typeof(mkfifoat) *original_mkfifo = dlsym(RTLD_NEXT, "mkfifoat");
  int ret = (*original_mkfifo)(dirfd, pathname, mode);
  int saved_errno = errno;
  prelog_open(ret, MKFIFOAT_SCI, 1, dirfd, pathname, 0);
  errno = saved_errno;
  return ret;
}

void prelog_pipe(int ret, int pipefd[2], int flags, const char *interpretation)
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
      prelog_log_old_new_event("", p0_txt, -1, "", p1_txt, -1, interpretation);
      free (p0_txt);
      free (p1_txt);
    }
    pthread_mutex_lock(&_prelog_fd_lock);
    fds = prelog_slist_prepend(fds, PRELOG_INT_TO_POINTER(pipefd[0]));
    fds = prelog_slist_prepend(fds, PRELOG_INT_TO_POINTER(pipefd[1]));
    pthread_mutex_unlock(&_prelog_fd_lock);
  }
}

int pipe2(int pipefd[2], int flags)
{
  typeof(pipe2) *original_pipe2 = dlsym(RTLD_NEXT, "pipe2");
  int ret = (*original_pipe2)(pipefd, flags);
  int saved_errno = errno;
  prelog_pipe(ret, pipefd, flags, PIPE2_SCI);
  errno = saved_errno;
  return ret;
}

int pipe(int pipefd[2])
{
  typeof(pipe) *original_pipe = dlsym(RTLD_NEXT, "pipe");
  int ret = (*original_pipe)(pipefd);
  int saved_errno = errno;
  prelog_pipe(ret, pipefd, 0, PIPE_SCI);
  errno = saved_errno;
  return ret;
}

int socketpair(int domain, int type, int protocol, int sv[2])
{
  typeof(socketpair) *original_socket = dlsym(RTLD_NEXT, "socketpair");
  int ret = (*original_socket)(domain, type, protocol, sv);
  int saved_errno = errno;

  if ((geteuid() >= 1000) && (domain == AF_UNIX || domain == AF_LOCAL) && (errno!=EFAULT)) {
    char *error_str = NULL;//, error[1024];
    if (errno) {
      //error_str = strerror_r (errno, error, 1024);
      error_str = malloc (26);
      snprintf (error_str, 26, "e%d", errno);
    }
    
    size_t len = 50;
    size_t txtlen = 600 + 1024 /* error_str string */;
    char *p0_txt = malloc (sizeof (char) * len);
    char *open_txt = malloc (sizeof (char) * txtlen);
    if (p0_txt && open_txt) {
      snprintf (p0_txt, len, "socket %d", sv[0]);
      snprintf (open_txt, txtlen, "socket %d: with domain %d, type %d, protocol %d, %s", sv[1], domain, type, protocol, (ret<0? error_str:"e0"));
      prelog_log_old_new_event("", p0_txt, -1, "", open_txt, -1, SOCKETPAIR_SCI);
      free (p0_txt);
      free (open_txt);
    }
    pthread_mutex_lock(&_prelog_fd_lock);
    fds = prelog_slist_prepend(fds, PRELOG_INT_TO_POINTER(sv[0]));
    fds = prelog_slist_prepend(fds, PRELOG_INT_TO_POINTER(sv[1]));
    pthread_mutex_unlock(&_prelog_fd_lock);
  }

  errno = saved_errno;
  return ret;
}

FILE *popen(const char *command, const char *type)
{
  typeof(popen) *original_open = dlsym(RTLD_NEXT, "popen");
  FILE *ret = (*original_open)(command, type);
  int saved_errno = errno;
  prelog_fopen (ret, command, type, POPEN_SCI, 1);
  errno = saved_errno;
  return ret;
}

pid_t fork(void)
{
  typeof(fork) *original_fork = dlsym(RTLD_NEXT, "fork");
  pid_t ret = (*original_fork)();
  int saved_errno = errno;

  // Reset the log for the child
  if (ret == 0)
  {
    prelog_log_get_default(PRELOG_LOG_RESET_FORK);
  }
  else 
  {
    char *error_str = NULL;//, error[1024];
    if (errno) {
      //error_str = strerror_r (errno, error, 1024);
      error_str = malloc (26);
      snprintf (error_str, 26, "e%d", errno);
    }
    
    size_t len = 24;
    size_t txtlen = 100 + 1024 /* error_str string */;
    char *pid_txt = malloc (sizeof (char) * len);
    char *err_txt = malloc (sizeof (char) * txtlen);
    if (pid_txt && err_txt) {
      snprintf (pid_txt, len, "pid %d", ret);
      snprintf (err_txt, txtlen, "%s", (ret<0? error_str:"e0"));
      prelog_log_event(err_txt, pid_txt, -1, FORK_SCI);
      free (pid_txt);
      free (err_txt);
    }
  }

  errno = saved_errno;
  return ret;
}

DIR *opendir(const char *name)
{
  typeof(opendir) *original_open = dlsym(RTLD_NEXT, "opendir");
  DIR *ret = (*original_open)(name);
  int saved_errno = errno;

  if((geteuid() >= 1000) && (prelog_is_home (name) || prelog_is_tmp (name) || prelog_is_relative (name))) {
    char *error_str = NULL;//, error[1024];
    if (errno) {
      //error_str = strerror_r (errno, error, 1024);
      error_str = malloc (26);
      snprintf (error_str, 26, "e%d", errno);
    }

    size_t len = 12 /* flag */ + 200 /* snprintf format string + some extra for safety */ + 1024 /* error_str string */;
    char *open_txt = malloc (sizeof (char) * len);
    if (open_txt) {
      snprintf (open_txt, len, "DIR %p: %s", ret, (ret? "e0":error_str));
      pthread_mutex_lock(&_prelog_fd_lock);
      dirs = prelog_slist_prepend(dirs, ret);
      pthread_mutex_unlock(&_prelog_fd_lock);
      prelog_log_event(open_txt, name, -1, OPENDIR_SCI);
      free (open_txt);
    }
  }

  errno = saved_errno;
  return ret;
}

DIR *fdopendir(int fd)
{
  typeof(fdopendir) *original_open = dlsym(RTLD_NEXT, "fdopendir");
  DIR *ret = (*original_open)(fd);
  int saved_errno = errno;

  pthread_mutex_lock(&_prelog_fd_lock);
  if(prelog_slist_find(fds, PRELOG_INT_TO_POINTER(fd))) {
    char *error_str = NULL;//, error[1024];
    if (errno) {
      //error_str = strerror_r (errno, error, 1024);
      error_str = malloc (26);
      snprintf (error_str, 26, "e%d", errno);
    }

    size_t len = 200 /* snprintf format string + some extra for safety */ + 1024 /* error_str string */;
    size_t plen = 80;
    char *open_txt = malloc (sizeof (char) * len);
    char *file = malloc (sizeof(char)*plen);
    char *old_fd = malloc (sizeof(char)*plen);
    if (open_txt && file && old_fd) {
      snprintf (old_fd, plen, "fd: %d", fd);
      snprintf (file, plen, "DIR %p", ret);
      snprintf (open_txt, len, "%s", (ret? "e0":error_str));
      dirs = prelog_slist_prepend(dirs, ret);
      prelog_log_old_new_event ("", old_fd, -1, open_txt, file, -1, FDOPENDIR_SCI);
      free (open_txt);
      free (old_fd);
      free (file);
    }
  }
  pthread_mutex_unlock(&_prelog_fd_lock);

  errno = saved_errno;
  return ret;
}

int shm_open(const char *name, int oflag, mode_t mode)
{
  typeof(shm_open) *original_shm_open = dlsym(RTLD_NEXT, "shm_open");

  if (!original_shm_open)
    original_shm_open = dlsym(dlopen("librt.so", RTLD_NOW), "shm_open");

  if (!original_shm_open)
  {
    fprintf(stderr, "PreloadLogger Error: no symbol could be found for function 'shm_open'. Your application is probably loading C symbols in a very peculiar way. Aborting call to shm_open...\n");
    errno = EACCES;
    return -1;
  }

  int ret = (*original_shm_open)(name, oflag, mode);
  int saved_errno = errno;

  if (geteuid() >= 1000)
  {
    char *error_str = NULL;//, error[1024];
    if (errno) {
      //error_str = strerror_r (errno, error, 1024);
      error_str = malloc (26);
      snprintf (error_str, 26, "e%d", errno);
    }

    size_t len = 300 + 1024 /* error_str string */;
    char *open_txt = malloc (sizeof (char) * len);
    if (open_txt) {
      snprintf (open_txt, len, "shm %d: with flag %d and mode %d: %s", ret, oflag, mode, (ret<0? error_str:"e0"));
      prelog_log_event(open_txt, name, -1, SHM_OPEN_SCI);
      free (open_txt);
    }
  }
  
  errno = saved_errno;
  return ret;
}

int shm_unlink(const char *name)
{
  typeof(shm_unlink) *original_shm_unlink = dlsym(RTLD_NEXT, "shm_unlink");

  if (!original_shm_unlink)
    original_shm_unlink = dlsym(dlopen("librt.so", RTLD_NOW), "shm_unlink");

  if (!original_shm_unlink)
  {
    fprintf(stderr, "PreloadLogger Error: no symbol could be found for function 'shm_unlink'. Your application is probably loading C symbols in a very peculiar way. Aborting call to shm_unlink...\n");
    errno = EACCES;
    return -1;
  }

  int ret = (*original_shm_unlink)(name);
  int saved_errno = errno;

  if (geteuid() >= 1000)
  {
    char *error_str = NULL;//, error[1024];
    if (errno) {
      //error_str = strerror_r (errno, error, 1024);
      error_str = malloc (26);
      snprintf (error_str, 26, "e%d", errno);
    }

    size_t len = 300 + 1024 /* error_str string */;
    char *open_txt = malloc (sizeof (char) * len);
    if (open_txt) {
      snprintf (open_txt, len, "shm: %s", (ret<0? error_str:"e0"));
      prelog_log_event(open_txt, name, -1, SHM_UNLINK_SCI);
      free (open_txt);
    }
  }
  
  errno = saved_errno;
  return ret;
}

int mkdir(const char *pathname, mode_t mode)
{
  typeof(mkdir) *original_mkdir = dlsym(RTLD_NEXT, "mkdir");
  int ret = (*original_mkdir)(pathname, mode);
  int saved_errno = errno;
  
  prelog_open(ret, MKDIR_SCI, 1, -1, pathname, O_CREAT);

  errno = saved_errno;
  return ret;
}

int mkdirat(int dirfd, const char *pathname, mode_t mode)
{
  typeof(mkdirat) *original_mkdir = dlsym(RTLD_NEXT, "mkdirat");
  int ret = (*original_mkdir)(dirfd, pathname, mode);
  int saved_errno = errno;
  
  prelog_open(ret, MKDIRAT_SCI, 1, dirfd, pathname, O_CREAT);

  errno = saved_errno;
  return ret;
}

void prelog_rename(int ret, const char *oldpath, const int olddirfd, const char *newpath, const int newdirfd, const int flags, const char *interpretation)
{
  if(( prelog_is_home (oldpath) || prelog_is_tmp (oldpath) || prelog_is_relative (oldpath) ||  prelog_is_home (newpath) || prelog_is_tmp (newpath) || prelog_is_relative (newpath) ) /* We don't care about /etc, /usr... */
      && !(prelog_is_forbidden_file (oldpath) || prelog_is_forbidden_file (newpath)) ) {
    char *error_str = NULL;//, error[1024];
    if (errno) {
      //error_str = strerror_r (errno, error, 1024);
      error_str = malloc (26);
      snprintf (error_str, 26, "e%d", errno);
    }

    size_t newlen = 100 + 1024;
    char *newtxt = malloc (sizeof(char)*newlen);
    snprintf (newtxt, newlen, "New file: with flags %d, %s", flags, (ret? error_str:"e0"));
    prelog_log_old_new_event ("Old file", oldpath, olddirfd, newtxt, newpath, newdirfd, interpretation);
    free (newtxt);
  }
}

int rename(const char *oldpath, const char *newpath)
{
  typeof(rename) *original_rename = dlsym(RTLD_NEXT, "rename");
  int ret = (*original_rename)(oldpath, newpath);
  int saved_errno = errno;
  
  prelog_rename(ret, oldpath, -1, newpath, -1, 0, RENAME_SCI);
  errno = saved_errno;
  return ret;
}

int renameat(int olddirfd, const char *oldpath,
            int newdirfd, const char *newpath)
{
  typeof(renameat) *original_rename = dlsym(RTLD_NEXT, "renameat");
  int ret = (*original_rename)(olddirfd, oldpath, newdirfd, newpath);
  int saved_errno = errno;
  
  prelog_rename(ret, oldpath, olddirfd, newpath, newdirfd, 0, RENAMEAT_SCI);
  errno = saved_errno;
  return ret;
}

int renameat2(int olddirfd, const char *oldpath,
             int newdirfd, const char *newpath, unsigned int flags)
{
  typeof(renameat2) *original_rename = dlsym(RTLD_NEXT, "renameat2");
  int ret = (*original_rename)(olddirfd, oldpath, newdirfd, newpath, flags);
  int saved_errno = errno;
  
  prelog_rename(ret, oldpath, olddirfd, newpath, newdirfd, flags, RENAMEAT2_SCI);
  errno = saved_errno;
  return ret;
}



int close (int fd)
{
  typeof(close) *original_close = dlsym(RTLD_NEXT, "close");
  int ret = (*original_close)(fd);
  int saved_errno = errno;
  
  pthread_mutex_lock(&_prelog_fd_lock);
  if(prelog_slist_find(fds, PRELOG_INT_TO_POINTER(fd))) {
    char *error_str = NULL;//, error[1024];
    if (errno) {
      //error_str = strerror_r (errno, error, 1024);
      error_str = malloc (26);
      snprintf (error_str, 26, "e%d", errno);
    }
    
    char *path = malloc(sizeof(char) * 200);
    snprintf(path, 200, "fd: %d", fd);
    size_t close_len = 200 + 1024;
    char *close_txt = malloc (sizeof(char) * close_len);
    snprintf (close_txt, close_len, "%s", (ret? error_str:"e0"));
    prelog_log_event (close_txt, path, -1, CLOSE_SCI);
    free (close_txt);
    free (path);

    fds = prelog_slist_remove(fds, PRELOG_INT_TO_POINTER(fd));
  }
  pthread_mutex_unlock(&_prelog_fd_lock);
  
  errno = saved_errno;
  return ret;
}

void prelog_fclose (int ret, FILE *fp, const char *interpretation)
{
  pthread_mutex_lock(&_prelog_fd_lock);
  if(prelog_slist_find(files, fp)) {
    char *error_str = NULL;//, error[1024];
    if (errno) {
      //error_str = strerror_r (errno, error, 1024);
      error_str = malloc (26);
      snprintf (error_str, 26, "e%d", errno);
    }
    
    char *path = malloc(sizeof(char) * 200);
    snprintf(path, 200, "FILE: %p", fp);
    size_t close_len = 200 + 1024;
    char *close_txt = malloc (sizeof(char) * close_len);
    snprintf (close_txt, close_len, "%s", (ret? error_str:"e0"));
    prelog_log_event (close_txt, path, -1, interpretation);
    free (close_txt);
    free (path);

    files = prelog_slist_remove(files, fp);
  }
  pthread_mutex_unlock(&_prelog_fd_lock);
}

int fclose (FILE *fp)
{
  typeof(fclose) *original_fclose = dlsym(RTLD_NEXT, "fclose");
  int ret = (*original_fclose)(fp);
  int saved_errno = errno;
  prelog_fclose (ret, fp, FCLOSE_SCI);
  errno = saved_errno;
  return ret;
}

int pclose (FILE *fp)
{
  typeof(pclose) *original_pclose = dlsym(RTLD_NEXT, "pclose");
  int ret = (*original_pclose)(fp);
  int saved_errno = errno;
  prelog_fclose (ret /* not actual fs error... */, fp, PCLOSE_SCI);
  errno = saved_errno;
  return ret;
}

int closedir(DIR *dirp)
{
  typeof(closedir) *original_closedir = dlsym(RTLD_NEXT, "closedir");
  int ret = (*original_closedir)(dirp);
  int saved_errno = errno;
  pthread_mutex_lock(&_prelog_fd_lock);
  if(prelog_slist_find(dirs, dirp)) {
    char *error_str = NULL;//, error[1024];
    if (errno) {
      //error_str = strerror_r (errno, error, 1024);
      error_str = malloc (26);
      snprintf (error_str, 26, "e%d", errno);
    }

    char *path = malloc(sizeof(char) * 200);
    snprintf(path, 200, "DIR: %p", dirp);
    size_t close_len = 200 + 1024;
    char *close_txt = malloc (sizeof(char) * close_len);
    snprintf (close_txt, close_len, "%s", (ret? error_str:"e0"));
    prelog_log_event (close_txt, path, -1, CLOSEDIR_SCI);
    free (close_txt);
    free (path);

    dirs = prelog_slist_remove(dirs, dirp);
  }
  pthread_mutex_unlock(&_prelog_fd_lock);

  errno = saved_errno;
  return ret;
}


int socket(int domain, int type, int protocol)
{
  typeof(socket) *original_socket = dlsym(RTLD_NEXT, "socket");
  int ret = (*original_socket)(domain, type, protocol);
  int saved_errno = errno;

  if ((geteuid() >= 1000) && (domain == AF_UNIX || domain == AF_LOCAL)) {
    char *error_str = NULL;//, error[1024];
    if (errno) {
      //error_str = strerror_r (errno, error, 1024);
      error_str = malloc (26);
      snprintf (error_str, 26, "e%d", errno);
    }

    size_t len = 600 + 1024 /* error_str string */;
    char *open_txt = malloc (sizeof (char) * len);
    if (open_txt) {
      snprintf (open_txt, len, "socket %d: with domain %d, type %d, protocol %d, %s", ret, domain, type, protocol, (ret<0? error_str:"e0"));
      prelog_log_event(open_txt, "socket", -1, SOCKET_SCI);
      free (open_txt);
    }
    pthread_mutex_lock(&_prelog_fd_lock);
    fds = prelog_slist_prepend(fds, PRELOG_INT_TO_POINTER(ret));
    pthread_mutex_unlock(&_prelog_fd_lock);
  }

  errno = saved_errno;
  return ret;
}




void prelog_rm (int ret, const char *pathname, const char *interpretation)
{
  if (
         (geteuid() >= 1000)               /* Limit the performance hit on service processes */
      && (!prelog_is_forbidden_file (pathname))   /* Our log files in ~/.local/share/... are off-limits */
     )
  {
    char *error_str = NULL;//, error[1024];
    if (errno) {
      //error_str = strerror_r (errno, error, 1024);
      error_str = malloc (26);
      snprintf (error_str, 26, "e%d", errno);
    }

    size_t len = 200 /* snprintf format string + some extra for safety */ + 1024 /* error_str string */;
    char *rm_txt = malloc (sizeof (char) * len);
    if (rm_txt) {
      snprintf (rm_txt, len, "%s", (ret<0? error_str:"e0"));
      prelog_log_event(rm_txt, pathname, -1, interpretation);
      free (rm_txt);
    }
  }
}

int remove (const char *pathname)
{
  typeof(remove) *original_remove = dlsym(RTLD_NEXT, "remove");
  int ret = (*original_remove)(pathname);
  int saved_errno = errno;

  prelog_rm (ret, pathname, REMOVE_SCI);
  errno = saved_errno;
  return ret;
}

int rmdir (const char *pathname)
{
  typeof(rmdir) *original_rmdir = dlsym(RTLD_NEXT, "rmdir");
  int ret = (*original_rmdir)(pathname);
  int saved_errno = errno;

  prelog_rm (ret, pathname, RMDIR_SCI);
  errno = saved_errno;
  return ret;
}

int unlink (const char *pathname)
{
  typeof(unlink) *original_unlink = dlsym(RTLD_NEXT, "unlink");
  int ret = (*original_unlink)(pathname);
  int saved_errno = errno;

  prelog_rm (ret, pathname, UNLINK_SCI);
  errno = saved_errno;
  return ret;
}

