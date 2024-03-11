#include <stdio.h>
#include <stdlib.h>

#include "avl.h"

void print_avl_dot_h(FILE *f, struct avl_node *root) {
	if (root) {
		fprintf(f, "%ld [label=\"%ld\"];\n", root->key, root->key);
		if (root->left)
			fprintf(f, "%ld -> %ld [label=\"L\"];\n", root->key,
					root->left->key);
		if (root->right)
			fprintf(f, "%ld -> %ld [label=\"R\"];\n", root->key,
					root->right->key);

		print_avl_dot_h(f, root->left);
		print_avl_dot_h(f, root->right);
	}
}

void print_avl_dot(char *fname, struct avl_node *root) {
	FILE *f = fopen(fname, "w");
	fprintf(f, "digraph G {\n");
	print_avl_dot_h(f, root);
	fprintf(f, "}\n");
	fclose(f);
}

void print_avl(int level, struct avl_node *root) {
	if (root) {
		for (int i = 0; i < level; i++)
			printf("\t");
		printf("%ld\n", root->key);

		print_avl(level + 1, root->left);
		print_avl(level + 1, root->right);
	}
}

int main() {
	struct avl_node *root = NULL;

	srand(10);
	for (int i = 0; i < 14; i++)
		avl_insert(&root, rand(), i);

	print_avl_dot("avl.dot", root);

	srand(10);
	for (int i = 0; i < 14; i++)
		avl_delete(&root, rand());


	print_avl(0, root);

	printf("%d\n", RAND_MAX);
}
