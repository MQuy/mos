#ifndef INCLUDE_LIST_H
#define INCLUDE_LIST_H

// MQ 2019-08-08
// Explain how list_head works https://kernelnewbies.org/FAQ/LinkedLists

typedef struct list_head
{
  struct list_head *next, *prev;
} list_head;

#define LIST_HEAD_INIT(name) \
  {                          \
    &(name), &(name)         \
  }

#define LIST_HEAD(name) \
  struct list_head name = LIST_HEAD_INIT(name)

#define INIT_LIST_HEAD(ptr) \
  do                        \
  {                         \
    (ptr)->next = (ptr);    \
    (ptr)->prev = (ptr);    \
  } while (0)

static inline void __list_add(struct list_head *new,
                              struct list_head *prev,
                              struct list_head *next)
{
  next->prev = new;
  new->next = next;
  new->prev = prev;
  prev->next = new;
}

static inline void list_add(struct list_head *new, struct list_head *head)
{
  __list_add(new, head, head->next);
}

static inline void list_add_tail(struct list_head *new, struct list_head *head)
{
  __list_add(new, head->prev, head);
}

static inline void __list_del(struct list_head *prev, struct list_head *next)
{
  next->prev = prev;
  prev->next = next;
}

static inline void list_del(struct list_head *entry)
{
  __list_del(entry->prev, entry->next);
  entry->next = NULL;
  entry->prev = NULL;
}

static inline int list_empty(const struct list_head *head)
{
  return head->next == head;
}

#define list_for_each(pos, head) for (pos = (head)->next; pos != (head); pos = pos->next)

#define list_for_each_entry(pos, head, member) \
  for (pos = list_entry((head)->next, typeof(*pos), member); &pos->member != (head); pos = list_entry(pos->member.next, typeof(*pos), member))

#define offsetof(TYPE, MEMBER) ((size_t) & ((TYPE *)0)->MEMBER)

#define container_of(ptr, type, member) ({			\
        const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
        (type *)( (char *)__mptr - offsetof(type,member) ); })

#define list_entry(ptr, type, member) container_of(ptr, type, member)

#endif