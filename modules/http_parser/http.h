#ifndef __HTTP_ID_H_
#define __HTTP_ID_H_

#include "http_parser.h"
#include "http_hash.h"

enum state
  { s_dead = 1 /* important that this is > 0 */

  , s_start_req_or_res
  , s_res_or_resp_H
  , s_start_res
  , s_res_H //5
  , s_res_HT
  , s_res_HTT
  , s_res_HTTP
  , s_res_first_http_major
  , s_res_http_major // 10
  , s_res_first_http_minor
  , s_res_http_minor
  , s_res_first_status_code
  , s_res_status_code
  , s_res_status_start // 15
  , s_res_status
  , s_res_line_almost_done

  , s_start_req

  , s_req_method
  , s_req_spaces_before_url // 20
  , s_req_schema
  , s_req_schema_slash
  , s_req_schema_slash_slash
  , s_req_server_start
  , s_req_server // 25
  , s_req_server_with_at
  , s_req_path
  , s_req_query_string_start
  , s_req_query_string
  , s_req_fragment_start // 30
  , s_req_fragment
  , s_req_http_start
  , s_req_http_H
  , s_req_http_HT
  , s_req_http_HTT // 35
  , s_req_http_HTTP
  , s_req_first_http_major
  , s_req_http_major
  , s_req_first_http_minor
  , s_req_http_minor // 40
  , s_req_line_almost_done

  , s_header_field_start
  , s_header_field
  , s_header_value_discard_ws
  , s_header_value_discard_ws_almost_done // 45
  , s_header_value_discard_lws
  , s_header_value_start
  , s_header_value
  , s_header_value_lws

  , s_header_almost_done // 50

  , s_chunk_size_start
  , s_chunk_size
  , s_chunk_parameters
  , s_chunk_size_almost_done

  , s_headers_almost_done // 55
  , s_headers_done

  /* Important: 's_headers_done' must be the last 'header' state. All
   * states beyond this must be 'body' states. It is used for overflow
   * checking. See the PARSING_HEADER() macro.
   */

  , s_chunk_data
  , s_chunk_data_almost_done
  , s_chunk_data_done

  , s_body_identity // 60
  , s_body_identity_eof

  , s_message_done // 62
  };


#define MAX_HTTP_HEADER_COUNT	(16)
#define MAX_HTTP_URL_EXT_LEN 	(5)

struct http_section {
  char * name;
  unsigned char index;
};

typedef struct http_header_data {
	const char *value;
	unsigned int value_len;
} http_header_data;

enum {
	HTTP_CONTENT_ENCODING_TYPE_NONE = 0,
	HTTP_CONTENT_ENCODING_TYPE_GZIP = 1,
	HTTP_CONTENT_ENCODING_TYPE_DEFALTE = 2,
	HTTP_CONTENT_ENCODING_TYPE_CHUNKED = 3,
};

typedef struct http_parser_data {
	http_header_data header[HTTP_MAX];
	unsigned char header_seq[MAX_HTTP_HEADER_COUNT];
	unsigned char ext[MAX_HTTP_URL_EXT_LEN];
	unsigned char cur_header_index;
	unsigned char 
		is_url_ext_parsed:1,
		content_encoding_type:2,
		is_body_seen:1,
		is_body_unzip:1,
		:3;	

	unsigned long header_bitmap[2];		
} http_parser_data;

//tmp global functions
extern int request_url_cb(http_parser *p, const char *buf, size_t len);
extern int request_url_cb_stop(http_parser *p, const char *buf, size_t len);
extern int header_field_cb(http_parser *p, const char *buf, size_t len);
extern int header_value_cb(http_parser *p, const char *buf, size_t len);
extern int headers_complete_cb(http_parser *p);
extern int body_cb(http_parser *p, const char *buf, size_t len);
extern void get_url_ext(http_parser_data *hpd);
extern void get_real_body(http_parser_data *hpd);
#endif
