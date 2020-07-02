#include "timer.h"

#include <kernel/cpu/idt.h>

extern volatile unsigned long jiffies;

struct list_head list_of_timer;

void assert_timer_valid(struct timer_list *timer)
{
	if (timer->magic != TIMER_MAGIC)
		__asm__ __volatile("int $0x0E");
}

void add_timer(struct timer_list *timer)
{
	struct timer_list *iter, *node;
	list_for_each_entry(iter, &list_of_timer, sibling)
	{
		assert_timer_valid(iter);
		if (timer->expires < iter->expires)
			break;
		node = iter;
	}

	if (node)
		list_add(timer, node);
	else
		list_add_tail(&list_of_timer, timer);
}

void del_timer(struct timer_list *timer)
{
	list_del(&timer->sibling);
}

void mod_timer(struct timer_list *timer, unsigned long expires)
{
	del_timer(timer);
	timer->expires = expires;
	add_timer(timer);
}

int32_t timer_schedule_handler(struct interrupt_registers *regs)
{
	struct timer_list *iter, *next;
	list_for_each_entry_safe(iter, next, &list_of_timer, sibling)
	{
		assert_timer_valid(iter);
		if (iter->expires <= jiffies)
			iter->function(iter);
	}

	return IRQ_HANDLER_CONTINUE;
}

void timer_init()
{
	INIT_LIST_HEAD(&list_of_timer);
	register_interrupt_handler(IRQ0, timer_schedule_handler);
}
