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

/*
 * MT safe
 */

#include <stddef.h>
#include <stdlib.h>
#include "gslist.h"

/**
 * SECTION:linked_lists_single
 * @title: Singly-Linked Lists
 * @short_description: linked lists that can be iterated in one direction
 *
 * The #LZGSList structure and its associated functions provide a
 * standard singly-linked list data structure.
 *
 * Each element in the list contains a piece of data, together with a
 * pointer which links to the next element in the list. Using this
 * pointer it is possible to move through the list in one direction
 * only (unlike the [double-linked lists][glib-Doubly-Linked-Lists],
 * which allow movement in both directions).
 *
 * The data contained in each element can be either integer values, by
 * using one of the [Type Conversion Macros][glib-Type-Conversion-Macros],
 * or simply pointers to any type of data.
 *
 * List elements are allocated from the [slice allocator][glib-Memory-Slices],
 * which is more efficient than allocating elements individually.
 *
 * Note that most of the #LZGSList functions expect to be passed a
 * pointer to the first element in the list. The functions which insert
 * elements return the new start of the list, which may have changed.
 *
 * There is no function to create a #LZGSList. %NULL is considered to be
 * the empty list so you simply set a #LZGSList* to %NULL.
 *
 * To add elements, use lzg_slist_append(), lzg_slist_prepend(),
 * lzg_slist_insert() and lzg_slist_insert_sorted().
 *
 * To remove elements, use lzg_slist_remove().
 *
 * To find elements in the list use lzg_slist_last(), lzg_slist_next(),
 * lzg_slist_nth(), lzg_slist_nth_data(), lzg_slist_find() and
 * lzg_slist_find_custom().
 *
 * To find the index of an element use lzg_slist_position() and
 * lzg_slist_index().
 *
 * To call a function for each element in the list use
 * lzg_slist_foreach().
 *
 * To free the entire list, use lzg_slist_free().
 **/

/**
 * LZGSList:
 * @data: holds the element's data, which can be a pointer to any kind
 *        of data, or any integer value using the
 *        [Type Conversion Macros][glib-Type-Conversion-Macros]
 * @next: contains the link to the next element in the list.
 *
 * The #LZGSList struct is used for each element in the singly-linked
 * list.
 **/

/**
 * lzg_slist_next:
 * @slist: an element in a #LZGSList.
 *
 * A convenience macro to get the next element in a #LZGSList.
 *
 * Returns: the next element, or %NULL if there are no more elements.
 **/


/**
 * lzg_slist_alloc:
 *
 * Allocates space for one #LZGSList element. It is called by the
 * lzg_slist_append(), lzg_slist_prepend(), lzg_slist_insert() and
 * lzg_slist_insert_sorted() functions and so is rarely used on its own.
 *
 * Returns: a pointer to the newly-allocated #LZGSList element.
 **/
LZGSList*
lzg_slist_alloc (void)
{
  return malloc (sizeof (LZGSList));
}

/**
 * lzg_slist_free:
 * @list: a #LZGSList
 *
 * Frees all of the memory used by a #LZGSList.
 * The freed elements are returned to the slice allocator.
 *
 * If list elements contain dynamically-allocated memory,
 * you should either use lzg_slist_free_full() or free them manually
 * first.
 */
void
lzg_slist_free (LZGSList *list)
{
  if (list) {
    lzg_slist_free (list->next); 
    free (list);
  }
}

/**
 * lzg_slist_free_1:
 * @list: a #LZGSList element
 *
 * Frees one #LZGSList element.
 * It is usually used after lzg_slist_remove_link().
 */
/**
 * lzg_slist_free1:
 *
 * A macro which does the same as lzg_slist_free_1().
 *
 * Since: 2.10
 **/
void
lzg_slist_free_1 (LZGSList *list)
{
  free (list);
}

/**
 * lzg_slist_free_full:
 * @list: a pointer to a #LZGSList
 * @free_func: the function to be called to free each element's data
 *
 * Convenience method, which frees all the memory used by a #LZGSList, and
 * calls the specified destroy function on every element's data.
 *
 * Since: 2.28
 **/
void
lzg_slist_free_full (LZGSList         *list,
		   LZGDestroyNotify  free_func)
{
  lzg_slist_foreach (list, (LZGFunc) free_func, NULL);
  lzg_slist_free (list);
}

