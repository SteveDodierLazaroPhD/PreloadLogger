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

#ifndef __prelog_G_SLIST_H__
#define __prelog_G_SLIST_H__

typedef struct _PrelogSList PrelogSList;

struct _PrelogSList
{
  void *data;
  PrelogSList *next;
};

#define PRELOG_INT_TO_POINTER(i) ((void *) (long) (i))
#define PRELOG_POINTER_TO_INT(p) ((int) (long) (p))
#define PRELOG_POINTER_TO_UINT(p) ((unsigned int) (unsigned long) (p))

typedef int (*PrelogCompareFunc) (const void *, const void *);
typedef int (*PrelogCompareDataFunc) (const void *, const void *, void *);
typedef void (*PrelogDestroyNotify) (void *);
typedef void * (*PrelogCopyFunc) (const void *, void *);
typedef void (*PrelogFunc) (void *, void *);
typedef int (*PrelogEqualFunc) (const void *, const void *);
typedef unsigned int (*PrelogHashFunc) (const void *);
typedef void (*PrelogHFunc) (void *, void *, void *);


/* Singly linked lists
 */
PrelogSList*  prelog_slist_alloc                   (void);
void     prelog_slist_free                    (PrelogSList           *list);
void     prelog_slist_free_1                  (PrelogSList           *list);
#define	 prelog_slist_free1		         prelog_slist_free_1
void     prelog_slist_free_full               (PrelogSList           *list,
                                          PrelogDestroyNotify);
PrelogSList*  prelog_slist_append                  (PrelogSList           *list,
					  void *          data);
PrelogSList*  prelog_slist_prepend                 (PrelogSList           *list,
					  void *          data);
PrelogSList*  prelog_slist_insert                  (PrelogSList           *list,
					  void *          data,
					  int              position);
PrelogSList*  prelog_slist_insert_sorted           (PrelogSList           *list,
					  void *          data,
                                          PrelogCompareFunc);
PrelogSList*  prelog_slist_insert_sorted_with_data (PrelogSList           *list,
					  void *          data,
                                          PrelogCompareDataFunc,
					  void *          user_data);
PrelogSList*  prelog_slist_insert_before           (PrelogSList           *slist,
                                          PrelogSList           *sibling,
					  void *          data);
PrelogSList*  prelog_slist_concat                  (PrelogSList           *list1,
                                          PrelogSList           *list2);
PrelogSList*  prelog_slist_remove                  (PrelogSList           *list,
					  const void *     data);
PrelogSList*  prelog_slist_remove_all              (PrelogSList           *list,
					  const void *     data);
PrelogSList*  prelog_slist_remove_link             (PrelogSList           *list,
                                          PrelogSList           *link_);
PrelogSList*  prelog_slist_delete_link             (PrelogSList           *list,
                                          PrelogSList           *link_);
PrelogSList*  prelog_slist_reverse                 (PrelogSList           *list);
PrelogSList*  prelog_slist_copy                    (PrelogSList           *list);

PrelogSList*  prelog_slist_copy_deep               (PrelogSList            *list,
                                          PrelogCopyFunc,
					  void *          user_data);
PrelogSList*  prelog_slist_nth                     (PrelogSList           *list,
					  unsigned int             n);
PrelogSList*  prelog_slist_find                    (PrelogSList           *list,
					  const void *     data);
PrelogSList*  prelog_slist_find_custom             (PrelogSList           *list,
					  const void *     data,
                                          PrelogCompareFunc);
int     prelog_slist_position                (PrelogSList           *list,
                                          PrelogSList           *llink);
int     prelog_slist_index                   (PrelogSList           *list,
					  const void *     data);
PrelogSList*  prelog_slist_last                    (PrelogSList           *list);
unsigned int    prelog_slist_length                  (PrelogSList           *list);
void     prelog_slist_foreach                 (PrelogSList           *list,
                                          PrelogFunc,
					  void *          user_data);
PrelogSList*  prelog_slist_sort                    (PrelogSList           *list,
                                          PrelogCompareFunc);
PrelogSList*  prelog_slist_sort_with_data          (PrelogSList           *list,
                                          PrelogCompareDataFunc,
					  void *          user_data);
void * prelog_slist_nth_data                (PrelogSList           *list,
					  unsigned int             n);

#define  prelog_slist_next(slist)	         ((slist) ? (((prelogSList *)(slist))->next) : NULL)

#endif /* __prelog_SLIST_H__ */
