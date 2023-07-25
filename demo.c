#include "list.h"

#include <stdio.h>
#include <stdlib.h>

typedef struct demo_s {
	int key;
	struct list_head node;
} demo_t;

static demo_t* demo_new(int key)
{
	demo_t* dn;
	dn = (demo_t*)malloc(sizeof(demo_t));
	dn->key = key;
	INIT_LIST_HEAD(&dn->node);
	return dn;
}

int main(int argc, char** argv)
{
	int i;
	demo_t* p;
	demo_t init = {
		.key = 0,
		.node = LIST_HEAD_INIT(init.node)
	};

	for (i = 1; i < 10; i++) {
		demo_t* dn = demo_new(i);
		list_add(&dn->node, &init.node);
	}

	list_for_each_entry_reverse(p, &init.node, node) {
		printf("key: %d\n", p->key);
	}

	return 0;
}