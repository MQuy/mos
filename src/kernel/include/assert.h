#ifndef INCLUDE_ASSERT_H
#define INCLUDE_ASSERT_H

#define assert_not_reached() (asm volatile("ud2"))

#endif
