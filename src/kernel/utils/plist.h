#ifndef UTILS_PLIST_H
#define UTILS_PLIST_H

#include <include/list.h>
#include <stddef.h>
#include <stdint.h>

struct plist_head
{
	struct list_head node_list;
};

struct plist_node
{
	int prio;
	struct list_head prio_list;
	struct list_head node_list;
};

/**
 * PLIST_HEAD_INIT - static struct plist_head initializer
 * @head:	struct plist_head variable name
 */
#define PLIST_HEAD_INIT(head)                         \
	{                                                 \
		.node_list = LIST_HEAD_INIT((head).node_list) \
	}

/**
 * PLIST_HEAD - declare and init plist_head
 * @head:	name for struct plist_head variable
 */
#define PLIST_HEAD(head) \
	struct plist_head head = PLIST_HEAD_INIT(head)

/**
 * PLIST_NODE_INIT - static struct plist_node initializer
 * @node:	struct plist_node variable name
 * @__prio:	initial node priority
 */
#define PLIST_NODE_INIT(node, __prio)                  \
	{                                                  \
		.prio = (__prio),                              \
		.prio_list = LIST_HEAD_INIT((node).prio_list), \
		.node_list = LIST_HEAD_INIT((node).node_list), \
	}

/**
 * plist_head_init - dynamic struct plist_head initializer
 * @head:	&struct plist_head pointer
 */
static inline void
plist_head_init(struct plist_head *head)
{
	INIT_LIST_HEAD(&head->node_list);
}

/**
 * plist_node_init - Dynamic struct plist_node initializer
 * @node:	&struct plist_node pointer
 * @prio:	initial node priority
 */
static inline void plist_node_init(struct plist_node *node, int prio)
{
	node->prio = prio;
	INIT_LIST_HEAD(&node->prio_list);
	INIT_LIST_HEAD(&node->node_list);
}

extern void plist_add(struct plist_node *node, struct plist_head *head);
extern void plist_del(struct plist_node *node, struct plist_head *head);

extern void plist_requeue(struct plist_node *node, struct plist_head *head);

/**
 * plist_for_each - iterate over the plist
 * @pos:	the type * to use as a loop counter
 * @head:	the head for your list
 */
#define plist_for_each(pos, head) \
	list_for_each_entry(pos, &(head)->node_list, node_list)

/**
 * plist_for_each_continue - continue iteration over the plist
 * @pos:	the type * to use as a loop cursor
 * @head:	the head for your list
 *
 * Continue to iterate over plist, continuing after the current position.
 */
#define plist_for_each_continue(pos, head) \
	list_for_each_entry_continue(pos, &(head)->node_list, node_list)

/**
 * plist_for_each_safe - iterate safely over a plist of given type
 * @pos:	the type * to use as a loop counter
 * @n:	another type * to use as temporary storage
 * @head:	the head for your list
 *
 * Iterate over a plist of given type, safe against removal of list entry.
 */
#define plist_for_each_safe(pos, n, head) \
	list_for_each_entry_safe(pos, n, &(head)->node_list, node_list)

/**
 * plist_for_each_entry	- iterate over list of given type
 * @pos:	the type * to use as a loop counter
 * @head:	the head for your list
 * @mem:	the name of the list_head within the struct
 */
#define plist_for_each_entry(pos, head, mem) \
	list_for_each_entry(pos, &(head)->node_list, mem.node_list)

/**
 * plist_for_each_entry_continue - continue iteration over list of given type
 * @pos:	the type * to use as a loop cursor
 * @head:	the head for your list
 * @m:		the name of the list_head within the struct
 *
 * Continue to iterate over list of given type, continuing after
 * the current position.
 */
#define plist_for_each_entry_continue(pos, head, m) \
	list_for_each_entry_continue(pos, &(head)->node_list, m.node_list)

/**
 * plist_for_each_entry_safe - iterate safely over list of given type
 * @pos:	the type * to use as a loop counter
 * @n:		another type * to use as temporary storage
 * @head:	the head for your list
 * @m:		the name of the list_head within the struct
 *
 * Iterate over list of given type, safe against removal of list entry.
 */
#define plist_for_each_entry_safe(pos, n, head, m) \
	list_for_each_entry_safe(pos, n, &(head)->node_list, m.node_list)

/**
 * plist_head_empty - return !0 if a plist_head is empty
 * @head:	&struct plist_head pointer
 */
static inline int plist_head_empty(const struct plist_head *head)
{
	return list_empty(&head->node_list);
}

/**
 * plist_node_empty - return !0 if plist_node is not on a list
 * @node:	&struct plist_node pointer
 */
static inline int plist_node_empty(const struct plist_node *node)
{
	return list_empty(&node->node_list);
}

/* All functions below assume the plist_head is not empty. */

/**
 * plist_first_entry - get the struct for the first entry
 * @head:	the &struct plist_head pointer
 * @type:	the type of the struct this is embedded in
 * @member:	the name of the list_head within the struct
 */

#define plist_first_entry(head, type, member) \
	container_of(plist_first(head), type, member)

/**
 * plist_last_entry - get the struct for the last entry
 * @head:	the &struct plist_head pointer
 * @type:	the type of the struct this is embedded in
 * @member:	the name of the list_head within the struct
 */
#define plist_last_entry(head, type, member) \
	container_of(plist_last(head), type, member)

/**
 * plist_next - get the next entry in list
 * @pos:	the type * to cursor
 */
#define plist_next(pos) \
	list_next_entry(pos, node_list)

/**
 * plist_prev - get the prev entry in list
 * @pos:	the type * to cursor
 */
#define plist_prev(pos) \
	list_prev_entry(pos, node_list)

/**
 * plist_first - return the first node (and thus, highest priority)
 * @head:	the &struct plist_head pointer
 *
 * Assumes the plist is _not_ empty.
 */
static inline struct plist_node *plist_first(const struct plist_head *head)
{
	return list_entry(head->node_list.next,
					  struct plist_node, node_list);
}

/**
 * plist_last - return the last node (and thus, lowest priority)
 * @head:	the &struct plist_head pointer
 *
 * Assumes the plist is _not_ empty.
 */
static inline struct plist_node *plist_last(const struct plist_head *head)
{
	return list_entry(head->node_list.prev,
					  struct plist_node, node_list);
}

#endif
