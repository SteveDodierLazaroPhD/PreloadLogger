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
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "logger.h"

static int _prelog_exit_registered = 0;

char *__pl_strndup (const char *s, size_t n)
{
  if (!s)
    return NULL;

  size_t len = strnlen (s, n);
  char *new = (char *) __libc_malloc (len + 1);

  if (new == NULL)
    return NULL;

  new[len] = '\0';
  return (char *) memcpy (new, s, len);
}

char *__pl_strdup (const char *s)
{
  if (!s)
    return NULL;

  size_t len = strlen (s) + 1;
  void *new = __libc_malloc (len);

  if (new == NULL)
    return NULL;

  return (char *) memcpy (new, s, len);
}

char *prelog_get_actor_from_pid (pid_t pid)
{
  static char *cached = NULL;
  static pid_t cached_pid = 0;
  
  if (cached_pid && cached_pid != pid) {
    if (cached) {
      __libc_free (cached);
      cached = NULL;
    }
  }

  if (cached) {
    return __pl_strdup (cached);
  }

  char         *link_file     = NULL;
  char         *link_target   = NULL;
  char         *split_target  = NULL;
  char         *actor_name    = NULL;
  ssize_t       read_len      = PATH_MAX; // 4096, so it's unlikely link_len ever overflows :)
  ssize_t       link_len      = 1;

  if (pid <= 0)
  {
      cached = __pl_strdup ("unknown");
      return __pl_strdup (cached);
  }

  size_t len = strlen ("/proc//exe") + 100 + 1; //100 is much more than current pid_t's longest digit representation
  link_file = __libc_malloc (sizeof (char) * len);
  snprintf (link_file, len, "/proc/%d/exe", pid);
  if (link_file == NULL)
  {
      cached = __pl_strdup ("unknown");
      return __pl_strdup (cached);
  }

  // It is impossible to obtain the size of /proc link targets as /proc is
  // not POSIX compliant. Hence, we start from the NAME_MAX limit and increase
  // it all the way up to readlink failing. readlink will fail upon reaching
  // the PATH_MAX limit on Linux implementations. read_len will be strictly
  // inferior to link_len as soon as the latter is big enough to contain the
  // path to the executable and a trailing null character.
  while (read_len >= link_len)
  {
    link_len += NAME_MAX;

    __libc_free (link_target);
    link_target = __libc_malloc (link_len * sizeof (char));

    if (link_target == NULL)
    {
      __libc_free (link_file);
      __libc_free (link_target);
      cached = __pl_strdup ("unknown");
      return __pl_strdup (cached);
    }

    read_len= readlink (link_file, link_target, link_len);

    if (read_len < 0)
    {
      __libc_free (link_file);
      __libc_free (link_target);
      cached = __pl_strdup ("unknown");
      return __pl_strdup (cached);
    }
  }

  __libc_free (link_file);

  // readlink does not null-terminate the string
  link_target[read_len] = '\0';

  split_target = strrchr (link_target, '/');

  if(split_target == NULL)
  {
    __libc_free (link_target);
      cached = __pl_strdup ("unknown");
      return __pl_strdup (cached);
  }

  // Turn it into an arbitrary actor name
  size_t actor_len = strlen (split_target) + 1;
  actor_name = __libc_malloc (sizeof (char) * actor_len);
  snprintf (actor_name, actor_len, "%s", split_target+1);
  __libc_free (link_target);

  // Better safe than sorry
  if (!actor_name)
    actor_name = __pl_strdup ("unknown");

  cached = actor_name;
  return __pl_strdup(actor_name);
}

PrelogSubject *prelog_subject_new (void)
{
  PrelogSubject *s = __libc_malloc(sizeof(PrelogSubject));

  if(!s)
    return NULL;

  s->uri = NULL;
  s->origin = NULL;
  s->text = NULL;

  return s;
}

void prelog_subject_free (PrelogSubject *s)
{
  if(!s)
    return;

  if(s->uri)
    __libc_free(s->uri);
  if(s->origin)
    __libc_free(s->origin);

  __libc_free(s);
}

void prelog_subject_set_uri (PrelogSubject *s, const char *uri)
{
  if(!s)
    return;

  if(s->uri)
    __libc_free(s->uri);
  
  s->uri = __pl_strndup (uri, 8192);
}

void prelog_subject_set_origin (PrelogSubject *s, const char *origin)
{
  if(!s)
    return;

  if(s->origin)
    __libc_free(s->origin);
  
  s->origin = __pl_strndup (origin, 8192);
}

void prelog_subject_set_text (PrelogSubject *s, const char *text)
{
  if(!s)
    return;

  if(s->text)
    __libc_free(s->text);
  
  s->text = __pl_strndup (text, 8192);
}

