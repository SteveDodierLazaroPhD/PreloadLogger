/* GLIB - Library of useful routines for C programming
 * Copyright (C) 1995-1997  Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Modified by the GLib Team and others 1997-2000.  See the AUTHORS
 * file for a list of people on the GLib Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GLib at ftp://ftp.gtk.org/pub/gtk/.
 */

#ifndef __G_SLIST_H__
#define __G_SLIST_H__

typedef struct _LZGSList LZGSList;

struct _LZGSList
{
  void *data;
  LZGSList *next;
};

#define INT_TO_POINTER(i) ((void *) (long) (i))
#define POINTER_TO_INT(p) ((int) (long) (p))
#define POINTER_TO_UINT(p) ((unsigned int) (unsigned long) (p))

typedef int (*LZGCompareFunc) (const void *, const void *);
typedef int (*LZGCompareDataFunc) (const void *, const void *, void *);
typedef void (*LZGDestroyNotify) (void *);
typedef void * (*LZGCopyFunc) (const void *, void *);
typedef void (*LZGFunc) (void *, void *);
typedef int (*LZGEqualFunc) (const void *, const void *);
typedef unsigned int (*LZGHashFunc) (const void *);
typedef void (*LZGHFunc) (void *, void *, void *);


/* Singly linked lists
 */
LZGSList*  lzg_slist_alloc                   (void);
void     lzg_slist_free                    (LZGSList           *list);
void     lzg_slist_free_1                  (LZGSList           *list);
#define	 lzg_slist_free1		         lzg_slist_free_1
void     lzg_slist_free_full               (LZGSList           *list,
					  LZGDestroyNotify);
LZGSList*  lzg_slist_append                  (LZGSList           *list,
					  void *          data);
LZGSList*  lzg_slist_prepend                 (LZGSList           *list,
					  void *          data);
LZGSList*  lzg_slist_insert                  (LZGSList           *list,
					  void *          data,
					  int              position);
LZGSList*  lzg_slist_insert_sorted           (LZGSList           *list,
					  void *          data,
					  LZGCompareFunc);
LZGSList*  lzg_slist_insert_sorted_with_data (LZGSList           *list,
					  void *          data,
					  LZGCompareDataFunc,
					  void *          user_data);
LZGSList*  lzg_slist_insert_before           (LZGSList           *slist,
					  LZGSList           *sibling,
					  void *          data);
LZGSList*  lzg_slist_concat                  (LZGSList           *list1,
					  LZGSList           *list2);
LZGSList*  lzg_slist_remove                  (LZGSList           *list,
					  const void *     data);
LZGSList*  lzg_slist_remove_all              (LZGSList           *list,
					  const void *     data);
LZGSList*  lzg_slist_remove_link             (LZGSList           *list,
					  LZGSList           *link_);
LZGSList*  lzg_slist_delete_link             (LZGSList           *list,
					  LZGSList           *link_);
LZGSList*  lzg_slist_reverse                 (LZGSList           *list);
LZGSList*  lzg_slist_copy                    (LZGSList           *list);

LZGSList*  lzg_slist_copy_deep               (LZGSList            *list,
					  LZGCopyFunc,
					  void *          user_data);
LZGSList*  lzg_slist_nth                     (LZGSList           *list,
					  unsigned int             n);
LZGSList*  lzg_slist_find                    (LZGSList           *list,
					  const void *     data);
LZGSList*  lzg_slist_find_custom             (LZGSList           *list,
					  const void *     data,
					  LZGCompareFunc);
int     lzg_slist_position                (LZGSList           *list,
					  LZGSList           *llink);
int     lzg_slist_index                   (LZGSList           *list,
					  const void *     data);
LZGSList*  lzg_slist_last                    (LZGSList           *list);
unsigned int    lzg_slist_length                  (LZGSList           *list);
void     lzg_slist_foreach                 (LZGSList           *list,
					  LZGFunc,
					  void *          user_data);
LZGSList*  lzg_slist_sort                    (LZGSList           *list,
					  LZGCompareFunc);
LZGSList*  lzg_slist_sort_with_data          (LZGSList           *list,
					  LZGCompareDataFunc,
					  void *          user_data);
void * lzg_slist_nth_data                (LZGSList           *list,
					  unsigned int             n);

#define  lzg_slist_next(slist)	         ((slist) ? (((LZGSList *)(slist))->next) : NULL)

#endif /* __lzg_SLIST_H__ */
