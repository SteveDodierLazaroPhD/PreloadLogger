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

/*
 * Copyright (C) 2010 Canonical, Ltd. UNTIL INDICATED OTHERWISE BELOW
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
 * Authored by
 *             Mikkel Kamstrup Erlandsen <mikkel.kamstrup@canonical.com>
 */

#define ZEITGEIST_NCAL_ALARM "http://www.semanticdesktop.org/ontologies/2007/04/02/ncal#Alarm"
#define ZEITGEIST_NCAL_CALENDAR "http://www.semanticdesktop.org/ontologies/2007/04/02/ncal#Calendar"
#define ZEITGEIST_NCAL_EVENT "http://www.semanticdesktop.org/ontologies/2007/04/02/ncal#Event"
#define ZEITGEIST_NCAL_FREEBUSY "http://www.semanticdesktop.org/ontologies/2007/04/02/ncal#Freebusy"
#define ZEITGEIST_NCAL_JOURNAL "http://www.semanticdesktop.org/ontologies/2007/04/02/ncal#Journal"
#define ZEITGEIST_NCAL_TIMEZONE "http://www.semanticdesktop.org/ontologies/2007/04/02/ncal#Timezone"
#define ZEITGEIST_NCAL_TODO "http://www.semanticdesktop.org/ontologies/2007/04/02/ncal#Todo"
#define ZEITGEIST_NCO_CONTACT "http://www.semanticdesktop.org/ontologies/2007/03/22/nco#Contact"
#define ZEITGEIST_NCO_CONTACT_GROUP "http://www.semanticdesktop.org/ontologies/2007/03/22/nco#ContactGroup"
#define ZEITGEIST_NCO_CONTACT_LIST "http://www.semanticdesktop.org/ontologies/2007/03/22/nco#ContactList"
#define ZEITGEIST_NCO_ORGANIZATION_CONTACT "http://www.semanticdesktop.org/ontologies/2007/03/22/nco#OrganizationContact"
#define ZEITGEIST_NCO_PERSON_CONTACT "http://www.semanticdesktop.org/ontologies/2007/03/22/nco#PersonContact"
#define ZEITGEIST_NFO_APPLICATION "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#Application"
#define ZEITGEIST_NFO_ARCHIVE "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#Archive"
#define ZEITGEIST_NFO_AUDIO "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#Audio"
#define ZEITGEIST_NFO_BOOKMARK "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#Bookmark"
#define ZEITGEIST_NFO_BOOKMARK_FOLDER "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#BookmarkFolder"
#define ZEITGEIST_NFO_CURSOR "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#Cursor"
#define ZEITGEIST_NFO_DATA_CONTAINER "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#DataContainer"
#define ZEITGEIST_NFO_DOCUMENT "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#Document"
#define ZEITGEIST_NFO_EXECUTABLE "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#Executable"
#define ZEITGEIST_NFO_FILESYSTEM "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#Filesystem"
#define ZEITGEIST_NFO_FILESYSTEM_IMAGE "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#FilesystemImage"
#define ZEITGEIST_NFO_FOLDER "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#Folder"
#define ZEITGEIST_NFO_FONT "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#Font"
#define ZEITGEIST_NFO_HTML_DOCUMENT "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#HtmlDocument"
#define ZEITGEIST_NFO_ICON "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#Icon"
#define ZEITGEIST_NFO_IMAGE "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#Image"
#define ZEITGEIST_NFO_MEDIA "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#Media"
#define ZEITGEIST_NFO_MEDIA_LIST "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#MediaList"
#define ZEITGEIST_NFO_MIND_MAP "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#MindMap"
#define ZEITGEIST_NFO_OPERATING_SYSTEM "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#OperatingSystem"
#define ZEITGEIST_NFO_PAGINATED_TEXT_DOCUMENT "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#PaginatedTextDocument"
#define ZEITGEIST_NFO_PLAIN_TEXT_DOCUMENT "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#PlainTextDocument"
#define ZEITGEIST_NFO_PRESENTATION "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#Presentation"
#define ZEITGEIST_NFO_RASTER_IMAGE "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#RasterImage"
#define ZEITGEIST_NFO_SOFTWARE "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#Software"
#define ZEITGEIST_NFO_SOURCE_CODE "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#SourceCode"
#define ZEITGEIST_NFO_SPREADSHEET "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#Spreadsheet"
#define ZEITGEIST_NFO_TEXT_DOCUMENT "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#TextDocument"
#define ZEITGEIST_NFO_TRASH "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#Trash"
#define ZEITGEIST_NFO_VECTOR_IMAGE "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#VectorImage"
#define ZEITGEIST_NFO_VIDEO "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#Video"
#define ZEITGEIST_NFO_VISUAL "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#Visual"
#define ZEITGEIST_NFO_WEBSITE "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#Website"
#define ZEITGEIST_NMM_MOVIE "http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#Movie"
#define ZEITGEIST_NMM_MUSIC_ALBUM "http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#MusicAlbum"
#define ZEITGEIST_NMM_MUSIC_PIECE "http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#MusicPiece"
#define ZEITGEIST_NMM_TVSERIES "http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#TVSeries"
#define ZEITGEIST_NMM_TVSHOW "http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#TVShow"
#define ZEITGEIST_NMO_EMAIL "http://www.semanticdesktop.org/ontologies/2007/03/22/nmo#Email"
#define ZEITGEIST_NMO_IMMESSAGE "http://www.semanticdesktop.org/ontologies/2007/03/22/nmo#IMMessage"
#define ZEITGEIST_NMO_MAILBOX "http://www.semanticdesktop.org/ontologies/2007/03/22/nmo#Mailbox"
#define ZEITGEIST_NMO_MESSAGE "http://www.semanticdesktop.org/ontologies/2007/03/22/nmo#Message"
#define ZEITGEIST_NMO_MIME_ENTITY "http://www.semanticdesktop.org/ontologies/2007/03/22/nmo#MimeEntity"
#define ZEITGEIST_ZG_ACCEPT_EVENT "http://www.zeitgeist-project.com/ontologies/2010/01/27/zg#AcceptEvent"
#define ZEITGEIST_ZG_ACCESS_EVENT "http://www.zeitgeist-project.com/ontologies/2010/01/27/zg#AccessEvent"
#define ZEITGEIST_ZG_CREATE_EVENT "http://www.zeitgeist-project.com/ontologies/2010/01/27/zg#CreateEvent"
#define ZEITGEIST_ZG_DELETE_EVENT "http://www.zeitgeist-project.com/ontologies/2010/01/27/zg#DeleteEvent"
#define ZEITGEIST_ZG_DENY_EVENT "http://www.zeitgeist-project.com/ontologies/2010/01/27/zg#DenyEvent"
#define ZEITGEIST_ZG_EVENT_INTERPRETATION "http://www.zeitgeist-project.com/ontologies/2010/01/27/zg#EventInterpretation"
#define ZEITGEIST_ZG_EXPIRE_EVENT "http://www.zeitgeist-project.com/ontologies/2010/01/27/zg#ExpireEvent"
#define ZEITGEIST_ZG_LEAVE_EVENT "http://www.zeitgeist-project.com/ontologies/2010/01/27/zg#LeaveEvent"
#define ZEITGEIST_ZG_MODIFY_EVENT "http://www.zeitgeist-project.com/ontologies/2010/01/27/zg#ModifyEvent"
#define ZEITGEIST_ZG_MOVE_EVENT "http://www.zeitgeist-project.com/ontologies/2010/01/27/zg#MoveEvent"
#define ZEITGEIST_ZG_RECEIVE_EVENT "http://www.zeitgeist-project.com/ontologies/2010/01/27/zg#ReceiveEvent"
#define ZEITGEIST_ZG_SEND_EVENT "http://www.zeitgeist-project.com/ontologies/2010/01/27/zg#SendEvent"

