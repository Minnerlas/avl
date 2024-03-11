#include "avl.h"
#include <stdio.h>
#include <stdlib.h>

#define ABORT(...) do { fprintf(stderr, __VA_ARGS__); exit(1); } while (0)

int avl_bf(struct avl_node *node) {
	return (int)(node->parbf & 3) - 1;
}


struct avl_node *avl_parent(struct avl_node *node) {
	return (struct avl_node *)(node->parbf & ~3);
}

struct avl_node *avl_child(struct avl_node *node, int sign) {
	return sign < 0 ? node->left : node->right;
}

void avl_set_child(struct avl_node *parent, int sign, struct avl_node *child) {
	if (sign < 0)
		parent->left = child;
	else
		parent->right = child;
}

void avl_set_parent(struct avl_node *node, struct avl_node *parent) {
	node->parbf = (uintptr_t)parent | (node->parbf & 3);
}

void avl_replace_child(struct avl_node **rootp, struct avl_node *parent,
		struct avl_node *old_child, struct avl_node *new_child) {
	if (parent) {
		if (old_child == parent->left)
			parent->left = new_child;
		else
			parent->right = new_child;
	} else
		*rootp = new_child;
}

void avl_set_parent_bf(struct avl_node *node, struct avl_node *parent,
		int balance_factor) {
	node->parbf = (uintptr_t)parent | (balance_factor + 1);
}

void avl_rotate(struct avl_node **rootp, struct avl_node *A, int sign) {
	struct avl_node *P = avl_parent(A);
	struct avl_node *B = avl_child(A, -sign);
	struct avl_node *E = avl_child(B, +sign);

	avl_set_child(A, -sign, E);
	avl_set_parent(A, B);

	avl_set_child(B, +sign, A);
	avl_set_parent(B, P);

	if (E)
		avl_set_parent(E, A);

	avl_replace_child(rootp, P, A, B);
}

struct avl_node *avl_double_rotate(struct avl_node **root_ptr, struct avl_node *B,
		struct avl_node *A, int sign) {
	struct avl_node *E = avl_child(B, +sign);
	struct avl_node *F = avl_child(E, -sign);
	struct avl_node *G = avl_child(E, +sign);
	struct avl_node *P = avl_parent(A);
	int e = avl_bf(E);

	avl_set_child(A, -sign, G);
	avl_set_parent_bf(A, E, ((sign * e >= 0) ? 0 : -e));

	avl_set_child(B, +sign, F);
	avl_set_parent_bf(B, E, ((sign * e <= 0) ? 0 : -e));

	avl_set_child(E, +sign, A);
	avl_set_child(E, -sign, B);
	avl_set_parent_bf(E, P, 0);

	if (G)
		avl_set_parent(G, A);

	if (F)
		avl_set_parent(F, B);

	avl_replace_child(root_ptr, P, A, E);

	return E;
}

int avl_ins_rebalance(struct avl_node **rootp, struct avl_node *node,
		int sign) {
	struct avl_node *parent = avl_parent(node);

	int oldbf = avl_bf(parent);
	int newbf = oldbf + sign;

	if (oldbf == 0) {
		parent->parbf += sign;
		return 0;
	}

	if (newbf == 0) {
		parent->parbf += sign;
		return 1;
	}

	if (sign * avl_bf(node) > 0) {
		avl_rotate(rootp, parent, -sign);
		parent->parbf -= sign;
		node->parbf -= sign;
	} else {
		avl_double_rotate(rootp, node, parent, -sign);
	}

	return 1;
}

void avl_insert_rebalance(struct avl_node **root, struct avl_node *new) {
	struct avl_node *node = new, *parent = avl_parent(node);

	new->left = new->right = NULL;

	if (!parent)
		return;

	if (parent->left == node)
		parent->parbf--; /* Should be fine */
	else
		parent->parbf++;

	if (avl_bf(parent) == 0)
		return;

	for (;;) {
		node = parent;
		parent = avl_parent(node);
		if (!parent)
			break;

		if (parent->left == node) {
			if (avl_ins_rebalance(root, node, -1))
				break;
		} else {
			if (avl_ins_rebalance(root, node, +1))
				break;
		}
	}
}

