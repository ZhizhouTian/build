#ifndef __IK_HIGH_PRIO_URL_H__
#define __IK_HIGH_PRIO_URL_H__

void ik_high_prio_url_recv(void *data, unsigned int data_len);
int __init high_prio_url_init(void);
void __exit high_prio_url_exit(void);

#endif
