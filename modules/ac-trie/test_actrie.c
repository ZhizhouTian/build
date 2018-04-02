#include "ahocorasick.h"

char *sample_patterns[] = {
    "baidu.com",
    "www.163.com.cn",
};
#define PATTERN_COUNT (sizeof(sample_patterns)/sizeof(char*))

AC_TRIE_t *g_fuzzy_trie;
AC_TRIE_t *g_accurate_trie;

#define MAX_HOST_LEN 512

int dot_num(const char *str, int len) {
	int i;
	int ndot = 0;

	if (len > MAX_HOST_LEN)
		len = MAX_HOST_LEN;

	for (i = 0; i < len; i++) {
		if (str[i] == '.') {
			ndot++;
		}
	}

	return ndot;
}

void print_match (AC_MATCH_t *m)
{
	unsigned int j;
	AC_PATTERN_t *pp;

	pr_info ("@%2lu found: ", m->position);

	for (j = 0; j < m->size; j++)
	{
		pp = &m->patterns[j];

		pr_info("#%ld \"%.*s\", ", pp->id.u.number,
				(int)pp->ptext.length, pp->ptext.astring);

		/* CAUTION: the AC_PATTERN_t::ptext.astring pointers, point to the 
		 * sample patters in our program, since we added patterns with copy 
		 * option disabled.
		 */
	}

	pr_info ("\n");
}

int us_create_ac_tries(char **patterns, size_t patterns_len)
{
	unsigned int i;
	AC_TRIE_t *fuzzy_trie, *old_fuzzy_trie;
	AC_TRIE_t *accurate_trie, *old_accurate_trie;

	/* Get new tries */
	fuzzy_trie = ac_trie_create();
	accurate_trie = ac_trie_create();

	if (!fuzzy_trie || !accurate_trie)
		return -ENOMEM;

	for (i = 0; i < patterns_len; i++) {
		AC_PATTERN_t patt;
		char *pattern = patterns[i];
		size_t plen = strnlen(pattern, MAX_HOST_LEN);
		int nr_dot = dot_num(pattern, plen);
		bool is_fuzzy = false;

		if (nr_dot < 1) {
			continue;
		}

		if (nr_dot == 1) {
			is_fuzzy = true;
		}

		if (pattern[0] == '*') {
			is_fuzzy = true;
			do {
				pattern++;
				plen--;
			} while (*pattern != '.');
			pattern++;
			plen--;
		}

		/* Fill the pattern data */
		patt.ptext.astring = pattern;
		patt.ptext.length = plen;

		/* The replacement pattern is not applicable in this program, so better 
		 * to initialize it with 0 */
		patt.rtext.astring = NULL;
		patt.rtext.length = 0;

		/* Pattern identifier is optional */
		patt.id.u.number = i + 1;
		patt.id.type = AC_PATTID_TYPE_NUMBER;

		/* Add pattern to automata */
		if (is_fuzzy) {
			ac_trie_add(fuzzy_trie, &patt, 1, NULL);
		} else {
			ac_trie_add(accurate_trie, &patt, 1, NULL);
		}

		/* We added pattern with copy option disabled. It means that the 
		 * pattern memory must remain valid inside our program until the end of 
		 * search. If you are using a temporary buffer for patterns then you 
		 * may want to make a copy of it so you can use it later. */
	}

	/* Now the preprocessing stage ends. You must finalize the trie. Remember 
	 * that you can not add patterns anymore. */
	ac_trie_finalize(fuzzy_trie);
	ac_trie_finalize(accurate_trie);

	/* Finalizing the trie is the slowest part of the task. It may take a 
	 * longer time for a very large number of patters */

	/* Display the trie if you wish */
	//ac_trie_display(fuzzy_trie);
	//ac_trie_display(accurate_trie);
	old_fuzzy_trie = rcu_dereference(g_fuzzy_trie);
	old_accurate_trie = rcu_dereference(g_accurate_trie);
	rcu_assign_pointer(g_fuzzy_trie, fuzzy_trie);
	rcu_assign_pointer(g_accurate_trie, accurate_trie);

	synchronize_rcu();
	if (old_fuzzy_trie)
		ac_trie_release(old_fuzzy_trie);
	if (old_accurate_trie)
		ac_trie_release(old_accurate_trie);

	return 0;
}

static bool us_is_host_in_whitelist(const char *host, unsigned int host_len)
{
	AC_TEXT_t chunk;
	AC_MATCH_t match;
	const char *phost = host;
	unsigned int len = host_len;
	int nr_dot;
	AC_TRIE_t *fuzzy_trie = rcu_dereference(g_fuzzy_trie);
	AC_TRIE_t *accurate_trie = rcu_dereference(g_accurate_trie);

	if (!fuzzy_trie || !accurate_trie)
		return false;

	if (host_len > MAX_HOST_LEN)
		return false;

	nr_dot = dot_num(host, host_len);

	if (nr_dot < 1)
		return false;

	/* fuzzy match */
	if (nr_dot > 1) {
		while (*phost != '.') {
			phost++;
			len--;
		}
		phost++;
		len--;
	}

	rcu_read_lock();

	chunk.astring = phost;
	chunk.length = len;
	ac_trie_settext (fuzzy_trie, &chunk, 0);

	if ((match = ac_trie_findnext(fuzzy_trie)).size) {
		rcu_read_unlock();
		return true;
	}

	/* accurate match */
	chunk.astring = host;
	chunk.length = host_len;
	ac_trie_settext (accurate_trie, &chunk, 0);
	if ((match = ac_trie_findnext(accurate_trie)).size) {
		rcu_read_unlock();
		return true;
	}

	rcu_read_unlock();
	return false;
}

static int __init test_actrie_init(void)
{
	const char *chunk1 = "map.baidu.com.cn";
	const char *chunk2 = "163.com";
	const char *chunk3 = "a.baidu.com";
	bool is_in_whitelist = false;

	us_create_ac_tries(sample_patterns, PATTERN_COUNT);

	is_in_whitelist = us_is_host_in_whitelist(chunk1, strlen(chunk1));
	pr_info("%s %s in whiltelist\n", chunk1, is_in_whitelist?"is":"is not");

	is_in_whitelist = us_is_host_in_whitelist(chunk2, strlen(chunk2));
	pr_info("%s %s in whiltelist\n", chunk2, is_in_whitelist?"is":"is not");

	is_in_whitelist = us_is_host_in_whitelist(chunk3, strlen(chunk3));
	pr_info("%s %s in whiltelist\n", chunk3, is_in_whitelist?"is":"is not");

	return 0;
}

static void __exit test_actrie_exit(void)
{
	/* You may release the automata after you have done with it. */
	if (g_fuzzy_trie)
		ac_trie_release(g_fuzzy_trie);
	if (g_accurate_trie)
		ac_trie_release(g_accurate_trie);
}

module_init(test_actrie_init);
module_exit(test_actrie_exit);
MODULE_LICENSE("GPL");
