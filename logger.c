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


char *_zg_get_actor_from_pid (pid_t pid)
{
  static char *cached = NULL;
  static pid_t cached_pid = 0;
  
  if (cached_pid && cached_pid != pid) {
    if (cached) {
      free (cached);
      cached = NULL;
    }
  }

  if (cached) {
    return strdup (cached);
  }

  char         *link_file     = NULL;
  char         *link_target   = NULL;
  char         *split_target  = NULL;
  char         *actor_name    = NULL;
  ssize_t       read_len      = PATH_MAX; // 4096, so it's unlikely link_len ever overflows :)
  ssize_t       link_len      = 1;

  if (pid <= 0)
  {
      cached = strdup ("unknown");
      return strdup (cached);
  }

  size_t len = strlen ("/proc//exe") + 100 + 1; //100 is much more than current pid_t's longest digit representation
  link_file = malloc (sizeof (char) * len);
  snprintf (link_file, len, "/proc/%d/exe", pid);
  if (link_file == NULL)
  {
      cached = strdup ("unknown");
      return strdup (cached);
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

    free (link_target);
    link_target = malloc (link_len * sizeof (char));

    if (link_target == NULL)
    {
      free (link_file);
      free (link_target);
      cached = strdup ("unknown");
      return strdup (cached);
    }

    read_len= readlink (link_file, link_target, link_len);

    if (read_len < 0)
    {
      free (link_file);
      free (link_target);
      cached = strdup ("unknown");
      return strdup (cached);
    }
  }

  free (link_file);

  // readlink does not null-terminate the string
  link_target[read_len] = '\0';

  split_target = strrchr (link_target, '/');

  if(split_target == NULL)
  {
    free (link_target);
      cached = strdup ("unknown");
      return strdup (cached);
  }

  // Turn it into an arbitrary actor name
  size_t actor_len = strlen (split_target) + 1;
  actor_name = malloc (sizeof (char) * actor_len);
  snprintf (actor_name, actor_len, "%s", split_target+1);
  free (link_target);

  // Better safe than sorry
  if (!actor_name)
    actor_name = strdup ("unknown");

  cached = actor_name;
  return strdup(actor_name);
}

LZGSubject *lzg_subject_new (void)
{
  LZGSubject *s = malloc(sizeof(LZGSubject));

  if(!s)
    return NULL;

  s->uri = NULL;
  s->origin = NULL;
  s->text = NULL;

  return s;
}

void lzg_subject_free (LZGSubject *s)
{
  if(!s)
    return;

  if(s->uri)
    free(s->uri);
  if(s->origin)
    free(s->origin);

  free(s);
}

void lzg_subject_set_uri (LZGSubject *s, const char *uri)
{
  if(!s)
    return;

  if(s->uri)
    free(s->uri);
  
  s->uri = strndup (uri, 8192);
}

void lzg_subject_set_origin (LZGSubject *s, const char *origin)
{
  if(!s)
    return;

  if(s->origin)
    free(s->origin);
  
  s->origin = strndup (origin, 8192);
}

void lzg_subject_set_text (LZGSubject *s, const char *text)
{
  if(!s)
    return;

  if(s->text)
    free(s->text);
  
  s->text = strndup (text, 8192);
}

LZGEvent *lzg_event_new (void)
{
  LZGEvent *e = malloc(sizeof(LZGEvent));

  if(!e)
    return NULL;

  e->timestamp = time(NULL);
  e->subjects = NULL;
  e->interpretation = NULL;

  return e;
}

void lzg_event_free (LZGEvent *e)
{
  if(!e)
    return;

  if(e->subjects) {
    unsigned int i=0;
    while(e->subjects[i]) {
      lzg_subject_free(e->subjects[i]);
      ++i;
    }
    free(e->subjects);
  }

  free(e);
}

void lzg_event_set_interpretation (LZGEvent *e, const char *interpretation)
{
  if(!e)
    return;

  e->interpretation = strndup (interpretation, 8192);
}

/*void lzg_event_set_manifestation (LZGEvent *e, const char *manifestation)
{
  if(!e)
    return;

  if(e->manifestation)
    free(e->manifestation);
  
  e->manifestation = strndup (manifestation, 8192);
}*/
void lzg_event_add_subject (LZGEvent *e, LZGSubject *s)
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
  
  e->subjects = realloc(e->subjects, sizeof(LZGSubject*) * (count + 2)); // count + new element + trailing NULL
  e->subjects[count] = s;
  e->subjects[count+1] = NULL;
}

void lzg_log_free (LZGLog *log)
{
  if (log->write_fd >= 0) {
    typeof(close) *original_close;
    original_close = dlsym(RTLD_NEXT, "close");
    (*original_close) (log->write_fd);
  }
  
  free (log);
}

static void _mkdir (const char *dir)
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

