#include <linux/version.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <net/tcp.h>
#include <net/ip.h>
#include <net/route.h>
#include <net/dst.h>

#include "http.h"
#include "http_hash.h"

#if 0
#define HTTP_REAL_BODY_BUF_SIZE (256)
static DEFINE_PER_CPU(char, g_real_body[HTTP_REAL_BODY_BUF_SIZE]);

// from http_compress.c
extern int gunzip(const char *dst, int dstlen, const char *src, int srclen);
#endif
// from http.c
extern u8 http_lookup (register const char *str, register unsigned int len);

const char * const HTTP_header_name[] = {
	"URL",
	"Host",
	"User-Agent",
	"Referer",
	"Seq",
	"Pragma",
	"Accept",
	"Cookie",
	"Keep-Alive",
	"Cache-Control",
	"Content-Encoding",
	"Content-Language",
	"Content-Type",
	"Range",
	"Content-Length",
	"Connection",
	"Date",
	"Transfer-Encoding",
	"Upgrade",
	"Via",
	"Accept-Charset",
	"Accept-Encoding",
	"Accept-Language",
	"Authorization",
	"From",
	"If-Modified-Since",
	"If-Match",
	"If-None-Match",
	"If-Range",
	"If-Unmodified-Since",
	"Max-Forwards",
	"Proxy-Authorization",
	"Age",
	"Location",
	"Proxy-Authenticate",
	"Public",
	"Retry-After",
	"Server",
	"Vary",
	"Warning",
	"WWW-Authenticate",
	"Allow",
	"Content-Base",
	"Content-Location",
	"Content-MD5",
	"Content-Range",
	"Etag",
	"Expires",
	"Last-Modified",
	"Version",
	"Status",
	"Priv",
	"Ext",
	"Body",
	"Priv2",
	"Priv3",
	"X-Request-With",
	"Set-Coookie"
};



void get_url_ext(http_parser_data *hpd) 
{
	http_header_data *url;
	const char *buf, *end, *last = NULL;
	int i;

	hpd->is_url_ext_parsed = 1;
	url = &hpd->header[HTTP_Url];
	
	if (0 == url->value_len) {
		return;
	}

	buf = url->value;
	end = url->value + url->value_len;
	while(buf < end) {
		if (*buf == '?')
			break;
		else if (*buf == '.')
			last = buf;
		else if (*buf == '/')
			last = NULL;
		buf++;
	}

	if (!last) {
		return;
	}
	
	// file..ext
	while (*last == '.')
		last++;

	if (buf != end)
		end = buf;

	if (last >= end) {
		return;
	}

	if (end-last > 	MAX_HTTP_URL_EXT_LEN) {
		hpd->header[HTTP_Ext].value_len = MAX_HTTP_URL_EXT_LEN;
	} else {
		hpd->header[HTTP_Ext].value_len = end-last;
	}

#define LOWER(c)            (unsigned char)(c | 0x20)
	for (i = 0; i < hpd->header[HTTP_Ext].value_len; i++) {
		hpd->ext[i] = LOWER(*last);
		++last;
	}

	__set_bit(HTTP_Ext, hpd->header_bitmap);

	return;
}

int request_url_cb(http_parser *p, const char *buf, size_t len)
{
	http_parser_data *hpd;
	http_header_data *hhd;

	hpd = (http_parser_data *)p->data;

	hhd = &(hpd->header[HTTP_Url]);
	
	hhd->value = buf;
	hhd->value_len = len;

	hpd->header_seq[0] = HTTP_Url;
	++hpd->header[HTTP_Seq].value_len;

	hpd->cur_header_index = HTTP_Url;
	__set_bit(HTTP_Url, hpd->header_bitmap);
	
	return 0;
}

int request_url_cb_stop(http_parser *p, const char *buf, size_t len)
{
	http_parser_data *hpd;
	http_header_data *hhd;

	hpd = (http_parser_data *)p->data;

	hhd = &(hpd->header[HTTP_Url]);
	
	hhd->value = buf;
	hhd->value_len = len;

	hpd->header_seq[0] = HTTP_Url;
	++hpd->header[HTTP_Seq].value_len;

	hpd->cur_header_index = HTTP_Url;
	__set_bit(HTTP_Url, hpd->header_bitmap);
	
	return 1;
}


