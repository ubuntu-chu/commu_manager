#ifndef    _LIST_H
#define    _LIST_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NULL
#define NULL                            ((void *)0)
#endif

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

void list_init(list *l);
void list_insert_after(list *l, list *n);
void list_insert_before(list *l, list *n);
void list_del(list *n);
#define		list_remove(n)			list_del(n)
int list_empty(const list *l);
list *list_find(list_head_t *head, list_node_t *node);
void list_splice(list_head_t *list, list_head_t *head);
void list_splice_init(list_head_t *list, list_head_t *head);

#ifdef __cplusplus
}
#endif


/******************************************************************************
 *                             END  OF  FILE                                                                          
******************************************************************************/
#endif
