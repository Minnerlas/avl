#ifndef ALLOC_H
#define ALLOC_H

#include <stddef.h>

#include "avl.h"

void *avl_alloc(size_t sz);
void avl_free(void *ptr);

#endif /* ALLOC_H */
