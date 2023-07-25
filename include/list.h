#ifndef __MY_LIST_H__
#define __MY_LIST_H__

#ifndef offsetof
#define offsetof(type, member) ((unsigned long)&((type*)0)->member)
#endif

#ifndef container_of
#define container_of(ptr, type, member) ({							\
	const typeof( ((type*)0)->member ) *__mptr = (ptr);				\
	(type *)((char*)__mptr - offsetof(type, member)); })
#endif

#ifndef NULL
#define NULL 0
#endif

#define LIST_POISON1 ((void *)0x00100100)
#define LIST_POISON2 ((void *)0x00200200)

struct list_head {
	struct list_head* next, * prev;
};

/* ��ʼ�� */
#define LIST_HEAD_INIT(name) { &(name), &(name) }
#define LIST_HEAD(name) struct list_head name = LIST_HEAD_INIT(name)
#define INIT_LIST_HEAD(ptr) do { \
	(ptr)->next = (ptr); (ptr)->prev = (ptr); \
} while (0)

/* �����½ڵ� */
static inline void __list_add(struct list_head* new,
							  struct list_head* prev,
							  struct list_head* next)
{
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}
static inline void list_add(struct list_head* new, struct list_head* head)
{
	__list_add(new, head, head->next);
}
static inline void list_add_tail(struct list_head* new, struct list_head* head)
{
	__list_add(new, head->prev, head);
}

/* ɾ���ڵ� */
static inline void __list_del(struct list_head* prev, struct list_head* next)
{
	prev->next = next;
	next->prev = prev;
}
static inline void list_del(struct list_head* entry)
{
	__list_del(entry->prev, entry->next);
	entry->next = LIST_POISON1;
	entry->prev = LIST_POISON2;
}
static inline void list_del_init(struct list_head* entry)
{
	__list_del(entry->prev, entry->next);
	INIT_LIST_HEAD(entry);
}
static inline void list_replace(struct list_head* old, struct list_head* new)
{
	new->next = old->next;
	new->prev = old->prev;
	new->next->prev = new;
	new->prev->next = new;
}
static inline void list_move(struct list_head* list, struct list_head* head)
{
	__list_del(list->prev, list->next);
	list_add(list, head);
}
static inline void list_move_tail(struct list_head* list, struct list_head* head)
{
	__list_list(list->prev, list->next);
	list_add_tail(list, head);
}

/* �ж�Ϊ�գ�ע�⣺���������������Ԫ�ض��Ǵ�β����ӵĻ�����ú����Է���Ϊ�� */
static inline int list_empty(const struct list_head* head)
{
	return head->next == head;
}
static inline int list_empty_careful(const struct list_head* head)
{
	struct list_head* next = head->next;
	return (next == head) && (next == head->prev);
}

/* �ϲ�����: ��listΪͷ��������ϲ���head�����У�ע��list�ڵ㱾������ӽ�ȥ */
static inline void __list_splice(struct list_head* list, struct list_head* head)
{
	struct list_head* first = list->next;
	struct list_head* last = list->prev;
	struct list_head* at = head->next;

	first->prev = head;
	head->next = first;

	last->next = at;
	at->prev = last;
}
static inline void list_splice(struct list_head* list, struct list_head* head)
{
	if (!list_empty(list))
		__list_splice(list, head);
}
static inline void list_splice_init(struct list_head* list, struct list_head* head)
{
	if (!list_empty(list)) {
		__list_splice(list, head);
		INIT_LIST_HEAD(list);
	}
}

/*
* @ptr:		Ϊstruct list_head��type�еĳ�Ա������ָ��
* @type:	Ϊ����
* @member:	Ϊptr��type�еĳ�Ա��
*/
#define list_entry(ptr, type, member) container_of(ptr, type, member)

/*
* @pos:		Ϊstruct list_headָ�����
* @head:	Ϊstruct list_headͷָ���ַ
*/
#define list_for_each(pos, head)	\
	for (pos = (head)->next; pos != (head); pos = pos->next)

/*
* @pos:		Ϊ����ָ�����
* @head:	Ϊstruct headͷָ���ַ
* @member:	Ϊstruct list_head��Ա�����������еĳ�Ա��
*/
#define list_for_each_entry(pos, head, member)	\
	for (pos = list_entry((head)->next, typeof(*pos), member);		\
		 &pos->member != (head);									\
		 pos = list_entry(pos->member.next, typeof(*pos), member))

/*
* @pos:		Ϊ����ָ�����
* @head:	Ϊstruct headͷָ���ַ
* @member:	Ϊstruct list_head��Ա�����������еĳ�Ա��
*/
#define list_for_each_entry_reverse(pos, head, member)	\
	for (pos = list_entry((head)->prev, typeof(*pos), member);		\
		 &pos->member != head;										\
		 pos = list_entry(pos->member.prev, typeof(*pos), member))


struct hlist_head {
	struct hlist_node* first;
};

struct hlist_node {
	struct hlist_node* next, ** pprev;
};

#define HLIST_HEAD_INIT { .first = NULL }
#define HLIST_HEAD(name) struct hlist_head name = { .first = NULL }
#define INIT_HLIST_HEAD(ptr) ((ptr)->first = NULL)
#define INIT_HLIST_NODE(ptr) ((ptr)->next = NULL, (ptr)->pprev = NULL)

static inline int hlist_unhash(const struct hlist_node* h)
{
	return !h->pprev;
}

static inline int hlist_empty(const struct hlist_head* h)
{
	return !h->first;
}

static inline void __hlist_del(struct hlist_node* n)
{
	struct hlist_node* next = n->next;
	struct hlist_node** pprev = n->pprev;
	*pprev = next;
	if (next)
		next->pprev = pprev;
}
static inline void hlist_del(struct hlist_node* n)
{
	__hlist_del(n);
	n->next = LIST_POISON1;
	n->pprev = LIST_POISON2;
}
static inline void hlist_del_init(struct hlist_node* n)
{
	if (n->pprev) {
		__hlist_del(n);
		INIT_HLIST_NODE(n);
	}
}

static inline void hlist_add_head(struct hlist_node* n, struct hlist_head* h)
{
	struct hlist_node* first = h->first;
	n->next = first;
	if (first)
		first->pprev = &n->next;
	h->first = n;
	n->pprev = &h->first;
}
/* ��n��ӵ�nextǰ�棬next���벻Ϊ�� */
static inline void hlist_add_before(struct hlist_node* n, struct hlist_node* next)
{
	n->pprev = next->pprev;
	*(n->pprev) = n;
	n->next = next;
	next->pprev = &n->next;
}

/*
* @ptr:		Ϊstruct hlist_node��type�еĳ�Ա������ָ��
* @type:	Ϊ����
* @member:	Ϊptr��type�еĳ�Ա��
*/
#define hlist_entry(ptr, type, member) container_of(ptr, type, member)

/*
* @pos:		Ϊstruct hlist_node ָ�����
* @head:	Ϊstruct hlist_headָ�����
*/
#define hlist_for_each(pos, head)	\
	for (pos = (head)->first; pos; pos = pos->next)

/*
* @tpos:	Ϊ����ָ�����
* @pos:		Ϊstruct hlist_nodeָ�����
* @head:	Ϊstruct hlist_headָ�����
* @member:	Ϊstruct hlist_node�������еĳ�Ա��
*/
#define hlist_for_each_entry(tpos, pos, head, member)			\
	for (pos = (head)->first;									\
		 pos && tpos = hlist_entry(pos, typeof(*tpos), member);	\
		 pos = pos->next)


#endif
