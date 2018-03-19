#include "ahocorasick.h"

char *sample_patterns[] = {
    "*.baidu.com",
    "www.qq.com",
    "baidu.com",
    "www.163.com.cn",
};
#define PATTERN_COUNT (sizeof(sample_patterns)/sizeof(char*))

AC_TRIE_t *fuzzy_trie;
AC_TRIE_t *accurate_trie;

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

void create_ac_tries(char **patterns)
{
	unsigned int i;

	/* Get new tries */
	fuzzy_trie = ac_trie_create();
	accurate_trie = ac_trie_create();

	for (i = 0; i < PATTERN_COUNT; i++) {
		AC_PATTERN_t patt;
		char *pattern = patterns[i];
		int nr_dot = dot_num(pattern, strlen(pattern));
		bool is_fuzzy = false;

		if (nr_dot < 1) {
			continue;
		}

		if (nr_dot == 1) {
			is_fuzzy = true;
		}

		if (pattern[0] == '*') {
			int j = 0;
			is_fuzzy = true;
			while (*pattern != '.' && *pattern != '\0') {
				pattern++;
				j++;
				if (j >= MAX_HOST_LEN) {
					break;
				}
			}
			if (j >= MAX_HOST_LEN) {
				continue;
			}
			if (*pattern == '\0') {
				continue;
			}
			pattern++;
		}

		/* Fill the pattern data */
		patt.ptext.astring = pattern;
		patt.ptext.length = strlen(patt.ptext.astring);

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
	ac_trie_display(fuzzy_trie);
	ac_trie_display(accurate_trie);
}

int main (int argc, char **argv)
{
	AC_TEXT_t chunk;
	AC_MATCH_t match;
	const char *chunk1 = "map.baidu.com";

	create_ac_tries(sample_patterns);
	pr_info ("Searching: \"%s\"\n", chunk1);

	chunk.astring = chunk1;
	chunk.length = strlen(chunk.astring);

	/* Set the input text */
	ac_trie_settext (fuzzy_trie, &chunk, 0);

	/* The ownership of the input text belongs to the caller program. I.e. the
	 * API does not make a copy of that. It must remain valid until the end
	 * of search of the given chunk. */

	/* Find matches */
	while ((match = ac_trie_findnext(fuzzy_trie)).size) {
		print_match (&match);
	}

	/* You may release the automata after you have done with it. */
	ac_trie_release (fuzzy_trie);
	ac_trie_release(accurate_trie);

	return 0;
}