PrelogEvent *prelog_event_new (void)
{
  PrelogEvent *e = __libc_malloc(sizeof(PrelogEvent));

  if(!e)
    return NULL;

  e->timestamp = time(NULL);
  e->subjects = NULL;
  e->interpretation = NULL;

  return e;
}

void prelog_event_free (PrelogEvent *e)
{
  if(!e)
    return;

  if(e->subjects) {
    unsigned int i=0;
    while(e->subjects[i]) {
      prelog_subject_free(e->subjects[i]);
      ++i;
    }
    __libc_free(e->subjects);
  }

  __libc_free(e);
}

void prelog_event_set_interpretation (PrelogEvent *e, const char *interpretation)
{
  if(!e)
    return;

  e->interpretation = __pl_strndup (interpretation, 8192);
}

void prelog_event_add_subject (PrelogEvent *e, PrelogSubject *s)
{
  if(!e || !s)
    return;
  
  size_t count = 0;
  if(e->subjects) {
    size_t i = 0;
    while(e->subjects[i])
      i++;
    
    count = i;
  }
  
  e->subjects = realloc(e->subjects, sizeof(PrelogSubject*) * (count + 2)); // count + new element + trailing NULL
  e->subjects[count] = s;
  e->subjects[count+1] = NULL;
}

void prelog_log_free (PrelogLog *log, PrelogLogResetFlag reset)
{
  if(!log)
    return;

  if (log->write_zfd != NULL) {
    /*typeof(close) *original_close;
    original_close = dlsym(RTLD_NEXT, "close");
    (*original_close) (log->write_fd);*/
    if (reset == PRELOG_LOG_RESET_FORK)
      prelog_gzclose_no_flush (log->write_zfd);
    else
      prelog_gzclose_w (log->write_zfd);
  }
  
  __libc_free (log);
  log = NULL;
}

static void prelog_mkdir (const char *dir)
{
  typeof(mkdir) *original_mkdir;
  original_mkdir = dlsym(RTLD_NEXT, "mkdir");
  
  char tmp[PATH_MAX];
  char *p = NULL;
  size_t len;

  snprintf(tmp, sizeof(tmp),"%s",dir);
  len = strlen(tmp);

  if(tmp[len - 1] == '/')
    tmp[len - 1] = 0;

  for(p = tmp + 1; *p; p++) {
    if(*p == '/') {
      *p = 0;
      (*original_mkdir)(tmp, S_IRWXU);
      *p = '/';
    }
  }

  (*original_mkdir)(tmp, S_IRWXU);
}

int prelog_log_allowed_to_log ()
{
  typeof(access) *original_access;
  original_access = dlsym(RTLD_NEXT, "access");
  
  const char *home = getenv("HOME");
  if(!home)
    return 0;

  int accessed = -1;
  size_t len = strlen(home) + 1 + strlen(PRELOG_TARGET_DIR) + 1 + strlen(PRELOG_LOG_FORBIDDEN) + 1;
  char *full_path = __libc_malloc (sizeof (char) * len);
  if (full_path) {
    snprintf (full_path, len, "%s/%s/%s", home, PRELOG_TARGET_DIR, PRELOG_LOG_FORBIDDEN);
    accessed = (*original_access) (full_path, F_OK);
    __libc_free (full_path);
  }

  if (accessed == 0)
    return 0;

  char *name = prelog_get_actor_from_pid (getpid());
  if (!name)
    return 0;

  len = strlen(home) + 1 + strlen(PRELOG_TARGET_DIR) + 1 + strlen(name) + 6;
  full_path = __libc_malloc (sizeof (char) * len);
  if (full_path) {
    snprintf (full_path, len, "%s/%s/%s.lock", home, PRELOG_TARGET_DIR, name);
    accessed = (*original_access) (full_path, F_OK);
    __libc_free (full_path);
    __libc_free (name);
  }

  return accessed;
}

void prelog_log_log_process_data (PrelogLog *log)
{
  if(log->write_zfd != NULL && prelog_log_allowed_to_log()) {
    pid_t pid = getpid();
    char *actor = prelog_get_actor_from_pid (pid);
    if (!actor)
      return;

    char *msg = NULL;
    size_t msg_len = 0;
    
    msg_len = strlen(actor) + 12 /* pid */ + 4 /* @ : \n \0 */;
    msg = __libc_malloc(sizeof (char) * msg_len);
    if (!msg) {
      __libc_free (actor);
      return;
    }

    snprintf(msg, msg_len, "@%s|%d\n", actor, pid);
    //write(log->write_fd, msg, strlen(msg));
    gzwrite(log->write_zfd, msg, strlen(msg));
    __libc_free(actor);
    __libc_free(msg);
  }
}

