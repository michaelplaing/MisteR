// avltree.h

#ifndef AVLTREE_H_
#define AVLTREE_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct avl_tree_node { // Node in an AVL tree.  Embed this in some other data structure.
    struct avl_tree_node *left; // Pointer to left child or NULL
    struct avl_tree_node *right; // Pointer to right child or NULL
    uintptr_t parent_balance;

    /* Pointer to parent combined with the balance factor.  This saves 4 or
     * 8 bytes of memory depending on the CPU architecture.
     *
     * Low 2 bits:  One greater than the balance factor of this subtree,
     * which is equal to height(right) - height(left).  The mapping is:
     *
     * 00 => -1
     * 01 =>  0
     * 10 => +1
     * 11 => undefined
     *
     * The rest of the bits are the pointer to the parent node which must be
     * 4-byte aligned. It will be NULL if this is the root node and
     * therefore has no parent.  */
};

// Cast an AVL tree node from its offset in the containing data structure.
#define avl_tree_entry(entry, type, member) ((type *) ((char *)(entry) - offsetof(type, member)))

struct avl_tree_node *avl_get_parent(const struct avl_tree_node *node);

static inline void avl_tree_node_set_unlinked(struct avl_tree_node *node);
static inline bool avl_tree_node_is_unlinked(const struct avl_tree_node *node);

/* (Internal use only)  */
static void avl_tree_rebalance_after_insert(struct avl_tree_node **root_ptr, struct avl_tree_node *inserted);

struct avl_tree_node *avl_tree_lookup(
    const struct avl_tree_node *root,
    const void *cmp_ctx,
    int (*cmp)(const void *,
    const struct avl_tree_node *)
);

struct avl_tree_node *avl_tree_lookup_node(
    const struct avl_tree_node *root,
    const struct avl_tree_node *node,
    int (*cmp)(const struct avl_tree_node *,
    const struct avl_tree_node *)
);

struct avl_tree_node *avl_tree_insert(
    struct avl_tree_node **root_ptr,
    struct avl_tree_node *item,
    int (*cmp)(const struct avl_tree_node *, const struct avl_tree_node *)
);

/* Removes an item from the specified AVL tree.
 * See implementation for details.  */
void avl_tree_remove(struct avl_tree_node **root_ptr, struct avl_tree_node *node);

/* Nonrecursive AVL tree traversal functions  */

struct avl_tree_node *avl_tree_first_in_order(const struct avl_tree_node *root);
struct avl_tree_node *avl_tree_last_in_order(const struct avl_tree_node *root);
struct avl_tree_node *avl_tree_next_in_order(const struct avl_tree_node *node);
struct avl_tree_node *avl_tree_prev_in_order(const struct avl_tree_node *node);
struct avl_tree_node *avl_tree_first_in_postorder(const struct avl_tree_node *root);

struct avl_tree_node *avl_tree_next_in_postorder(
    const struct avl_tree_node *prev, const struct avl_tree_node *prev_parent
);

/*
 * Iterate through the nodes in an AVL tree in sorted order.
 * You may not modify the tree during the iteration.
 *
 * @child_struct
 *    Variable that will receive a pointer to each struct inserted into the
 *    tree.
 * @root
 *    Root of the AVL tree.
 * @struct_name
 *    Type of *child_struct.
 * @struct_member
 *    Member of @struct_name type that is the AVL tree node.
 *
 * Example:
 *
 * struct int_wrapper {
 *    int data;
 *    struct avl_tree_node index_node;
 * };
 *
 * void print_ints(struct avl_tree_node *root)
 * {
 *    struct int_wrapper *i;
 *
 *    avl_tree_for_each_in_order(i, root, struct int_wrapper, index_node)
 *        printf("%d\n", i->data);
 * }
 */
#define avl_tree_for_each_in_order(child_struct, root, struct_name, struct_member) \
    for ( \
        struct avl_tree_node *_cur = avl_tree_first_in_order(root); \
        _cur && ((child_struct) = avl_tree_entry(_cur, struct_name, struct_member), 1); \
        _cur = avl_tree_next_in_order(_cur) \
    )

/*
 * Like avl_tree_for_each_in_order(), but uses the reverse order.
 */
#define avl_tree_for_each_in_reverse_order(child_struct, root, struct_name, struct_member) \
    for ( \
        struct avl_tree_node *_cur = avl_tree_last_in_order(root); \
        _cur && ((child_struct) = avl_tree_entry(_cur, struct_name, struct_member), 1); \
        _cur = avl_tree_prev_in_order(_cur) \
    )

/*
 * Like avl_tree_for_each_in_order(), but iterates through the nodes in
 * postorder, so the current node may be deleted or freed.
 */
#define avl_tree_for_each_in_postorder(child_struct, root,        \
                       struct_name, struct_member)    \
    for (struct avl_tree_node *_cur =                \
        avl_tree_first_in_postorder(root), *_parent;        \
         _cur && ((child_struct) =                    \
              avl_tree_entry(_cur, struct_name,            \
                     struct_member), 1)            \
              && (_parent = avl_get_parent(_cur), 1);        \
         _cur = avl_tree_next_in_postorder(_cur, _parent))

#endif /* AVLTREE_H_ */
