set debuginfod enabled off
file ./avl


skip avl_insert
skip avl_delete
skip avl_find_node
skip bucket_index
skip indexsz
skip enable

b main
b avl_alloc
b avl_free
r
