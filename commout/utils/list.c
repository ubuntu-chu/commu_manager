#include "list.h"


void list_init(list *l)
{
	l->m_next                                                   = l; 
    l->m_prev                                                   = l;
}

void list_insert_after(list *l, list *n)
{
	l->m_next->m_prev                                           = n;
	n->m_next                                                   = l->m_next;

	l->m_next                                                   = n;
	n->m_prev                                                   = l;
}

void list_insert_before(list *l, list *n)
{
	l->m_prev->m_next                                           = n;
	n->m_prev                                                   = l->m_prev;

	l->m_prev                                                   = n;
	n->m_next                                                   = l;
}

void list_del(list *n)
{
	n->m_next->m_prev                                           = n->m_prev;
	n->m_prev->m_next                                           = n->m_next;

	n->m_next                                                   = n;
    n->m_prev                                                   = n;
}

int list_empty(const list *l)
{
	return (l->m_next == l);
}

list *list_find(list_head_t *head, list_node_t *node)
{
	list_node_t 	*it;

	list_for_each(it, head){
		if (it == node){
			return it;
		}
	}

	return NULL;
}

void __list_splice(list_head_t *list,
				 list_head_t *prev,
				 list_head_t *next)
{
	list_head_t *first = list->m_next;
	list_head_t *last = list->m_prev;

	first->m_prev = prev;
	prev->m_next = first;

	last->m_next = next;
	next->m_prev = last;
}

/**
 * list_splice - join two lists, this is designed for stacks
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 */
void list_splice(list_head_t *list, list_head_t *head)
{
	if (!list_empty(list))
		__list_splice(list, head, head->m_next);
}

/**
 * list_splice_init - join two lists and reinitialise the emptied list.
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 *
 * The list at @list is reinitialised
 */
void list_splice_init(list_head_t *list, list_head_t *head)
{
	if (!list_empty(list)) {
		__list_splice(list, head, head->m_next);
		list_init(list);
	}
}


