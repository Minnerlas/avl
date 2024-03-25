#ifndef AVL_H
#define AVL_H

#include <stddef.h>
#include <stdint.h>

#define KEY(node) ((uintptr_t)(node))

struct avl_node {
	uint64_t value;

	struct avl_node *left;
	struct avl_node *right;

	/* Pointer to the parent node OR'd with the balance factor
	 * of the node. Balance factor can only be -1, 0 or 1
	 * Which maps like this:
	 * -1 -> 00
	 *  0 -> 01
	 *  1 -> 10
	 */
	uintptr_t parbf;
};

struct avl_node *avl_find_node(struct avl_node *root, uintptr_t key);
void avl_insert(struct avl_node **root, void *key, uint64_t value);
void avl_delete(struct avl_node **root, void *key);

#endif /* AVL_H */
