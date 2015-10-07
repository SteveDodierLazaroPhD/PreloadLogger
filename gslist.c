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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
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

#include <stddef.h>
#include <stdlib.h>
#include "gslist.h"
#include "logger.h"

PrelogSList*
prelog_slist_alloc (void)
{
  return __libc_malloc (sizeof (PrelogSList));
}

void
prelog_slist_free (PrelogSList *list)
{
  if (list) {
    prelog_slist_free (list->next);
    __libc_free (list);
  }
}

void
prelog_slist_free_1 (PrelogSList *list)
{
  __libc_free (list);
}


void
prelog_slist_free_full (PrelogSList *list,
                   PrelogDestroyNotify free_func)
{
  prelog_slist_foreach (list, (PrelogFunc) free_func, NULL);
  prelog_slist_free (list);
}

PrelogSList*
prelog_slist_append (PrelogSList *list,
                void * data)
{
  PrelogSList *new_list;
  PrelogSList *last;

  new_list = __libc_malloc (sizeof (PrelogSList));
  new_list->data = data;
  new_list->next = NULL;

  if (list)
    {
      last = prelog_slist_last (list);

      last->next = new_list;

      return list;
    }
  else
    return new_list;
}

PrelogSList*
prelog_slist_prepend (PrelogSList *list,
                 void * data)
{
  PrelogSList *new_list;

  new_list = __libc_malloc (sizeof (PrelogSList));
  new_list->data = data;
  new_list->next = list;

  return new_list;
}

PrelogSList*
prelog_slist_insert (PrelogSList *list,
                void * data,
                int position)
{
  PrelogSList *prev_list;
  PrelogSList *tmp_list;
  PrelogSList *new_list;

  if (position < 0)
    return prelog_slist_append (list, data);
  else if (position == 0)
    return prelog_slist_prepend (list, data);

  new_list = __libc_malloc (sizeof (PrelogSList));
  new_list->data = data;

  if (!list)
    {
      new_list->next = NULL;
      return new_list;
    }

  prev_list = NULL;
  tmp_list = list;

  while ((position-- > 0) && tmp_list)
    {
      prev_list = tmp_list;
      tmp_list = tmp_list->next;
    }

  new_list->next = prev_list->next;
  prev_list->next = new_list;

  return list;
}

PrelogSList*
prelog_slist_insert_before (PrelogSList *slist,
                       PrelogSList *sibling,
                       void * data)
{
  if (!slist)
    {
      slist = __libc_malloc (sizeof (PrelogSList));
      slist->data = data;
      slist->next = NULL;
      return slist;
    }
  else
    {
      PrelogSList *node, *last = NULL;

      for (node = slist; node; last = node, node = last->next)
        if (node == sibling)
          break;
      if (!last)
        {
          node = __libc_malloc (sizeof (PrelogSList));
          node->data = data;
          node->next = slist;

          return node;
        }
      else
        {
          node = __libc_malloc (sizeof (PrelogSList));
          node->data = data;
          node->next = last->next;
          last->next = node;

          return slist;
        }
    }
}

PrelogSList *
prelog_slist_concat (PrelogSList *list1, PrelogSList *list2)
{
  if (list2)
    {
      if (list1)
        prelog_slist_last (list1)->next = list2;
      else
        list1 = list2;
    }

  return list1;
}

PrelogSList*
prelog_slist_remove (PrelogSList *list,
                const void * data)
{
  PrelogSList *tmp, *prev = NULL;

  tmp = list;
  while (tmp)
    {
      if (tmp->data == data)
        {
          if (prev)
            prev->next = tmp->next;
          else
            list = tmp->next;

          prelog_slist_free_1 (tmp);
          break;
        }
      prev = tmp;
      tmp = prev->next;
    }

  return list;
}

