#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "avl.h"
#include "avl_alloc.h"

#define PAGESZ 4096

#define MIN_BUCKET_SHIFT 5
#define MIN_BUCKET (1 << MIN_BUCKET_SHIFT)
#define MAX_BUCKET_SHIFT 11
#define MAX_BUCKET (1 << MAX_BUCKET_SHIFT)
#define NUM_BUCKETS (MAX_BUCKET_SHIFT - MIN_BUCKET_SHIFT + 1)
/*
 * alloc sizes: 32, 64, 128, 256, 512, 1024, 2048
 */
struct avl_node *roots[NUM_BUCKETS] = {0};

#define ALLOC_HDR(hdr) ((struct alloc_header *)((char *)(hdr) \
			- sizeof(struct alloc_header)))

struct alloc_header {
	uint32_t sz;    /* bucket size of the allocation */
	uint32_t reqsz; /* TODO: requested alloc size */
	uint64_t _pad1; /* TODO: cpu id maybe? */
};

/*
 * TODO: Not a macro since C23
 */
#ifdef static_assert
static_assert(MAX_BUCKET == PAGESZ / 2, "MAX_BUCKET != PAGESZ / 2");
static_assert(MIN_BUCKET == sizeof(struct avl_node),
		"MIN_BUCKET != sizeof(struct avl_node)");
static_assert(sizeof(struct alloc_header) == MIN_BUCKET / 2,
		"alignof(struct alloc_header) != MIN_BUCKET /2");
#endif

/*
 * Return the bucket size for the given index
 */
size_t indexsz(int idx) {
	return 1 << (MIN_BUCKET_SHIFT + idx);
}

/*
 * Returns the bucket index for the given size.
 * The size doesn't need to be a power of 2;
 * TODO: optimize maybe?
 */
int bucket_index(size_t sz) {
	int i = 0; size_t tmp;
	for (tmp = MIN_BUCKET; tmp < sz; tmp <<= 1, i++);
	return i;
}

/*
 * Standin for a page allocator
 */
void *getpage(void) {
	void *ret;
	if (posix_memalign(&ret, PAGESZ, PAGESZ))
		return NULL;

	return memset(ret, 0, PAGESZ);
}

/*
 * Standin for a page allocator
 */
void releasepage(void *ptr) {
	free(ptr);
}

/*
 * Allocate a block, split it, return one half, and insert the other half into
 * the approproate bucket
 */
struct alloc_header *split_bucket(int idx) {
	void *node;
	struct avl_node *half;
	struct alloc_header *hdr;
	size_t sz = indexsz(idx);

	assert(idx > 0 && idx <= NUM_BUCKETS);

	if (idx == NUM_BUCKETS) {
		if (!(node = getpage())) exit(1); /* TODO: OOM */
	} else {
		if ((node = roots[idx]))
			avl_delete(&roots[idx], node);
		else
			node = split_bucket(idx + 1);
	}

	half = (void *)((char *)node + sz / 2);
	avl_insert(&roots[idx - 1], half, 0);

	hdr = (void *)node;
	hdr->sz = sz / 2;

	return hdr;
}

void *avl_alloc(size_t sz) {
	int i;
	size_t reqsz = sz;
	struct alloc_header *hdr;

	sz += sizeof(struct alloc_header);
	if (sz > MAX_BUCKET)
		return NULL;

	i = bucket_index(sz);
	sz = indexsz(i);

	if (!roots[i]) {
		hdr = split_bucket(i + 1);
	} else {
		struct avl_node *node = roots[i];
		avl_delete(&roots[i], node);

		hdr = (struct alloc_header *)node;
		hdr->sz = sz;
	}

	hdr->reqsz = reqsz;
	return (char *)hdr + sizeof(struct alloc_header);
}

struct alloc_header *find_half(struct alloc_header *hdr) {
	int isleft = !((uintptr_t)hdr % (hdr->sz * 2));
	int i = bucket_index(hdr->sz);
	struct alloc_header *node = NULL;

	if (isleft) {
		node = (void *)avl_find_node(roots[i], (uintptr_t)hdr + hdr->sz);
		if (node) {
			avl_delete(&roots[i], node);
			hdr->sz = hdr->sz << 1;

			node = hdr;
		}
	} else {
		node = (void *)avl_find_node(roots[i], (uintptr_t)hdr - hdr->sz);
		if (node) {
			avl_delete(&roots[i], node);
			node->sz = hdr->sz << 1;
		}
	}

	return node;
}

void avl_free_hdr(struct alloc_header *hdr) {
	struct alloc_header *half;
	int i;

	if (!hdr) return;
	i = bucket_index(hdr->sz);

	half = find_half(hdr);
	if (!half) {
		avl_insert(&roots[i], hdr, 0);
		return;
	}

	if (half->sz == PAGESZ) {
		releasepage(half);
		return;
	}

	/* avl_insert(&roots[i + 1], half, 0); */
	avl_free_hdr(half);
}

/*
 * TODO: Think about concurrency
 */
void avl_free(void *ptr) {
	avl_free_hdr(ALLOC_HDR(ptr));
}

void *avl_realloc(void *ptr, size_t sz) {
	struct alloc_header *hdr;
	size_t oldavailsz;

	if (!ptr)
		return avl_alloc(sz);

	if (!sz) {
		avl_free(ptr);
		return NULL;
	}

	hdr = ALLOC_HDR(ptr);
	oldavailsz = hdr->sz - sizeof(struct alloc_header);

	if (oldavailsz < sz) {
		void *newptr = avl_alloc(sz);

		if (!newptr)
			return NULL;

		memcpy(newptr, ptr, hdr->reqsz);
		ALLOC_HDR(newptr)->reqsz = sz;

		avl_free(ptr);
		return newptr;
	}

	hdr->reqsz = sz;
	return ptr;
}
