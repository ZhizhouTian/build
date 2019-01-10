#include <linux/skbuff.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/timer.h>
#include <linux/netdevice.h>
#include <linux/in.h>
#include "ik_hashtable.h"

struct ikbr_auth_mac {
	struct ht_elem ika_hte;
	unsigned long ika_access_time;
	unsigned long ika_auth_time;
	unsigned long ika_tmp_auth_time;
	__u8 ika_mac[ETH_ALEN];
};

static struct hashtab *g_ika_mac_htp;

static int ikbr_auth_mac_cmp(struct ht_elem *htep, void *key)
{
	struct ikbr_auth_mac *am = NULL;
	uint8_t *mac = key;
	am = container_of(htep, struct ikbr_auth_mac, ika_hte);

	return (mac[4] == am->ika_mac[4] && mac[5] == am->ika_mac[5]);
}

struct ikbr_auth_mac* ikbr_auth_mac_lookup(uint8_t *mac)
{
	struct ikbr_auth_mac *am = NULL;
	if (!mac)
		return NULL;
	return am;
}

bool ik_get_real_mac_with_snmp(struct sk_buff *skb, uint8_t *real_mac_buf)
{
	bool ret = false;

	return ret;
}

static bool ikbr_is_local_skb(struct sk_buff *skb)
{
	struct net_device *dev = skb->dev;
	const char *dev_name = NULL;

	if (!dev)
		return true;

	dev_name = dev->name;
	return (!strncmp(dev_name, "la", 2) || !strncmp(dev_name, "ppp", 3) ||
		!strncmp(dev_name, "vlan", 4) || !strncmp(dev_name, "sovp", 4));
}

uint32_t ikbr_auth_hook(struct sk_buff *skb, struct nf_conn *nfconn)
{
	struct ikbr_host *host = NULL;
	struct ikbr_auth_mac *am = NULL;
	uint8_t real_mac[ETH_ALEN] = {0};
	struct iphdr *iph = ip_hdr(skb);
	struct ethhdr *ethh = eth_hdr(skb);

	// 0. 如果不是本地包就略过
	if (!ikbr_is_local_skb(skb))
		goto accept;
	// 1. 检查方向，如果是不是forward就直接返回NF_ACCEPT
	// 2. 当前的mac可能不是skb的真正mac，可以通过ip-mac对照表进行查找
	if (!ik_get_real_mac_with_snmp(skb, real_mac)) {
		memcpy(real_mac, ethh->h_source, ETH_ALEN);
	}
	// 2. 检查链接：已经验证过了就返回NF_ACCEPT

	// 3. 检查host：
	//	3.1 根据mac找到当前nfconn所属于的host，如果没有找到就创建
	// 	3.2 如果当前的host已经经过验证了就返回NF_ACCEPT,需要注意版本
	// 	3.3 否则检查host是否符合auth条件
	// 		3.3.1 获得skb的mac地址，在auth mac table中查找
	// 		3.3.2 获得skb的ip地址，在auth ip table中查找
	// 		3.3.3 如果该host符合条件就设置host，返回NF_ACCEPT
	host = ikbr_host_get_by_mac(real_mac);
	if (!host) {
		host = __ikbr_host_alloc_init(real_mac, iph->saddr, GFP_ATOMIC);
		if (!host) {
			pr_info("[ikbr auth]: alloc ikbr_host failed\n");
			goto drop;
		}
	}
	if (host->ikh_is_authed)
		goto accept;
	am = ikbr_auth_mac_lookup(real_mac);
	if (am) {
		host->ikh_is_authed = true;
		goto accept;
	}
	// 4. 如果是UDP
	// 	4.1 如果是Forward方向：
	//		4.1.1 非53端口则返回NF_DROP
	//		4.1.2 dns则返回NF_ACCEPT
	//	4.2 如果是Reply反向：
	//		如果是53端口则返回NF_ACCEPT
	//	4.3 返回NF_DROP
	if (IPPROTO_UDP == iph->protocol) {
		if (ntohs(skb->sport) == 53 || ntohs(skb->dport) == 53)
			goto accept;
		goto redirect;
	}

	// 5. 如果是TCP:
	//	5.1 对tcp包进行线性化处理
	//	5.2 如果是syn、rst、fin包则放过
	//	5.3 https（443）返回NF_ACCEPT
	//	5.4 分析http header，获得访问的url
	//	5.4.1 如果是黑名单url，则drop
	//	5.4.2 如果是redirect url，则redirect
	//	5.4.3 如果是auth的url，则accept
	if (IPPROTO_TCP == iph->protocol) {
		struct tcphdr *tcph = NULL;
		http_parser *http =  NULL;
		uint8_t *header = NULL;
		uint32_t header_len;

		if (!pskb_may_pull(skb, sizeof(struct tcphdr)))
			goto drop;
		iph = ip_hdr(skb);
		tcph = tcp_hdr(skb);

		if (tcph->syn || tcph->rst || tcph->fin)
			goto accept;

		http = ik_parse_http_header(skb, IK_HTTP_PARSE_RETRANS_REQUEST);
		header = ik_get_http_header(http, HTTP_Url, &header_len);
		if (ik_is_auth_url(header, header_len))
			goto accept;
		goto redirect;
	}

	// 6. 如果是icmp就返回NF_ACCEPT
	if (IPPROTO_ICMP == iph->protocol) {
		goto accept;
	}

redirect:
	redirect_auth_url(host, skb);
	set_tcp_rst(skb);
	goto accept;
drop:
	return NF_DROP;
accept:
	return NF_ACCEPT;
}
