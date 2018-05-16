#ifndef __IK_UTILS_H__
#define __IK_UTILS_H__

#include "ahocorasick.h"

#define MAX_HOST_LEN (255)
#define IK_INFO_LOG  pr_info

bool ik_is_in_url_trie(AC_TRIE_t *url_trie, const char *host, unsigned int host_len);
int ik_url_trie_update(AC_TRIE_t **updating_trie, char **pattern, size_t nr_pattern);
void ik_url_trie_release(AC_TRIE_t *trie);

#endif