/**
 * lzg_slist_append:
 * @list: a #LZGSList
 * @data: the data for the new element
 *
 * Adds a new element on to the end of the list.
 *
 * The return value is the new start of the list, which may
 * have changed, so make sure you store the new value.
 *
 * Note that lzg_slist_append() has to traverse the entire list
 * to find the end, which is inefficient when adding multiple
 * elements. A common idiom to avoid the inefficiency is to prepend
 * the elements and reverse the list when all elements have been added.
 *
 * |[<!-- language="C" --> 
 * // Notice that these are initialized to the empty list.
 * LZGSList *list = NULL, *number_list = NULL;
 *
 * // This is a list of strings.
 * list = lzg_slist_append (list, "first");
 * list = lzg_slist_append (list, "second");
 *
 * // This is a list of integers.
 * ]|
 *
 * Returns: the new start of the #LZGSList
 */
LZGSList*
lzg_slist_append (LZGSList   *list,
                void *  data)
{
  LZGSList *new_list;
  LZGSList *last;

  new_list = malloc (sizeof (LZGSList));
  new_list->data = data;
  new_list->next = NULL;

  if (list)
    {
      last = lzg_slist_last (list);
      /* lzg_assert (last != NULL); */
      last->next = new_list;

      return list;
    }
  else
    return new_list;
}

/**
 * lzg_slist_prepend:
 * @list: a #LZGSList
 * @data: the data for the new element
 *
 * Adds a new element on to the start of the list.
 *
 * The return value is the new start of the list, which
 * may have changed, so make sure you store the new value.
 *
 * |[<!-- language="C" --> 
 * // Notice that it is initialized to the empty list.
 * LZGSList *list = NULL;
 * list = lzg_slist_prepend (list, "last");
 * list = lzg_slist_prepend (list, "first");
 * ]|
 *
 * Returns: the new start of the #LZGSList
 */
LZGSList*
lzg_slist_prepend (LZGSList   *list,
                 void *  data)
{
  LZGSList *new_list;

  new_list = malloc (sizeof (LZGSList));
  new_list->data = data;
  new_list->next = list;

  return new_list;
}

/**
 * lzg_slist_insert:
 * @list: a #LZGSList
 * @data: the data for the new element
 * @position: the position to insert the element.
 *     If this is negative, or is larger than the number
 *     of elements in the list, the new element is added on
 *     to the end of the list.
 *
 * Inserts a new element into the list at the given position.
 *
 * Returns: the new start of the #LZGSList
 */
