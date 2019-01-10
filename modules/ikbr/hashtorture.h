#define CACHE_LINE_SIZE 64
#define malloc(size) kmalloc(size, GFP_KERNEL)
#define free(ptr) kfree(ptr)

/*
 * Test variables.
 */

struct testhe {
	struct ht_elem the_e;
	unsigned long data;
	int in_table __attribute__((__aligned__(CACHE_LINE_SIZE)));
};

void defer_del_done_perftest(struct ht_elem *htep)
{
	struct testhe *p = container_of(htep, struct testhe, the_e);

	p->in_table = 0;
}

void *testgk(struct ht_elem *htep)
{
	struct testhe *thep;

	thep = container_of(htep, struct testhe, the_e);
	return (void *)thep->data;
}

int testcmp(struct ht_elem *htep, void *key)
{
	struct testhe *thep;

	thep = container_of(htep, struct testhe, the_e);
	return ((unsigned long)key) == thep->data;
}

struct testhe *smoketest_malloc(int key)
{
	struct testhe *ep;

	ep = malloc(sizeof(*ep));
	BUG_ON(!ep);
	ep->data = key;
	return ep;
}

void smoketest(void)
{
	struct testhe *e1p;
	struct testhe *e2p;
	struct testhe *e3p;
	struct testhe *e4p;
	struct hashtab *htp;
	struct ht_elem *htep;
	long i;

	htp = hashtab_alloc(5, testcmp);
	BUG_ON(htp == NULL);

	/* Should be empty. */
	for (i = 1; i <= 4; i++) {
		hashtab_lock_lookup(htp, i);
		BUG_ON(hashtab_lookup(htp, (unsigned long)i, (void *)i));
		hashtab_unlock_lookup(htp, i);
	}

	/* Add one by one and check. */
	e1p = smoketest_malloc(1);
	hashtab_lock_mod(htp, 1);
	hashtab_add(htp, 1, &e1p->the_e);
	htep = hashtab_lookup(htp, 1, (void *)1);
	BUG_ON(!htep);
	hashtab_unlock_mod(htp, 1);
	hashtab_lookup_done(htep);
	e2p = smoketest_malloc(2);
	hashtab_lock_mod(htp, 2);
	hashtab_add(htp, 2, &e2p->the_e);
	htep = hashtab_lookup(htp, 2, (void *)2);
	BUG_ON(!htep);
	hashtab_unlock_mod(htp, 2);
	hashtab_lookup_done(htep);
	e3p = smoketest_malloc(3);
	hashtab_lock_mod(htp, 3);
	hashtab_add(htp, 3, &e3p->the_e);
	htep = hashtab_lookup(htp, 3, (void *)3);
	BUG_ON(!htep);
	hashtab_unlock_mod(htp, 3);
	hashtab_lookup_done(htep);
	e4p = smoketest_malloc(4);
	hashtab_lock_mod(htp, 4);
	hashtab_add(htp, 4, &e4p->the_e);
	htep = hashtab_lookup(htp, 4, (void *)4);
	BUG_ON(!htep);
	hashtab_unlock_mod(htp, 4);
	hashtab_lookup_done(htep);

	/* Should be full. */
	for (i = 1; i <= 4; i++) {
		hashtab_lock_lookup(htp, i);
		htep = hashtab_lookup(htp, (unsigned long)i, (void *)i);
		BUG_ON(!htep);
		hashtab_unlock_lookup(htp, i);
		hashtab_lookup_done(htep);
	}

	/* Delete all and check one by one. */
	hashtab_lock_mod(htp, 1);
	hashtab_del(&e1p->the_e);
	BUG_ON(hashtab_lookup(htp, 1, (void *)1));
	hashtab_unlock_mod(htp, 1);
	hashtab_lock_mod(htp, 2);
	hashtab_del(&e2p->the_e);
	BUG_ON(hashtab_lookup(htp, 2, (void *)2));
	hashtab_unlock_mod(htp, 2);
	hashtab_lock_mod(htp, 3);
	hashtab_del(&e3p->the_e);
	BUG_ON(hashtab_lookup(htp, 3, (void *)3));
	hashtab_unlock_mod(htp, 3);
	hashtab_lock_mod(htp, 4);
	hashtab_del(&e4p->the_e);
	BUG_ON(hashtab_lookup(htp, 4, (void *)4));
	hashtab_unlock_mod(htp, 4);

	/* Should be empty. */
	for (i = 1; i <= 4; i++) {
		hashtab_lock_lookup(htp, i);
		BUG_ON(hashtab_lookup(htp, (unsigned long)i, (void *)i));
		hashtab_unlock_lookup(htp, i);
	}
	hashtab_free(htp);
	pr_info("End of smoketest.\n");
}
