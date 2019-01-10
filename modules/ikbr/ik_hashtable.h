#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/gfp.h>
#include <linux/types.h>
#include <linux/list.h>
#include <linux/rculist.h>

/* Hash-table element to be included in structures in a hash table. */
struct ht_elem {
	struct rcu_head rh;
	struct hlist_node hte_next;
	unsigned long hte_hash;
};

/* Hash-table bucket element. */
struct ht_bucket {
	struct hlist_head htb_head;
	spinlock_t htb_lock;
};

/* Top-level hash-table data structure, including buckets. */
struct hashtab {
	unsigned long ht_nbuckets;
	int (*ht_cmp)(struct ht_elem *htep, void *key);
	struct ht_bucket ht_bkt[0];
};

/* Map from hash value to corresponding bucket. */
#define HASH2BKT(htp, h) (&(htp)->ht_bkt[h % (htp)->ht_nbuckets])

/* Underlying lock/unlock functions. */
static void hashtab_lock(struct hashtab *htp, unsigned long hash)
{
	spin_lock(&HASH2BKT(htp, hash)->htb_lock);
}

static void hashtab_unlock(struct hashtab *htp, unsigned long hash)
{
	spin_unlock(&HASH2BKT(htp, hash)->htb_lock);
}

/* Read-side lock/unlock functions. */
static void hashtab_lock_lookup(struct hashtab *htp, unsigned long hash)
{
	rcu_read_lock();
}

static void hashtab_unlock_lookup(struct hashtab *htp, unsigned long hash)
{
	rcu_read_unlock();
}

/* Update-side lock/unlock functions. */
static void hashtab_lock_mod(struct hashtab *htp, unsigned long hash)
{
	hashtab_lock(htp, hash);
}

static void hashtab_unlock_mod(struct hashtab *htp, unsigned long hash)
{
	hashtab_unlock(htp, hash);
}

/*
 * Finished using a looked up hashtable element.
 */
void hashtab_lookup_done(struct ht_elem *htep)
{
}

/*
 * Look up a key.  Caller must have acquired either a read-side or update-side
 * lock via either hashtab_lock_lookup() or hashtab_lock_mod().  Note that
 * the return is a pointer to the ht_elem: Use offset_of() or equivalent
 * to get a pointer to the full data structure.
 *
 * Note that the caller is responsible for mapping from whatever type
 * of key is in use to an unsigned long, passed in via "hash".
 */
struct ht_elem *hashtab_lookup(struct hashtab *htp, unsigned long hash,
			       void *key)
{
	struct ht_elem *htep;

	hlist_for_each_entry_rcu(htep, &HASH2BKT(htp, hash)->htb_head,
				    hte_next) {
		if (htep->hte_hash != hash)
			continue;
		if (htp->ht_cmp(htep, key))
			return htep;
	}
	return NULL;
}

struct ht_elem *hashtab_iter_all(struct hashtab *htp, int (*iter)(struct ht_elem *hte, void *data), void *data, uint32_t *bkt_idx)
{
	struct ht_bucket *htb = NULL;
	struct ht_elem *htep = NULL;
	struct hlist_node *tmp = NULL;

	if (!htp)
		return NULL;

	for (; *bkt_idx < htp->ht_nbuckets; (*bkt_idx)++) {
		htb = &htp->ht_bkt[*bkt_idx];
		spin_lock_bh(&htb->htb_lock);
		hlist_for_each_entry_safe(htep, tmp, &htb->htb_head, hte_next) {
			if (iter(htep, data))
				goto found;
		}
		spin_unlock_bh(&htb->htb_lock);
	}

	return NULL;
found:
	spin_unlock_bh(&htb->htb_lock);
	return htep;
}

int hashtab_iter_one_bucket(struct hashtab *htp, int (*iter)(struct ht_elem *hte, void *data), void *data, uint32_t bkt_idx)
{
	struct ht_bucket *htb = NULL;
	struct ht_elem *htep = NULL;
	struct hlist_node *tmp = NULL;

	if (!htp)
		return -1;

	if (bkt_idx >= htp->ht_nbuckets)
		return -1;

	htb = &htp->ht_bkt[bkt_idx];
	spin_lock_bh(&htb->htb_lock);
	hlist_for_each_entry_safe(htep, tmp, &htb->htb_head, hte_next) {
		iter(htep, data);
	}
	spin_unlock_bh(&htb->htb_lock);

	return 0;
}

/*
 * Add an element to the hash table.  Caller must have acquired the
 * update-side lock via hashtab_lock_mod().
 */
void hashtab_add(struct hashtab *htp, unsigned long hash, struct ht_elem *htep)
{
	htep->hte_hash = hash;
	hlist_add_head_rcu(&htep->hte_next, &HASH2BKT(htp, hash)->htb_head);
}

/*
 * Remove the specified element from the hash table.  Caller must have
 * acquired the update-side lock via hashtab_lock_mod().
 */
void hashtab_del(struct ht_elem *htep)
{
	hlist_del_rcu(&htep->hte_next);
}

/*
 * Allocate a new hash table with the specified number of buckets.
 */
struct hashtab *hashtab_alloc(unsigned long nbuckets,
			      int (*cmp)(struct ht_elem *htep, void *key))
{
	struct hashtab *htp;
	int i;

	htp = kmalloc(sizeof(*htp) + nbuckets * sizeof(struct ht_bucket), GFP_KERNEL);
	if (!htp)
		return NULL;
	htp->ht_nbuckets = nbuckets;
	htp->ht_cmp = cmp;
	for (i = 0; i < nbuckets; i++) {
		INIT_HLIST_HEAD(&htp->ht_bkt[i].htb_head);
		spin_lock_init(&htp->ht_bkt[i].htb_lock);
	}
	return htp;
}

/*
 * Free a hash table.  It is the caller's responsibility to ensure that it
 * is empty and no longer in use.
 */
void hashtab_free(struct hashtab *htp)
{
	kfree(htp);
}

static inline unsigned long mac2hash(const uint8_t *mac)
{
	unsigned long hash = 0;

	hash = mac[3];
	hash = (hash << 8) + mac[2];
	hash = (hash << 8) + mac[1];
	hash = (hash << 8) + mac[0];

	return hash;
}