PrelogSList*
prelog_slist_remove_all (PrelogSList *list,
                    const void * data)
{
  PrelogSList *tmp, *prev = NULL;

  tmp = list;
  while (tmp)
    {
      if (tmp->data == data)
        {
          PrelogSList *next = tmp->next;

          if (prev)
            prev->next = next;
          else
            list = next;

          prelog_slist_free_1 (tmp);
          tmp = next;
        }
      else
        {
          prev = tmp;
          tmp = prev->next;
        }
    }

  return list;
}

static inline PrelogSList*
_prelog_slist_remove_link (PrelogSList *list,
                      PrelogSList *link)
{
  PrelogSList *tmp;
  PrelogSList *prev;

  prev = NULL;
  tmp = list;

  while (tmp)
    {
      if (tmp == link)
        {
          if (prev)
            prev->next = tmp->next;
          if (list == tmp)
            list = list->next;

          tmp->next = NULL;
          break;
        }

      prev = tmp;
      tmp = tmp->next;
    }

  return list;
}

PrelogSList*
prelog_slist_remove_link (PrelogSList *list,
                     PrelogSList *link_)
{
  return _prelog_slist_remove_link (list, link_);
}

PrelogSList*
prelog_slist_delete_link (PrelogSList *list,
                     PrelogSList *link_)
{
  list = _prelog_slist_remove_link (list, link_);
  __libc_free (link_);

  return list;
}

PrelogSList*
prelog_slist_copy (PrelogSList *list)
{
  return prelog_slist_copy_deep (list, NULL, NULL);
}

PrelogSList*
prelog_slist_copy_deep (PrelogSList *list, PrelogCopyFunc func, void * user_data)
{
  PrelogSList *new_list = NULL;

  if (list)
    {
      PrelogSList *last;

      new_list = __libc_malloc (sizeof (PrelogSList));
      if (func)
        new_list->data = func (list->data, user_data);
      else
        new_list->data = list->data;
      last = new_list;
      list = list->next;
      while (list)
        {
          last->next = __libc_malloc (sizeof (PrelogSList));
          last = last->next;
          if (func)
            last->data = func (list->data, user_data);
          else
            last->data = list->data;
          list = list->next;
        }
      last->next = NULL;
    }

  return new_list;
}

PrelogSList*
prelog_slist_reverse (PrelogSList *list)
{
  PrelogSList *prev = NULL;

  while (list)
    {
      PrelogSList *next = list->next;

      list->next = prev;

      prev = list;
      list = next;
    }

  return prev;
}
PrelogSList*
prelog_slist_nth (PrelogSList *list,
             unsigned int n)
{
  while (n-- > 0 && list)
    list = list->next;

  return list;
}
void *
prelog_slist_nth_data (PrelogSList *list,
                  unsigned int n)
{
  while (n-- > 0 && list)
    list = list->next;

  return list ? list->data : NULL;
}
PrelogSList*
prelog_slist_find (PrelogSList *list,
              const void * data)
{
  while (list)
    {
      if (list->data == data)
        break;
      list = list->next;
    }

  return list;
}
PrelogSList*
prelog_slist_find_custom (PrelogSList *list,
                     const void * data,
                     PrelogCompareFunc func)
{
  if (!func)
    return NULL;

  while (list)
    {
      if (! func (list->data, data))
        return list;
      list = list->next;
    }

  return NULL;
}
int
prelog_slist_position (PrelogSList *list,
                  PrelogSList *llink)
{
  int i;

  i = 0;
  while (list)
    {
      if (list == llink)
        return i;
      i++;
      list = list->next;
    }

  return -1;
}
int
prelog_slist_index (PrelogSList *list,
               const void * data)
{
  int i;

  i = 0;
  while (list)
    {
      if (list->data == data)
        return i;
      i++;
      list = list->next;
    }

  return -1;
}
PrelogSList*
prelog_slist_last (PrelogSList *list)
{
  if (list)
    {
      while (list->next)
        list = list->next;
    }

  return list;
}
unsigned int
prelog_slist_length (PrelogSList *list)
{
  unsigned int length;

  length = 0;
  while (list)
    {
      length++;
      list = list->next;
    }

  return length;
}
void
prelog_slist_foreach (PrelogSList *list,
                 PrelogFunc func,
                 void * user_data)
{
  while (list)
    {
      PrelogSList *next = list->next;
      (*func) (list->data, user_data);
      list = next;
    }
}

