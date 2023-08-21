/*
 * Code for basic C skills diagnostic.
 * Developed for courses 15-213/18-213/15-513 by R. E. Bryant, 2017
 * Modified to store strings, 2018
 */

/*
 * This program implements a queue supporting both FIFO and LIFO
 * operations.
 *
 * It uses a singly-linked list to represent the set of queue elements
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "harness.h"
#include "queue.h"

/*
  Create empty queue.
  Return NULL if could not allocate space.
*/
queue_t *q_new()
{
  queue_t *q = malloc(sizeof(queue_t));

  if (q == NULL)
  {
    return NULL;
  }

  q->head = NULL;
  q->tail = NULL;
  q->size = 0;
  return q;
}

/* Free all storage used by queue */
void q_free(queue_t *q)
{
  if (q != NULL)
  {
    list_ele_t *current_element = q->head;
    while (current_element != NULL)
    {
      list_ele_t *temp_element = current_element->next;
      free(current_element->value);
      free(current_element);
      current_element = temp_element;
    }
  free(q);
  }
}

/*
  Attempt to insert element at head of queue.
  Return true if successful.
  Return false if q is NULL or could not allocate space.
  Argument s points to the string to be stored.
  The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_head(queue_t *q, char *s)
{
  if (q == NULL)
  {
    return false;
  }

  list_ele_t *newh;
  newh = malloc(sizeof(list_ele_t));
  if (!newh)
  {
    return false;
  }

  newh->value = malloc(strlen(s) + 1);
  if (newh->value == NULL)
  {
    free(newh);
    return false;
  }

  strcpy(newh->value, s); // copies string with null terminated character, dont have to use memcpy
  if (q->head == NULL)
  {
    q->tail = newh;
  }

  newh->next = q->head;
  q->head = newh;
  q->size++;

  return true;
}

/*
  Attempt to insert element at tail of queue.
  Return true if successful.
  Return false if q is NULL or could not allocate space.
  Argument s points to the string to be stored.
  The function must explicitly allocate space and copy the string into it.
  It should operate in O(1) time 
*/

bool q_insert_tail(queue_t *q, char *s)
{
  if (q == NULL)
  {
    return false;
  }

  list_ele_t *newh;
  newh = malloc(sizeof(list_ele_t));
  if (newh == NULL)
  {
    return false;
  }

  newh->value = malloc(strlen(s) + 1);
  if (newh->value == NULL)
  {
    free(newh);
    return false;
  }
  strcpy(newh->value, s);

  if (q->size == 0)
  {
    q->head = newh;
  }
  else
  {
    q->tail->next = newh;
  }

  newh->next = NULL;
  q->tail = newh;

  q->size++;

  return true;
}

/*
  Attempt to remove element from head of queue.
  Return true if successful.
  Return false if queue is NULL or empty.
  If sp is non-NULL and an element is removed, copy the removed string to *sp
  (up to a maximum of bufsize-1 characters, plus a null terminator.)
  The space used by the list element and the string should be freed.
*/

bool q_remove_head(queue_t *q, char *sp, size_t bufsize)
{
  if (q == NULL)
  {
    return false;
  }

  if (q->head == NULL)
  {
    return false;
  }

  if (sp)
  {
    strncpy(sp, q->head->value, bufsize - 1);
    sp[bufsize - 1] = '\0';
  }

  list_ele_t *tmp;
  tmp = q->head;
  q->head = q->head->next;

  free(tmp->value);
  free(tmp);
  q->size--;
  return true;
}

/*
  Return number of elements in queue.
  Return 0 if q is NULL or empty
 */
int q_size(queue_t *q)
{
  if (q != NULL && (q->head != NULL))
  {
    return q->size;;
  }
  
  return 0;
}

/*
  Reverse elements in queue
  No effect if q is NULL or empty
  This function should not allocate or free any list elements
  (e.g., by calling q_insert_head, q_insert_tail, or q_remove_head).
  It should rearrange the existing ones.
 */
void q_reverse(queue_t *q)
{
  if (q && (q->size > 1)) 
  {
    list_ele_t *current_node = q->head;
    list_ele_t *previous_node = NULL;
    list_ele_t *next_node = NULL;
    q->tail = current_node;

    while(current_node != NULL)
    {
      next_node = current_node->next;
      current_node->next = previous_node;
      previous_node = current_node;
      current_node = next_node;
    }

    q->head = previous_node;
  }
}