int lzg_log_allowed_to_log ()
{
  typeof(access) *original_access;
  original_access = dlsym(RTLD_NEXT, "access");
  
  const char *home = getenv("HOME");
  if(!home)
    return 0;

  int accessed = -1;
  size_t len = strlen(home) + 1 + strlen(LZG_TARGET_DIR) + 1 + strlen(LZG_LOG_FORBIDDEN) + 1;
  char *full_path = malloc (sizeof (char) * len);
  if (full_path) {
    snprintf (full_path, len, "%s/%s/%s", home, LZG_TARGET_DIR, LZG_LOG_FORBIDDEN);
    accessed = (*original_access) (full_path, F_OK);
    free (full_path);
  }

  if (accessed == 0)
    return 0;

  char *name = _zg_get_actor_from_pid (getpid());
  if (!name)
    return 0;

  len = strlen(home) + 1 + strlen(LZG_TARGET_DIR) + 1 + strlen(name) + 6;
  full_path = malloc (sizeof (char) * len);
  if (full_path) {
    snprintf (full_path, len, "%s/%s/%s.lock", home, LZG_TARGET_DIR, name);
    accessed = (*original_access) (full_path, F_OK);
    free (full_path);
    free (name);
  }

  return accessed;
}

void lzg_log_log_process_data (LZGLog *log)
{
  if(log->write_fd >= 0 && lzg_log_allowed_to_log()) {
    pid_t pid = getpid();
    char *actor = _zg_get_actor_from_pid (pid);
    if (!actor)
      return;

    char *msg = NULL;
    size_t msg_len = 0;
    
    msg_len = strlen(actor) + 12 /* pid */ + 4 /* @ : \n \0 */;
    msg = malloc(sizeof (char) * msg_len);
    if (!msg) {
      free (actor);
      return;
    }

    snprintf(msg, msg_len, "@%s|%d\n", actor, pid);
    write(log->write_fd, msg, strlen(msg));
    free(actor);
    free(msg);
  }
}

LZGLog *lzg_log_get_default (int reset)
{
  static LZGLog *log = NULL;
  
  if (reset) {
    lzg_log_free (log);
    log = NULL;
  }

  if (geteuid() < 1000)
    return NULL;
  
  if (!log) {
    log = malloc(sizeof(LZGLog));
    if(!log)
        return NULL;
    log->write_fd = -1;

    /* Try to init write_fd */
    const char *env = getenv("HOME");
    if (env) {
    
      size_t elen = strlen (env) + 1 + strlen (LZG_TARGET_DIR) + 1;
      char *epath = malloc (sizeof (char) * elen);
      if (!epath) {
        free(log);
        log = NULL;
        return NULL;
      }
      snprintf (epath, elen, "%s/%s", env, LZG_TARGET_DIR);
      typeof(opendir) *original_opendir;
      original_opendir = dlsym(RTLD_NEXT, "opendir");
      
      DIR *exists = (*original_opendir) (epath);

      if (!exists) {
        _mkdir (epath);
      } else {
        typeof(closedir) *original_closedir;
        original_closedir = dlsym(RTLD_NEXT, "closedir");
        (*original_closedir) (exists);
      }
      free (epath);

      size_t len = strlen (env) + 1 + strlen (LZG_TARGET_DIR) + 1 + 24 + 5;
      char *path = malloc (sizeof (char) * len);
      if (!path) {
        free(epath);
        free(log);
        log = NULL;
        return NULL;
      }
      
      time_t t = time(NULL);
      struct tm ttm;
      localtime_r(&t, &ttm);
      char date[100] = {0};

      strftime(date, sizeof(date), "%Y-%m-%d_%H%M%S", &ttm);
      snprintf (path, len, "%s/%s/%s_%d.log", env, LZG_TARGET_DIR, date, getpid());
      
      typeof(open) *original_open;
      original_open = dlsym(RTLD_NEXT, "open");
      log->write_fd = (*original_open) (path, O_WRONLY | O_CREAT | O_APPEND, 00666);
      lzg_log_log_process_data(log);
    }
  }

  return log;
}

void lzg_log_insert_event (LZGLog *log, LZGEvent *event)
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
  msg = malloc(sizeof (char) * msg_len);
  snprintf(msg, msg_len, "%li|%s%s",
                         event->timestamp,
                         event->interpretation,
                         (event->subjects && !event->subjects[1] ? "|":"\n"));//, event->manifestation);

  int i=0;
  while (event->subjects && event->subjects[i]) {
    LZGSubject *s = event->subjects[i];

    char *old = strdup(msg);
    size_t slen = strlen(s->uri) + (s->origin ? strlen(s->origin):0) + (s->text ? strlen(s->text):0) + 200;
    msg = realloc (msg, strlen(old) + slen);
    snprintf(msg, strlen(old) + slen, "%s%s%s|%s|%s\n",
                                      old,
                                       (event->subjects && !event->subjects[1] ? "":" "),
                                      s->uri,
                                      s->text,
                                      s->origin? s->origin:"");
    free (old);
    ++i;
  }

  if(log->write_fd >= 0 && lzg_log_allowed_to_log()) {
    write(log->write_fd, msg, strlen(msg));
  }
  
  free(msg);
  lzg_event_free (event);
}