#define ZEITGEIST_NCAL_ATTACHMENT "http://www.semanticdesktop.org/ontologies/2007/04/02/ncal#Attachment"
#define ZEITGEIST_NCAL_CALENDAR_DATA_OBJECT "http://www.semanticdesktop.org/ontologies/2007/04/02/ncal#CalendarDataObject"
#define ZEITGEIST_NCO_CONTACT_LIST_DATA_OBJECT "http://www.semanticdesktop.org/ontologies/2007/03/22/nco#ContactListDataObject"
#define ZEITGEIST_NFO_ARCHIVE_ITEM "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#ArchiveItem"
#define ZEITGEIST_NFO_ATTACHMENT "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#Attachment"
#define ZEITGEIST_NFO_DELETED_RESOURCE "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#DeletedResource"
#define ZEITGEIST_NFO_EMBEDDED_FILE_DATA_OBJECT "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#EmbeddedFileDataObject"
#define ZEITGEIST_NFO_FILE_DATA_OBJECT "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#FileDataObject"
#define ZEITGEIST_NFO_HARD_DISK_PARTITION "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#HardDiskPartition"
#define ZEITGEIST_NFO_MEDIA_STREAM "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#MediaStream"
#define ZEITGEIST_NFO_REMOTE_DATA_OBJECT "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#RemoteDataObject"
#define ZEITGEIST_NFO_REMOTE_PORT_ADDRESS "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#RemotePortAddress"
#define ZEITGEIST_NFO_SOFTWARE_ITEM "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#SoftwareItem"
#define ZEITGEIST_NFO_SOFTWARE_SERVICE "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#SoftwareService"
#define ZEITGEIST_NFO_WEB_DATA_OBJECT "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#WebDataObject"
#define ZEITGEIST_NMO_MAILBOX_DATA_OBJECT "http://www.semanticdesktop.org/ontologies/2007/03/22/nmo#MailboxDataObject"
#define ZEITGEIST_ZG_EVENT_MANIFESTATION "http://www.zeitgeist-project.com/ontologies/2010/01/27/zg#EventManifestation"
#define ZEITGEIST_ZG_HEURISTIC_ACTIVITY "http://www.zeitgeist-project.com/ontologies/2010/01/27/zg#HeuristicActivity"
#define ZEITGEIST_ZG_SCHEDULED_ACTIVITY "http://www.zeitgeist-project.com/ontologies/2010/01/27/zg#ScheduledActivity"
#define ZEITGEIST_ZG_SYSTEM_NOTIFICATION "http://www.zeitgeist-project.com/ontologies/2010/01/27/zg#SystemNotification"
#define ZEITGEIST_ZG_USER_ACTIVITY "http://www.zeitgeist-project.com/ontologies/2010/01/27/zg#UserActivity"
#define ZEITGEIST_ZG_WORLD_ACTIVITY "http://www.zeitgeist-project.com/ontologies/2010/01/27/zg#WorldActivity"