LZGSList*
lzg_slist_insert (LZGSList   *list,
                void *  data,
                int      position)
{
  LZGSList *prev_list;
  LZGSList *tmp_list;
  LZGSList *new_list;

  if (position < 0)
    return lzg_slist_append (list, data);
  else if (position == 0)
    return lzg_slist_prepend (list, data);

  new_list = malloc (sizeof (LZGSList));
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

/**
 * lzg_slist_insert_before:
 * @slist: a #LZGSList
 * @sibling: node to insert @data before
 * @data: data to put in the newly-inserted node
 *
 * Inserts a node before @sibling containing @data.
 *
 * Returns: the new head of the list.
 */
LZGSList*
lzg_slist_insert_before (LZGSList  *slist,
                       LZGSList  *sibling,
                       void * data)
{
  if (!slist)
    {
      slist = malloc (sizeof (LZGSList));
      slist->data = data;
      slist->next = NULL;
      return slist;
    }
  else
    {
      LZGSList *node, *last = NULL;

      for (node = slist; node; last = node, node = last->next)
        if (node == sibling)
          break;
      if (!last)
        {
          node = malloc (sizeof (LZGSList));
          node->data = data;
          node->next = slist;

          return node;
        }
      else
        {
          node = malloc (sizeof (LZGSList));
          node->data = data;
          node->next = last->next;
          last->next = node;

          return slist;
        }
    }
}

/**
 * lzg_slist_concat:
 * @list1: a #LZGSList
 * @list2: the #LZGSList to add to the end of the first #LZGSList
 *
 * Adds the second #LZGSList onto the end of the first #LZGSList.
 * Note that the elements of the second #LZGSList are not copied.
 * They are used directly.
 *
 * Returns: the start of the new #LZGSList
 */
LZGSList *
lzg_slist_concat (LZGSList *list1, LZGSList *list2)
{
  if (list2)
    {
      if (list1)
        lzg_slist_last (list1)->next = list2;
      else
        list1 = list2;
    }

  return list1;
}

/**
 * lzg_slist_remove:
 * @list: a #LZGSList
 * @data: the data of the element to remove
 *
 * Removes an element from a #LZGSList.
 * If two elements contain the same data, only the first is removed.
 * If none of the elements contain the data, the #LZGSList is unchanged.
 *
 * Returns: the new start of the #LZGSList
 */
LZGSList*
lzg_slist_remove (LZGSList        *list,
                const void *  data)
{
  LZGSList *tmp, *prev = NULL;

  tmp = list;
  while (tmp)
    {
      if (tmp->data == data)
        {
          if (prev)
            prev->next = tmp->next;
          else
            list = tmp->next;

          lzg_slist_free_1 (tmp);
          break;
        }
      prev = tmp;
      tmp = prev->next;
    }

  return list;
}

/**
 * lzg_slist_remove_all:
 * @list: a #LZGSList
 * @data: data to remove
 *
 * Removes all list nodes with data equal to @data.
 * Returns the new head of the list. Contrast with
 * lzg_slist_remove() which removes only the first node
 * matching the given data.
 *
 * Returns: new head of @list
 */
LZGSList*
lzg_slist_remove_all (LZGSList        *list,
                    const void *  data)
{
  LZGSList *tmp, *prev = NULL;

  tmp = list;
  while (tmp)
    {
      if (tmp->data == data)
        {
          LZGSList *next = tmp->next;

          if (prev)
            prev->next = next;
          else
            list = next;

          lzg_slist_free_1 (tmp);
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

static inline LZGSList*
_lzg_slist_remove_link (LZGSList *list,
                      LZGSList *link)
{
  LZGSList *tmp;
  LZGSList *prev;

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

/**
 * lzg_slist_remove_link:
 * @list: a #LZGSList
 * @link_: an element in the #LZGSList
 *
 * Removes an element from a #LZGSList, without
 * freeing the element. The removed element's next
 * link is set to %NULL, so that it becomes a
 * self-contained list with one element.
 *
 * Removing arbitrary nodes from a singly-linked list
 * requires time that is proportional to the length of the list
 * (ie. O(n)). If you find yourself using lzg_slist_remove_link()
 * frequently, you should consider a different data structure,
 * such as the doubly-linked #GList.
 *
 * Returns: the new start of the #LZGSList, without the element
 */
LZGSList*
lzg_slist_remove_link (LZGSList *list,
                     LZGSList *link_)
{
  return _lzg_slist_remove_link (list, link_);
}

/**
 * lzg_slist_delete_link:
 * @list: a #LZGSList
 * @link_: node to delete
 *
 * Removes the node link_ from the list and frees it.
 * Compare this to lzg_slist_remove_link() which removes the node
 * without freeing it.
 *
 * Removing arbitrary nodes from a singly-linked list requires time
 * that is proportional to the length of the list (ie. O(n)). If you
 * find yourself using lzg_slist_delete_link() frequently, you should
 * consider a different data structure, such as the doubly-linked
 * #GList.
 *
 * Returns: the new head of @list
 */
LZGSList*
lzg_slist_delete_link (LZGSList *list,
                     LZGSList *link_)
{
  list = _lzg_slist_remove_link (list, link_);
  free (link_);

  return list;
}

/**
 * lzg_slist_copy:
 * @list: a #LZGSList
 *
 * Copies a #LZGSList.
 *
 * Note that this is a "shallow" copy. If the list elements
 * consist of pointers to data, the pointers are copied but
 * the actual data isn't. See lzg_slist_copy_deep() if you need
 * to copy the data as well.
 *
 * Returns: a copy of @list
 */
LZGSList*
lzg_slist_copy (LZGSList *list)
{
  return lzg_slist_copy_deep (list, NULL, NULL);
}

/**
 * lzg_slist_copy_deep:
 * @list: a #LZGSList
 * @func: a copy function used to copy every element in the list
 * @user_data: user data passed to the copy function @func, or #NULL
 *
 * Makes a full (deep) copy of a #LZGSList.
 *
 * In contrast with lzg_slist_copy(), this function uses @func to make a copy of
 * each list element, in addition to copying the list container itself.
 *
 * @func, as a #LZGCopyFunc, takes two arguments, the data to be copied and a user
 * pointer. It's safe to pass #NULL as user_data, if the copy function takes only
 * one argument.
 *
 * For instance, if @list holds a list of GObjects, you can do:
 * |[<!-- language="C" --> 
 * another_list = lzg_slist_copy_deep (list, (LZGCopyFunc) lzg_object_ref, NULL);
 * ]|
 *
 * And, to entirely free the new list, you could do:
 * |[<!-- language="C" --> 
 * lzg_slist_free_full (another_list, lzg_object_unref);
 * ]|
 *
 * Returns: a full copy of @list, use #lzg_slist_free_full to free it
 *
 * Since: 2.34
 */
LZGSList*
lzg_slist_copy_deep (LZGSList *list, LZGCopyFunc func, void * user_data)
{
  LZGSList *new_list = NULL;

  if (list)
    {
      LZGSList *last;

      new_list = malloc (sizeof (LZGSList));
      if (func)
        new_list->data = func (list->data, user_data);
      else
        new_list->data = list->data;
      last = new_list;
      list = list->next;
      while (list)
        {
          last->next = malloc (sizeof (LZGSList));
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

/**
 * lzg_slist_reverse:
 * @list: a #LZGSList
 *
 * Reverses a #LZGSList.
 *
 * Returns: the start of the reversed #LZGSList
 */
LZGSList*
lzg_slist_reverse (LZGSList *list)
{
  LZGSList *prev = NULL;

  while (list)
    {
      LZGSList *next = list->next;

      list->next = prev;

      prev = list;
      list = next;
    }

  return prev;
}

/**
 * lzg_slist_nth:
 * @list: a #LZGSList
 * @n: the position of the element, counting from 0
 *
 * Gets the element at the given position in a #LZGSList.
 *
 * Returns: the element, or %NULL if the position is off
 *     the end of the #LZGSList
 */
LZGSList*
lzg_slist_nth (LZGSList *list,
             unsigned int   n)
{
  while (n-- > 0 && list)
    list = list->next;

  return list;
}

/**
 * lzg_slist_nth_data:
 * @list: a #LZGSList
 * @n: the position of the element
 *
 * Gets the data of the element at the given position.
 *
 * Returns: the element's data, or %NULL if the position
 *     is off the end of the #LZGSList
 */
void *
lzg_slist_nth_data (LZGSList   *list,
                  unsigned int     n)
{
  while (n-- > 0 && list)
    list = list->next;

  return list ? list->data : NULL;
}

/**
 * lzg_slist_find:
 * @list: a #LZGSList
 * @data: the element data to find
 *
 * Finds the element in a #LZGSList which
 * contains the given data.
 *
 * Returns: the found #LZGSList element,
 *     or %NULL if it is not found
 */
LZGSList*
lzg_slist_find (LZGSList        *list,
              const void *  data)
{
  while (list)
    {
      if (list->data == data)
        break;
      list = list->next;
    }

  return list;
}


/**
 * lzg_slist_find_custom:
 * @list: a #LZGSList
 * @data: user data passed to the function
 * @func: the function to call for each element.
 *     It should return 0 when the desired element is found
 *
 * Finds an element in a #LZGSList, using a supplied function to
 * find the desired element. It iterates over the list, calling
 * the given function which should return 0 when the desired
 * element is found. The function takes two #const void * arguments,
 * the #LZGSList element's data as the first argument and the
 * given user data.
 *
 * Returns: the found #LZGSList element, or %NULL if it is not found
 */
LZGSList*
lzg_slist_find_custom (LZGSList        *list,
                     const void *  data,
                     LZGCompareFunc   func)
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

/**
 * lzg_slist_position:
 * @list: a #LZGSList
 * @llink: an element in the #LZGSList
 *
 * Gets the position of the given element
 * in the #LZGSList (starting from 0).
 *
 * Returns: the position of the element in the #LZGSList,
 *     or -1 if the element is not found
 */
int
lzg_slist_position (LZGSList *list,
                  LZGSList *llink)
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

/**
 * lzg_slist_index:
 * @list: a #LZGSList
 * @data: the data to find
 *
 * Gets the position of the element containing
 * the given data (starting from 0).
 *
 * Returns: the index of the element containing the data,
 *     or -1 if the data is not found
 */
int
lzg_slist_index (LZGSList        *list,
               const void *  data)
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

/**
 * lzg_slist_last:
 * @list: a #LZGSList
 *
 * Gets the last element in a #LZGSList.
 *
 * This function iterates over the whole list.
 *
 * Returns: the last element in the #LZGSList,
 *     or %NULL if the #LZGSList has no elements
 */
LZGSList*
lzg_slist_last (LZGSList *list)
{
  if (list)
    {
      while (list->next)
        list = list->next;
    }

  return list;
}

/**
 * lzg_slist_length:
 * @list: a #LZGSList
 *
 * Gets the number of elements in a #LZGSList.
 *
 * This function iterates over the whole list to
 * count its elements. To check whether the list is non-empty, it is faster to
 * check @list against %NULL.
 *
 * Returns: the number of elements in the #LZGSList
 */
unsigned int
lzg_slist_length (LZGSList *list)
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

/**
 * lzg_slist_foreach:
 * @list: a #LZGSList
 * @func: the function to call with each element's data
 * @user_data: user data to pass to the function
 *
 * Calls a function for each element of a #LZGSList.
 */
void
lzg_slist_foreach (LZGSList   *list,
                 LZGFunc     func,
                 void *  user_data)
{
  while (list)
    {
      LZGSList *next = list->next;
      (*func) (list->data, user_data);
      list = next;
    }
}

static LZGSList*
lzg_slist_insert_sorted_real (LZGSList   *list,
                            void *  data,
                            LZGFunc     func,
                            void *  user_data)
{
  LZGSList *tmp_list = list;
  LZGSList *prev_list = NULL;
  LZGSList *new_list;
  int cmp;

  if (!func)
      return list;

  if (!list)
    {
      new_list = malloc (sizeof (LZGSList));
      new_list->data = data;
      new_list->next = NULL;
      return new_list;
    }

  cmp = ((LZGCompareDataFunc) func) (data, tmp_list->data, user_data);

  while ((tmp_list->next) && (cmp > 0))
    {
      prev_list = tmp_list;
      tmp_list = tmp_list->next;

      cmp = ((LZGCompareDataFunc) func) (data, tmp_list->data, user_data);
    }

  new_list = malloc (sizeof (LZGSList));
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

/**
 * lzg_slist_insert_sorted:
 * @list: a #LZGSList
 * @data: the data for the new element
 * @func: the function to compare elements in the list.
 *     It should return a number > 0 if the first parameter
 *     comes after the second parameter in the sort order.
 *
 * Inserts a new element into the list, using the given
 * comparison function to determine its position.
 *
 * Returns: the new start of the #LZGSList
 */
LZGSList*
lzg_slist_insert_sorted (LZGSList       *list,
                       void *      data,
                       LZGCompareFunc  func)
{
  return lzg_slist_insert_sorted_real (list, data, (LZGFunc) func, NULL);
}

/**
 * lzg_slist_insert_sorted_with_data:
 * @list: a #LZGSList
 * @data: the data for the new element
 * @func: the function to compare elements in the list.
 *     It should return a number > 0 if the first parameter
 *     comes after the second parameter in the sort order.
 * @user_data: data to pass to comparison function
 *
 * Inserts a new element into the list, using the given
 * comparison function to determine its position.
 *
 * Returns: the new start of the #LZGSList
 *
 * Since: 2.10
 */
LZGSList*
lzg_slist_insert_sorted_with_data (LZGSList           *list,
                                 void *          data,
                                 LZGCompareDataFunc  func,
                                 void *          user_data)
{
  return lzg_slist_insert_sorted_real (list, data, (LZGFunc) func, user_data);
}

static LZGSList *
lzg_slist_sort_merge (LZGSList   *l1,
                    LZGSList   *l2,
                    LZGFunc     compare_func,
                    void *  user_data)
{
  LZGSList list, *l;
  int cmp;

  l=&list;

  while (l1 && l2)
    {
      cmp = ((LZGCompareDataFunc) compare_func) (l1->data, l2->data, user_data);

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

static LZGSList *
lzg_slist_sort_real (LZGSList   *list,
                   LZGFunc     compare_func,
                   void *  user_data)
{
  LZGSList *l1, *l2;

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

  return lzg_slist_sort_merge (lzg_slist_sort_real (list, compare_func, user_data),
                             lzg_slist_sort_real (l2, compare_func, user_data),
                             compare_func,
                             user_data);
}

/**
 * lzg_slist_sort:
 * @list: a #LZGSList
 * @compare_func: the comparison function used to sort the #LZGSList.
 *     This function is passed the data from 2 elements of the #LZGSList
 *     and should return 0 if they are equal, a negative value if the
 *     first element comes before the second, or a positive value if
 *     the first element comes after the second.
 *
 * Sorts a #LZGSList using the given comparison function.
 *
 * Returns: the start of the sorted #LZGSList
 */
LZGSList *
lzg_slist_sort (LZGSList       *list,
              LZGCompareFunc  compare_func)
{
  return lzg_slist_sort_real (list, (LZGFunc) compare_func, NULL);
}

/**
 * lzg_slist_sort_with_data:
 * @list: a #LZGSList
 * @compare_func: comparison function
 * @user_data: data to pass to comparison function
 *
 * Like lzg_slist_sort(), but the sort function accepts a user data argument.
 *
 * Returns: new head of the list
 */
LZGSList *
lzg_slist_sort_with_data (LZGSList           *list,
                        LZGCompareDataFunc  compare_func,
                        void *          user_data)
{
  return lzg_slist_sort_real (list, (LZGFunc) compare_func, user_data);
}
