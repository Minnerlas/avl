#ifndef ALLOC_H
#define ALLOC_H

#include <stddef.h>

void *avl_alloc(size_t sz);
void avl_free(void *ptr);
void *avl_realloc(void *ptr, size_t size);

#endif /* ALLOC_H */