/* END OF Copyright (C) 2010 Canonical, Ltd. */


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

typedef struct _LZGSubject {
  char              *uri;
  char              *origin;
  char              *text;
} LZGSubject;

typedef struct _LZGEvent {
  time_t             timestamp;
  char              *interpretation;
  LZGSubject **subjects;
} LZGEvent;

typedef struct _LZGLog {
  int                write_fd;
} LZGLog;

#define LZG_TARGET_DIR    ".local/share/zeitgeist"
#define LZG_TARGET_FILE   "syscalls.log"
#define LZG_LOG_FORBIDDEN "LOGGING-FORBIDDEN.lock"
#define LZG_TARGET_PATH    ".local/share/zeitgeist/syscalls.log"

char *_zg_get_actor_from_pid (pid_t pid);

LZGSubject *lzg_subject_new (void);
void lzg_subject_free (LZGSubject *s);
void lzg_subject_set_uri (LZGSubject *s, const char *uri);
void lzg_subject_set_origin (LZGSubject *s, const char *origin);
void lzg_subject_set_text (LZGSubject *s, const char *text);

LZGEvent *lzg_event_new (void);
void lzg_event_free (LZGEvent *e);
void lzg_event_set_interpretation (LZGEvent *e, const char *interpretation);
void lzg_event_add_subject (LZGEvent *e, LZGSubject *s);

LZGLog *lzg_log_get_default (int reset);
void lzg_log_insert_event (LZGLog *log, LZGEvent *event);


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
