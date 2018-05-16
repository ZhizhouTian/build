#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/rcupdate.h>
#include <linux/seq_file.h>

#include "ik_utils.h"

static bool is_url_format_valid(const char *str, int len)
{
	int i;
	int ndot = 0;

	if (len > MAX_HOST_LEN)
		return false;

	for (i = 0; i < len; i++) {
		if (str[i] == '.') {
			ndot++;
		}
	}

	return ndot >= 1;
}

bool ik_is_in_url_trie(AC_TRIE_t *url_trie, const char *host, unsigned int host_len)
{
	AC_TEXT_t chunk;
	AC_MATCH_t match;
	AC_TRIE_t *trie = rcu_dereference(url_trie);
	const char *match_pattern;
	size_t match_len;
	bool ret = true;

	if (!trie) {
		return false;
	}

	if (!is_url_format_valid(host, host_len)) {
		return false;
	}

	rcu_read_lock();

	chunk.astring = host;
	chunk.length = host_len;
	ac_trie_settext(trie, &chunk, 0);

	match = ac_trie_findnext(trie);
	if (!match.size) {
		ret = false;
		goto end;
	}
	match_pattern = match.patterns->ptext.astring;
	match_len = strnlen(match_pattern, MAX_HOST_LEN);
	if (match_len == host_len) {
		ret = true;
	} else if (match_len < host_len) {
		const char *phost = host + host_len - match_len;
		if (strncmp(phost, match_pattern, match_len)) {
			ret = false;
			goto end;
		}
		ret = *(phost-1) == '.';
	} else {
		ret = false;
	}

end:
	rcu_read_unlock();
	return ret;
}

int ik_url_trie_update(AC_TRIE_t **updating_trie, char **patterns, size_t nr_patterns)
{
	unsigned int i;
	AC_TRIE_t *new_trie, *old_trie;

	/* Get new tries */
	new_trie = ac_trie_create();

	if (!new_trie) {
		IK_INFO_LOG("[URL Trie]: allocate url ac-trie fail\n");
		return -ENOMEM;
	}

	for (i = 0; i < nr_patterns; i++) {
		AC_PATTERN_t patt;
		char *pattern = patterns[i];
		size_t plen = strnlen(pattern, MAX_HOST_LEN);

		if (!is_url_format_valid(pattern, plen)) {
			IK_INFO_LOG("[URL Trie]: invalid format url:%s\n", pattern);
			continue;
		}

		if (pattern[0] == '*') {
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

		IK_INFO_LOG("[URL Trie]: add pattern %s\n", pattern);
		/* The replacement pattern is not applicable in this program, so better 
		 * to initialize it with 0 */
		patt.rtext.astring = NULL;
		patt.rtext.length = 0;

		/* Pattern identifier is optional */
		patt.id.u.number = i + 1;
		patt.id.type = AC_PATTID_TYPE_NUMBER;

		/* Add pattern to automata */
		ac_trie_add(new_trie, &patt, 1, NULL);

		/* We added pattern with copy option disabled. It means that the
		 * pattern memory must remain valid inside our program until the end of 
		 * search. If you are using a temporary buffer for patterns then you 
		 * may want to make a copy of it so you can use it later. */
	}

	/* Now the preprocessing stage ends. You must finalize the trie. Remember
	 * that you can not add patterns anymore. */
	ac_trie_finalize(new_trie);

	/* Finalizing the trie is the slowest part of the task. It may take a
	 * longer time for a very large number of patters */

	/* Display the trie if you wish */
	//ac_trie_display(new_trie);
	old_trie = rcu_dereference(*updating_trie);
	rcu_assign_pointer(*updating_trie, new_trie);

	synchronize_rcu();
	if (old_trie) {
		ac_trie_release(old_trie);
	}

	return 0;
}

void ik_url_trie_release(AC_TRIE_t *trie)
{
	if (trie) {
		ac_trie_release(trie);
	}
}