int header_field_cb(http_parser *p, const char *buf, size_t len)
{
	http_parser_data *hpd;
	u8 index;

	hpd = (http_parser_data *)p->data;

	index = http_lookup(buf, len);

	if (HTTP_Priv == index) {
		// Check if the Priv is found
		if (test_bit(HTTP_Priv, hpd->header_bitmap)) {
			// Priv is used, try Priv2
			if (test_bit(HTTP_Priv2, hpd->header_bitmap)) {
				// Oh, Priv2 is used too, try Priv3
				if (test_bit(HTTP_Priv3, hpd->header_bitmap)) {
					/*
					Too many the headers we don't know.....
					return directly.
					*/
					hpd->cur_header_index = HTTP_MAX;
					return 0;
				}
				else {
					// We get the last chance
					index = HTTP_Priv3;
				}
			}
			else {
				// Ok, we use the priv2
				index = HTTP_Priv2;
			}
		} 
	}

	if (likely(hpd->header[HTTP_Seq].value_len < MAX_HTTP_HEADER_COUNT)) {
		hpd->header_seq[hpd->header[HTTP_Seq].value_len] = index;
		++hpd->header[HTTP_Seq].value_len;
	}

	hpd->cur_header_index = index;

	__set_bit(index, hpd->header_bitmap);
	
	return 0;
}

int header_value_cb(http_parser *p, const char *buf, size_t len)
{
	http_parser_data *hpd;
	http_header_data *hhd;
	u8 header_index;
	
	BUG_ON(!p->data);
	hpd = (http_parser_data *)p->data;
	header_index = hpd->cur_header_index;
	if (header_index >= HTTP_MAX) {
		return 0;
	}
	
	hhd = &(hpd->header[header_index]);
	
	hhd->value = buf;
	hhd->value_len = len;

#ifdef IK_DEVELOP
	if (unlikely(len == -1)) {
		IK_EMERG_LOG("IK_EMERG_LOG: The http header_index(%d) length is invalid value(%d)\n",
			header_index, (u32)len);
	}
#endif	
	
	if (header_index == HTTP_Content_Encoding) {
		if (!memcmp((void *)hhd->value, "gzip", 4)) {
			hpd->content_encoding_type = HTTP_CONTENT_ENCODING_TYPE_GZIP;
		}
		else if (!memcmp((void *)hhd->value, "deflate", 7)) {
			hpd->content_encoding_type = HTTP_CONTENT_ENCODING_TYPE_DEFALTE;
		}
		else if (!memcmp((void *)hhd->value, "chunked", 7)) {
			hpd->content_encoding_type = HTTP_CONTENT_ENCODING_TYPE_CHUNKED;
		}
	}

	return 0;
}

#if 0
static int unzip_http_body(const char *buf, size_t len, const char **rbody)
{
	// magic
	if (len <= 2 || buf[0] != '\x1f' || buf[1] != '\x8b')
		return 0;

	*rbody = (char *)__get_cpu_var(g_real_body);
	return gunzip(*rbody, HTTP_REAL_BODY_BUF_SIZE, buf, len);
}

void get_real_body(http_parser_data *hpd)
{
	http_header_data *hhd;
	const char *orig_body;

	hhd = &hpd->header[HTTP_Body];
	
	orig_body = hhd->value;
	hhd->value_len = unzip_http_body(orig_body, hhd->value_len, &hhd->value);
}
#endif


int body_cb(http_parser *p, const char *buf, size_t len)
{
	http_parser_data *hpd;
	http_header_data *hhd;
	
	hpd = (http_parser_data *)p->data;

	// only check first chunked data
	if (hpd->is_body_seen) {
		return 0;
	}
	
	hpd->is_body_seen = 1;
	hhd = &(hpd->header[HTTP_Body]);

	hhd->value = buf;
	hhd->value_len = len;

	if (likely(hpd->header[HTTP_Seq].value_len < MAX_HTTP_HEADER_COUNT)) {
		hpd->header_seq[hpd->header[HTTP_Seq].value_len] = HTTP_Body;
		++hpd->header[HTTP_Seq].value_len;
	}

	__set_bit(HTTP_Body, hpd->header_bitmap);
	return 0;
}


int headers_complete_cb(http_parser *p)
{
	BUILD_BUG_ON(ARRAY_SIZE(HTTP_header_name) != HTTP_MAX);

	if (p->method == HTTP_HEAD && p->type == HTTP_RESPONSE)
		return 1; // skip body

	return 0;
}


