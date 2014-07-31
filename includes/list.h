#ifndef    _LIST_H
#define    _LIST_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NULL
#define NULL                            ((void *)0)
#endif

#define OFFSET(Struct, Field)                           ((unsigned int)(unsigned char*)&(((Struct *)0)->Field))

#define list_entry(node, type, member) \
                                                                ((type *)((char *)(node) - (unsigned int)(&((type *)0)->member)))

#define list_entry_offset(node, type, offset) \
                                                                ((type *)((char *)(node) - (unsigned int)(offset)))

#define list_for_each(pos, head) \
    for (pos = (head)->m_next; pos != (head); \
        pos = pos->m_next)
#define list_for_each_safe(pos, n, head) \
    for (pos = (head)->m_next, n = pos->m_next; pos != (head); \
        pos = n, n = pos->m_next)

struct list_node{ 
    struct list_node				*m_next;	    /* point to next node. 	*/
	struct list_node				*m_prev;		/* point to prev node. 	*/
};

typedef struct list_node list;
typedef struct list_node list_t;
typedef struct list_node list_head_t;
typedef struct list_node list_node_t;

#define 	list_remove(n)			list_del(n)
#define    _INLINE_                inline

#if 1

//void list_init(list *l);
_INLINE_ void list_init(list *l)
{
	l->m_next                                                   = l; 
	l->m_prev                                                   = l;
}

_INLINE_ void list_insert_after(list *l, list *n)
{
	l->m_next->m_prev                                           = n;
	n->m_next                                                   = l->m_next;

	l->m_next                                                   = n;
	n->m_prev                                                   = l;
}

_INLINE_ void list_insert_before(list *l, list *n)
{
	l->m_prev->m_next                                           = n;
	n->m_prev                                                   = l->m_prev;

	l->m_prev                                                   = n;
	n->m_next                                                   = l;
}

_INLINE_ void list_del(list *n)
{
	n->m_next->m_prev                                           = n->m_prev;
	n->m_prev->m_next                                           = n->m_next;

	n->m_next                                                   = n;
    n->m_prev                                                   = n;
}

_INLINE_ int list_empty(const list *l)
{
	return (l->m_next == l);
}

_INLINE_ list *list_find(list_head_t *head, list_node_t *node)
{
	list_node_t 	*it;

	list_for_each(it, head){
		if (it == node){
			return it;
		}
	}

	return NULL;
}

_INLINE_ void __list_splice(list_head_t *list,
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
_INLINE_ void list_splice(list_head_t *list, list_head_t *head)
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
_INLINE_ void list_splice_init(list_head_t *list, list_head_t *head)
{
	if (!list_empty(list)) {
		__list_splice(list, head, head->m_next);
		list_init(list);
	}
}
#else

void list_init(list *l);
void list_insert_after(list *l, list *n);
void list_insert_before(list *l, list *n);
void list_del(list *n);
#define		list_remove(n)			list_del(n)
int list_empty(const list *l);
list *list_find(list_head_t *head, list_node_t *node);
void list_splice(list_head_t *list, list_head_t *head);
void list_splice_init(list_head_t *list, list_head_t *head);

#endif

#ifdef __cplusplus
}
#endif


/******************************************************************************
 *                             END  OF  FILE                                                                          
******************************************************************************/
#endif