static PrelogSList*
prelog_slist_insert_sorted_real (PrelogSList *list,
                            void * data,
                            PrelogFunc func,
                            void * user_data)
{
  PrelogSList *tmp_list = list;
  PrelogSList *prev_list = NULL;
  PrelogSList *new_list;
  int cmp;

  if (!func)
      return list;

  if (!list)
    {
      new_list = __libc_malloc (sizeof (PrelogSList));
      new_list->data = data;
      new_list->next = NULL;
      return new_list;
    }

  cmp = ((PrelogCompareDataFunc) func) (data, tmp_list->data, user_data);

  while ((tmp_list->next) && (cmp > 0))
    {
      prev_list = tmp_list;
      tmp_list = tmp_list->next;

      cmp = ((PrelogCompareDataFunc) func) (data, tmp_list->data, user_data);
    }

  new_list = __libc_malloc (sizeof (PrelogSList));
  new_list->data = data;

  if ((!tmp_list->next) && (cmp > 0))
    {
      tmp_list->next = new_list;
      new_list->next = NULL;
      return list;
    }

  if (prev_list)
    {
      prev_list->next = new_list;
      new_list->next = tmp_list;
      return list;
    }
  else
    {
      new_list->next = list;
      return new_list;
    }
}
PrelogSList*
prelog_slist_insert_sorted (PrelogSList *list,
                       void * data,
                       PrelogCompareFunc func)
{
  return prelog_slist_insert_sorted_real (list, data, (PrelogFunc) func, NULL);
}
PrelogSList*
prelog_slist_insert_sorted_with_data (PrelogSList *list,
                                 void * data,
                                 PrelogCompareDataFunc func,
                                 void * user_data)
{
  return prelog_slist_insert_sorted_real (list, data, (PrelogFunc) func, user_data);
}

static PrelogSList *
prelog_slist_sort_merge (PrelogSList *l1,
                    PrelogSList *l2,
                    PrelogFunc compare_func,
                    void * user_data)
{
  PrelogSList list, *l;
  int cmp;

  l=&list;

  while (l1 && l2)
    {
      cmp = ((PrelogCompareDataFunc) compare_func) (l1->data, l2->data, user_data);

      if (cmp <= 0)
        {
          l=l->next=l1;
          l1=l1->next;
        }
      else
        {
          l=l->next=l2;
          l2=l2->next;
        }
    }
  l->next= l1 ? l1 : l2;

  return list.next;
}

static PrelogSList *
prelog_slist_sort_real (PrelogSList *list,
                   PrelogFunc compare_func,
                   void * user_data)
{
  PrelogSList *l1, *l2;

  if (!list)
    return NULL;
  if (!list->next)
    return list;

  l1 = list;
  l2 = list->next;

  while ((l2 = l2->next) != NULL)
    {
      if ((l2 = l2->next) == NULL)
        break;
      l1=l1->next;
    }
  l2 = l1->next;
  l1->next = NULL;

  return prelog_slist_sort_merge (prelog_slist_sort_real (list, compare_func, user_data),
                             prelog_slist_sort_real (l2, compare_func, user_data),
                             compare_func,
                             user_data);
}
PrelogSList *
prelog_slist_sort (PrelogSList *list,
              PrelogCompareFunc compare_func)
{
  return prelog_slist_sort_real (list, (PrelogFunc) compare_func, NULL);
}
PrelogSList *
prelog_slist_sort_with_data (PrelogSList *list,
                        PrelogCompareDataFunc compare_func,
                        void * user_data)
{
  return prelog_slist_sort_real (list, (PrelogFunc) compare_func, user_data);
}
