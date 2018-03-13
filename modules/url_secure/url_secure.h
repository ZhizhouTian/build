#ifndef __URL_SECURE_H__
#define __URL_SECURE_H__

#define MAX_URL_LEN 2083

#define IK_ENABLE_URL_SECURE_WHITELIST

#ifdef IK_ENABLE_URL_SECURE_WHITELIST
void us_secure_init(void);
void us_secure_exit(void);
void us_start_protoc_data_handle_thread(void *data, uint32_t data_len);
bool us_select_whitelist(const char *url);
void us_whitelist_show(struct seq_file *s);
#else
void us_secure_init(void) {}
void us_secure_exit(void) {}
void us_start_protoc_data_handle_thread(void *data, uint32_t data_len) {}
bool us_select_whitelist(const char *url) {}
void us_whitelist_show(struct seq_file *s) {}
#endif
#endif