static void prelog_log_shutdown()
{
  prelog_log_get_default(PRELOG_LOG_RESET_SHUTDOWN);
}

PrelogLog *prelog_log_get_default (PrelogLogResetFlag reset)
{
  static PrelogLog *log = NULL;
  
  if (reset != PRELOG_LOG_DONT_RESET) {
    prelog_log_free (log, reset);
    log = NULL;

    if (reset == PRELOG_LOG_RESET_SHUTDOWN)
      return NULL;
  }

  if (geteuid() < 1000)
    return NULL;
  
  if (!log) {
    log = __libc_malloc(sizeof(PrelogLog));
    if(!log)
        return NULL;
    //log->write_fd = -1;
    log->write_zfd = NULL;

    /* Try to init write_fd / write_zfd */
    const char *env = getenv("HOME");
    if (env) {
      size_t elen = strlen (env) + 1 + strlen (PRELOG_TARGET_DIR) + 1;
      char *epath = __libc_malloc (sizeof (char) * elen);
      if (!epath) {
        __libc_free(log);
        log = NULL;
        return NULL;
      }
      snprintf (epath, elen, "%s/%s", env, PRELOG_TARGET_DIR);
      typeof(opendir) *original_opendir;
      original_opendir = dlsym(RTLD_NEXT, "opendir");
      
      DIR *exists = (*original_opendir) (epath);

      if (!exists) {
        prelog_mkdir (epath);
      } else {
        typeof(closedir) *original_closedir;
        original_closedir = dlsym(RTLD_NEXT, "closedir");
        (*original_closedir) (exists);
      }
      __libc_free (epath);

      time_t t = time(NULL);
      struct tm ttm;
      localtime_r(&t, &ttm);
      char date[100] = {0};
      if (!strftime(date, sizeof(date), "%Y-%m-%d_%H%M%S", &ttm))
        date[0] = '\0';

      size_t len = strlen (env) + 1/*/*/ + strlen (PRELOG_TARGET_DIR) + 1/*/*/ + strnlen(date, 100) + 1/*_*/ + 24/*pid*/ + 5/*.log+\0*/ + 3/*.gz*/;
      char *path = __libc_malloc (sizeof (char) * len);
      if (!path) {
        __libc_free(log);
        log = NULL;
        return NULL;
      }
      
      //snprintf (path, len, "%s/%s/%s_%d.log", env, PRELOG_TARGET_DIR, date, getpid());
      //typeof(open) *original_open;
      //original_open = dlsym(RTLD_NEXT, "open");
      //log->write_fd = (*original_open) (path, O_WRONLY | O_CREAT | O_APPEND, 00666);

      snprintf (path, len, "%s/%s/%s_%d.log.gz", env, PRELOG_TARGET_DIR, date, getpid());
      log->write_zfd = prelog_gzopen(path, "a");
      __libc_free (path);

      prelog_log_log_process_data(log);

      if (!_prelog_exit_registered) {
        _prelog_exit_registered = 1;
        atexit(prelog_log_shutdown);
      }
    }
  }

  return log;
}

void prelog_log_insert_event (PrelogLog *log, PrelogEvent *event)
{
  if(!log || !event)
    return;

  if (!event->interpretation) {
    //fprintf (stderr, "UCL study: trying to log event without interpretation, ignoring.\n");
    return;
  }


  char *msg = NULL;
  size_t msg_len = 0;
  
  msg_len = strlen(event->interpretation) + /* actually time_t is a long int so 12 would suffice */ + 24 /*timestamp*/ + 200 /* format string */;
  msg = __libc_malloc(sizeof (char) * msg_len);
  snprintf(msg, msg_len, "%li|%s%s",
                         event->timestamp,
                         event->interpretation,
                         (event->subjects && !event->subjects[1] ? "|":"\n"));//, event->manifestation);

  int i=0;
  while (event->subjects && event->subjects[i]) {
    PrelogSubject *s = event->subjects[i];

    char *old = __pl_strdup(msg);
    size_t slen = strlen(s->uri) + (s->origin ? strlen(s->origin):0) + (s->text ? strlen(s->text):0) + 200;
    msg = realloc (msg, strlen(old) + slen);
    snprintf(msg, strlen(old) + slen, "%s%s%s|%s|%s\n",
                                      old,
                                       (event->subjects && !event->subjects[1] ? "":" "),
                                      s->uri,
                                      s->text,
                                      s->origin? s->origin:"");
    __libc_free (old);
    ++i;
  }

  if(log->write_zfd != NULL && prelog_log_allowed_to_log()) {
    //write(log->write_fd, msg, strlen(msg));
    gzwrite(log->write_zfd, msg, strlen(msg));
  }
  
  __libc_free(msg);
  prelog_event_free (event);
}