void avl_insert(struct avl_node **root, uint64_t key, uint64_t value) {
	struct avl_node *cur = NULL, **curp = root;

	while (*curp) {
		cur = *curp;

		if (key < cur->key) {
			curp = &cur->left;
		} else if (key > cur->key) {
			curp = &cur->right;
		} else
			ABORT("Key %lu already exists\n", key);
	}

	*curp = malloc(sizeof **curp); /* NOTE: This is where we change stuff */
	if (!*curp) ABORT("Out of memory\n");

	**curp = (struct avl_node) {
		.key = key,
		.value = value,
		.parbf = (uintptr_t)cur | 1, /* BF of 0 */
	};
	avl_insert_rebalance(root, *curp);
}

struct avl_node *avl_find_node(struct avl_node *root, uint64_t key) {
	struct avl_node *cur = root;

	while (cur) {
		if (key < cur->key)
			cur = cur->left;
		else if (key > cur->key)
			cur = cur->right;
		else
			return cur;
	}

	return NULL;
}

uint64_t avl_find(struct avl_node *root, uint64_t key) {
	struct avl_node *node = avl_find_node(root, key);
	if (!node)
		return 0; /* TODO wat do */

	return node->value;
}

struct avl_node *avl_swap_successor(struct avl_node **root, struct avl_node *X,
		int *left_deleted) {
	struct avl_node *Y, *ret, *Q;

	Y = X->right;
	if (!Y->left) {
		ret = Y;
		*left_deleted = 0;
	} else {
		do {
			Q = Y;
			Y = Y->left;
		} while (Y->left);

		Q->left = Y->right;
		if (Q->left)
			avl_set_parent(Q->left, Q);
		Y->right = X->right;
		avl_set_parent(X->right, Y);
		ret = Q;
		*left_deleted = 1;
	}

	Y->left = X->left;
	avl_set_parent(X->left, Y);

	Y->parbf = X->parbf;
	avl_replace_child(root, avl_parent(X), X, Y);

	return ret;
}

struct avl_node *avl_del_rebalance(struct avl_node **root, struct avl_node *parent,
		int sign, int *left_deleted) {
	struct avl_node *node;

	int oldbf = avl_bf(parent);
	int newbf = oldbf + sign;

	if (oldbf == 0) {
		parent->parbf += sign;
		return NULL;
	}

	if (newbf == 0) {
		parent->parbf += sign;
		node = parent;
	} else {
		node = avl_child(parent, newbf);

		if (sign * avl_bf(node) >= 0) {
			avl_rotate(root, parent, -sign);
			if (avl_bf(parent) == 0) {
				parent->parbf -= sign;
				return NULL;
			} else {
				parent->parbf -= sign;
				node->parbf -= sign;
			}
		} else {
			node = avl_double_rotate(root, node, parent, -sign);
		}
	}

	parent = avl_parent(node);
	if (parent)
		*left_deleted = (node == parent->left);

	return parent;
}

void avl_delete_node(struct avl_node **root, struct avl_node *node) {
	struct avl_node *parent, *child;
	int left_deleted = 0;

	if (node->left && node->right) {
		parent = avl_swap_successor(root, node, &left_deleted);
	} else {
		child = node->left ? node->left : node->right;
		parent = avl_parent(node);

		if (parent) {
			if (parent->left == node) {
				parent->left = child;
				left_deleted = 1;
			} else {
				parent->right = child;
				left_deleted = 0;
			}

			if (child)
				avl_set_parent(child, parent);
		} else {
			if (child)
				avl_set_parent(child, parent);

			*root = child;
			return;
		}
	}

	do {
		if (left_deleted)
			parent = avl_del_rebalance(root, parent, +1, &left_deleted);
		else
			parent = avl_del_rebalance(root, parent, -1, &left_deleted);
	} while (parent);
}

void avl_delete(struct avl_node **root, uint64_t key) {
	struct avl_node *node = avl_find_node(*root, key);
	if (!node)
		return;
	avl_delete_node(root, node);
	free(node); /* NOTE: This is where we do stuff */
}
