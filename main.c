#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "avl_alloc.h"

#define malloc avl_alloc
#define free avl_free

struct llist {
	struct llist *next;
	int val;
};

void llist_insert(struct llist **root, int val) {
	struct llist *node = malloc(sizeof *node);
	node->next = *root;
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
	struct llist *root = NULL; /* , *tmp; */

	for (i = 0; i < 4; i++)
		llist_insert(&root, i);

	llist_free(root);
	return 0;
}
