#include <time.h>
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

#define COUNT 50

int main(int argc, char **argv) {
	(void)argc;
	(void)argv;

	struct avl_node *root = NULL;
	unsigned seed = time(NULL);
	// unsigned seed = 1710195314;

	printf("SEED = %u\n", seed);

	srand(seed);
	for (int i = 0; i < COUNT; i++)
		avl_insert(&root, rand() % (100 * COUNT), i);

	print_avl_dot("avl.dot", root);

	srand(seed);
	for (int i = 0; i < COUNT; i++) {
		uint64_t key = rand() % (100 * COUNT);
		printf("DELETE %ld\n", key);
		char *path;
		asprintf(&path, "dots/avl-%05d-%05ld.dot", i, key);
		print_avl_dot(path, root);
		free(path);
		avl_delete(&root, key);
	}


	print_avl(0, root);
}
