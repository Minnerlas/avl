#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "avl_alloc.h"

#define malloc avl_alloc
#define free avl_free


struct llist {
	struct llist *next;
	int val;
};

void llist_insert(struct llist **root, int val) {
	struct llist *node = malloc(sizeof *node);
	assert(node);
	node->next = *root;
	node->val = val;
	*root = node;
}

void llist_free(struct llist *root) {
	struct llist *node;
	while (root) {
		node = root;
		root = root->next;
		free(node);
	}
}

int main(void) {
	int i;
	int listlen = 150;
	struct llist **lists = malloc(listlen * sizeof(*lists));
	int rnd;

	assert(lists);

	memset(lists, 0, listlen * sizeof *lists);

	srand(123);
	/* srand(time(NULL)); */

	for (i = 0; i < listlen * listlen * listlen; i++) {
		rnd = rand();
		llist_insert(&lists[rnd % listlen], rnd % listlen);
	}

	for (i = 0; i < listlen; i++)
		llist_free(lists[i]);

	free(lists);
	return 0;
}
