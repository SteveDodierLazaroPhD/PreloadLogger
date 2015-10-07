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

#ifndef	_LOGGER_H
#define	_LOGGER_H	1

#include <stdio.h>
#include "zlib/zlib.h"

#ifndef __GLIBC__
# error PreloadLogger works only with the glibc, because it needs to be able to perform direct calls to syscalls via __libc_*
#endif

extern int   __open(const char *pathname, int flags, ...);
extern int   __open64(const char *pathname, int flags, ...);
extern int   __close(int fd);

extern void* __libc_malloc(size_t size);
extern void  __libc_free(void* ptr);
extern void* __libc_realloc(void* ptr, size_t size);
extern void* __libc_calloc(size_t n, size_t size);
extern void  __libc_cfree(void* ptr);
extern void* __libc_memcpy(void *dest, const void *src, size_t n);
extern void* __libc_memalign(size_t align, size_t s);
extern void* __libc_valloc(size_t size);
extern char* __libc_strdup(const char *s);
extern char* __libc_strndup(const char *s, size_t n);
extern void* __libc_pvalloc(size_t size);
extern int   __posix_memalign(void** r, size_t a, size_t s);

typedef enum {
  PRELOG_LOG_DONT_RESET = 0,
  PRELOG_LOG_RESET_SHUTDOWN = 1,
  PRELOG_LOG_RESET_FORK = 2
} PrelogLogResetFlag;

/*
 * Copyright (C) 2010-2015 Canonical, Ltd.
 *           (C) 2015 Steve Dodier-Lazaro UNTIL INDICATED OTHERWISE BELOW
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see
 * <http://www.gnu.org/licenses/>.
 *
 * Originally authored by under above license
 *             Mikkel Kamstrup Erlandsen <mikkel.kamstrup@canonical.com>
 *
 * API below forked by, C code authored by
 *             Steve Dodier-Lazaro <sidnioulz@gmail.com>
 */

typedef struct _PrelogSubject {
  char              *uri;
  char              *origin;
  char              *text;
} PrelogSubject;

typedef struct _PrelogEvent {
  time_t             timestamp;
  char              *interpretation;
  PrelogSubject **subjects;
} PrelogEvent;

typedef struct _PrelogLog {
//  int                write_fd;
  gzFile             write_zfd;
} PrelogLog;

#define PRELOG_TARGET_DIR    ".local/share/zeitgeist"
#define PRELOG_TARGET_FILE   "syscalls.log"
#define PRELOG_LOG_FORBIDDEN "LOGGING-FORBIDDEN.lock"
#define PRELOG_TARGET_PATH    ".local/share/zeitgeist/syscalls.log"

char *__pl_strdup (const char *s);
char *__pl_strndup (const char *s, size_t n);

char *prelog_get_actor_from_pid (pid_t pid);

PrelogSubject *prelog_subject_new (void);
void prelog_subject_free (PrelogSubject *s);
void prelog_subject_set_uri (PrelogSubject *s, const char *uri);
void prelog_subject_set_origin (PrelogSubject *s, const char *origin);
void prelog_subject_set_text (PrelogSubject *s, const char *text);

PrelogEvent *prelog_event_new (void);
void prelog_event_free (PrelogEvent *e);
void prelog_event_set_interpretation (PrelogEvent *e, const char *interpretation);
void prelog_event_add_subject (PrelogEvent *e, PrelogSubject *s);

PrelogLog *prelog_log_get_default (PrelogLogResetFlag reset);
void prelog_log_insert_event (PrelogLog *log, PrelogEvent *event);


#define CREAT_SCI          "creat"
#define OPEN_SCI           "open"
#define OPENAT_SCI         "openat"
#define OPEN64_SCI         "open64"
#define OPENAT64_SCI       "openat64"
#define CLOSE_SCI          "close"
#define FOPEN_SCI          "fopen"
#define POPEN_SCI          "popen"
#define FREOPEN_SCI        "freopen"
#define FDOPEN_SCI         "fdopen"
#define FCLOSE_SCI         "fclose"
#define PCLOSE_SCI         "pclose"
#define FORK_SCI             "fork"
#define DUP_SCI              "dup"
#define DUP2_SCI             "dup2"
#define DUP3_SCI             "dup3"
#define LINK_SCI             "link"
#define LINKAT_SCI           "linkat"
#define SYMLINK_SCI          "symlink"
#define SYMLINKAT_SCI        "symlinkat"
#define UNLINK_SCI           "unlink"
#define RMDIR_SCI            "rmdir"
#define REMOVE_SCI           "remove"
#define SOCKET_SCI           "socket"
#define SOCKETPAIR_SCI       "socketpair"
#define MKFIFO_SCI           "mkfifo"
#define MKFIFOAT_SCI         "mkfifoat"
#define PIPE_SCI             "pipe"
#define PIPE2_SCI            "pipe2"
#define OPENDIR_SCI          "opendir"
#define FDOPENDIR_SCI        "fdopendir"
#define CLOSEDIR_SCI         "closedir"
#define MKDIR_SCI            "mkdir"
#define MKDIRAT_SCI          "mkdirat"
#define RENAME_SCI           "rename"
#define RENAMEAT_SCI         "renameat"
#define RENAMEAT2_SCI        "renameat2"
#define SHM_OPEN_SCI         "shm_open"
#define SHM_UNLINK_SCI       "shm_unlink"

#endif /* LOGGER.h  */
