/* C code produced by gperf version 3.0.4 */
/* Command-line: gperf -D -t --ignore-case -K name -H http_hash -N http_lookup http.gperf  */
/* Computed positions: -k'9,$' */

#include <linux/types.h>
#include <linux/bug.h>
#include <linux/string.h>

#include "http_hash.h"
#include "http.h"

#ifdef TEST_HASH
#include <stdlib.h>
#include <string.h>

struct http_section {
  char * name;
  unsigned char index;
};

#define u8 unsigned char

#endif

#if !((' ' == 32) && ('!' == 33) && ('"' == 34) && ('#' == 35) \
      && ('%' == 37) && ('&' == 38) && ('\'' == 39) && ('(' == 40) \
      && (')' == 41) && ('*' == 42) && ('+' == 43) && (',' == 44) \
      && ('-' == 45) && ('.' == 46) && ('/' == 47) && ('0' == 48) \
      && ('1' == 49) && ('2' == 50) && ('3' == 51) && ('4' == 52) \
      && ('5' == 53) && ('6' == 54) && ('7' == 55) && ('8' == 56) \
      && ('9' == 57) && (':' == 58) && (';' == 59) && ('<' == 60) \
      && ('=' == 61) && ('>' == 62) && ('?' == 63) && ('A' == 65) \
      && ('B' == 66) && ('C' == 67) && ('D' == 68) && ('E' == 69) \
      && ('F' == 70) && ('G' == 71) && ('H' == 72) && ('I' == 73) \
      && ('J' == 74) && ('K' == 75) && ('L' == 76) && ('M' == 77) \
      && ('N' == 78) && ('O' == 79) && ('P' == 80) && ('Q' == 81) \
      && ('R' == 82) && ('S' == 83) && ('T' == 84) && ('U' == 85) \
      && ('V' == 86) && ('W' == 87) && ('X' == 88) && ('Y' == 89) \
      && ('Z' == 90) && ('[' == 91) && ('\\' == 92) && (']' == 93) \
      && ('^' == 94) && ('_' == 95) && ('a' == 97) && ('b' == 98) \
      && ('c' == 99) && ('d' == 100) && ('e' == 101) && ('f' == 102) \
      && ('g' == 103) && ('h' == 104) && ('i' == 105) && ('j' == 106) \
      && ('k' == 107) && ('l' == 108) && ('m' == 109) && ('n' == 110) \
      && ('o' == 111) && ('p' == 112) && ('q' == 113) && ('r' == 114) \
      && ('s' == 115) && ('t' == 116) && ('u' == 117) && ('v' == 118) \
      && ('w' == 119) && ('x' == 120) && ('y' == 121) && ('z' == 122) \
      && ('{' == 123) && ('|' == 124) && ('}' == 125) && ('~' == 126))
/* The character set is not based on ISO-646.  */
error "gperf generated tables don't work with this execution character set. Please report a bug to <bug-gnu-gperf@gnu.org>."
#endif

#ifndef __KERNEL__
#include <stdio.h>
#include <string.h>
#include <assert.h>
#endif

#include "http_hash.h"

#define TOTAL_KEYWORDS 51
#define MIN_WORD_LENGTH 3
#define MAX_WORD_LENGTH 19
#define MIN_HASH_VALUE 4
#define MAX_HASH_VALUE 82
/* maximum key range = 79, duplicates = 0 */

static const struct http_section wordlist[] =
{
	{"Host",  HTTP_Host},
	{"Allow",HTTP_Allow},
	{"Accept", HTTP_Accept},
	{"Age",HTTP_Age},
	{"Date",HTTP_Date},
	{"Range", HTTP_Range},
	{"Cookie", HTTP_Cookie},
	{"Upgrade",HTTP_Upgrade},
	{"If-Range",HTTP_If_Range},
	{"Vary",HTTP_Vary},
	{"Set-Cookie", HTTP_Set_Cookie},
	{"Public",HTTP_Public},
	{"Content-Type",HTTP_Content_Type},
	{"Via",HTTP_Via},
	{"From",HTTP_From},
	{"Keep-Alive", HTTP_Keep_Alive},
	{"Pragma", HTTP_Pragma},
	{"If-Modified-Since",HTTP_If_Modified_Since},
	{"Proxy-Authenticate",HTTP_Proxy_Authenticate},
	{"If-Unmodified-Since",HTTP_If_Unmodified_Since},
	{"WWW-Authenticate",HTTP_WWW_Authenticate},
	{"Version", HTTP_Version},
	{"Location",HTTP_Location},
	{"User-Agent", HTTP_User_Agent},
	{"Server",HTTP_Server},
	{"Referer", HTTP_Referer},
	{"Last-Modified",HTTP_Last_Modified},
	{"Etag",HTTP_Etag},
	{"Accept-Language",HTTP_Accept_Language},
	{"Retry-After",HTTP_Retry_After},
	{"Warning",HTTP_Warning},
	{"If-Match",HTTP_If_Match},
	{"Proxy-Authorization",HTTP_Proxy_Authorization},
	{"Connection", HTTP_Connection},
	{"Content-Language", HTTP_Content_Language},
	{"Content-Base",HTTP_Content_Base},
	{"Content-Range",HTTP_Content_Range},
	{"Accept-Charset",HTTP_Accept_Charset},
	{"X-Requested-With", HTTP_X_Request_With},
	{"Transfer-Encoding",HTTP_Transfer_Encoding},
	{"Authorization",HTTP_Authorization},
	{"Content-Encoding", HTTP_Content_Encoding},
	{"Cache-Control", HTTP_Cache_Control},
	{"Content-Location",HTTP_Content_Location},
	{"If-None-Match",HTTP_If_None_Match},
	{"Status", HTTP_Status},
	{"Expires",HTTP_Expires},
	{"Content-Length", HTTP_Content_Length},
	{"Accept-Encoding",HTTP_Accept_Encoding},
	{"Content-MD5",HTTP_Content_MD5},
	{"Max-Forwards",HTTP_Max_Forwards}
};

#ifdef TEST_HASH
static struct http_section wordlist2[] =
{
	{"host",  HTTP_Host},
	{"allow",HTTP_Allow},
	{"accept", HTTP_Accept},
	{"age",HTTP_Age},
	{"date",HTTP_Date},
	{"range", HTTP_Range},
	{"cookie", HTTP_Cookie},
	{"upgrade",HTTP_Upgrade},
	{"if-Range",HTTP_If_Range},
	{"vary",HTTP_Vary},
	{"set-Cookie", HTTP_Set_Cookie},
	{"public",HTTP_Public},
	{"content-Type",HTTP_Content_Type},
	{"via",HTTP_Via},
	{"from",HTTP_From},
	{"keep-Alive", HTTP_Keep_Alive},
	{"pragma", HTTP_Pragma},
	{"if-modified-Since",HTTP_If_Modified_Since},
	{"proxy-Authenticate",HTTP_Proxy_Authenticate},
	{"if-Unmodified-Since",HTTP_If_Unmodified_Since},
	{"www-Authenticate",HTTP_WWW_Authenticate},
	{"version", HTTP_Version},
	{"location",HTTP_Location},
	{"user-Agent", HTTP_User_Agent},
	{"server",HTTP_Server},
	{"referer", HTTP_Referer},
	{"last-Modified",HTTP_Last_Modified},
	{"etag",HTTP_Etag},
	{"accept-Language",HTTP_Accept_Language},
	{"retry-After",HTTP_Retry_After},
	{"warning",HTTP_Warning},
	{"if-Match",HTTP_If_Match},
	{"proxy-Authorization",HTTP_Proxy_Authorization},
	{"connection", HTTP_Connection},
	{"content-Language", HTTP_Content_Language},
	{"content-Base",HTTP_Content_Base},
	{"content-Range",HTTP_Content_Range},
	{"accept-Charset",HTTP_Accept_Charset},
	{"x-Requested-With", HTTP_X_Request_With},
	{"transfer-Encoding",HTTP_Transfer_Encoding},
	{"authorization",HTTP_Authorization},
	{"content-Encoding", HTTP_Content_Encoding},
	{"cache-Control", HTTP_Cache_Control},
	{"content-Location",HTTP_Content_Location},
	{"if-None-Match",HTTP_If_None_Match},
	{"status", HTTP_Status},
	{"expires",HTTP_Expires},
	{"content-Length", HTTP_Content_Length},
	{"accept-Encoding",HTTP_Accept_Encoding},
	{"content-MD5",HTTP_Content_MD5},
	{"max-Forwards",HTTP_Max_Forwards}
};
#endif    

static unsigned int
http_hash (register const char *str, register unsigned int len)
{
  static unsigned char asso_values[] =
    {
      83, 83, 83, 83, 83, 83, 83, 83, 83, 83,
      83, 83, 83, 83, 83, 83, 83, 83, 83, 83,
      83, 83, 83, 83, 83, 83, 83, 83, 83, 83,
      83, 83, 83, 83, 83, 83, 83, 83, 83, 83,
      83, 83, 83, 83, 83,  0, 83, 83, 83, 83,
      83, 83, 83, 40, 83, 83, 83, 83, 83, 83,
      83, 83, 83, 83, 83, 15, 25, 10, 20,  5,
      83, 30, 30,  0, 83, 83, 20, 15, 20, 10,
      83, 83, 25, 55,  0, 83,  5,  0, 83, 10,
      83, 83, 83, 83, 83, 83, 83, 15, 25, 10,
      20,  5, 83, 30, 30,  0, 83, 83, 20, 15,
      20, 10, 83, 83, 25, 55,  0, 83,  5,  0,
      83, 10, 83, 83, 83, 83, 83, 83, 83, 83,
      83, 83, 83, 83, 83, 83, 83, 83, 83, 83,
      83, 83, 83, 83, 83, 83, 83, 83, 83, 83,
      83, 83, 83, 83, 83, 83, 83, 83, 83, 83,
      83, 83, 83, 83, 83, 83, 83, 83, 83, 83,
      83, 83, 83, 83, 83, 83, 83, 83, 83, 83,
      83, 83, 83, 83, 83, 83, 83, 83, 83, 83,
      83, 83, 83, 83, 83, 83, 83, 83, 83, 83,
      83, 83, 83, 83, 83, 83, 83, 83, 83, 83,
      83, 83, 83, 83, 83, 83, 83, 83, 83, 83,
      83, 83, 83, 83, 83, 83, 83, 83, 83, 83,
      83, 83, 83, 83, 83, 83, 83, 83, 83, 83,
      83, 83, 83, 83, 83, 83, 83, 83, 83, 83,
      83, 83, 83, 83, 83, 83
    };
  register int hval = len;

  switch (hval)
    {
      default:
        hval += asso_values[(unsigned char)str[8]];
      /*FALLTHROUGH*/
      case 8:
      case 7:
      case 6:
      case 5:
      case 4:
      case 3:
        break;
    }
  return hval + asso_values[(unsigned char)str[len - 1]];
}

u8 http_lookup (register const char *str, register unsigned int len)
{

  static const short lookup[] =
    {
      -1, -1, -1, -1,  0,  1,  2, -1,  3,  4,  5,  6,  7,  8,
       9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, -1, 20, 21,
      22, -1, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34,
      35, 36, 37, -1, 38, 39, 40, -1, -1, 41, -1, 42, -1, -1,
      43, -1, 44, -1, -1, 45, 46, -1, 47, 48, 49, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 50
    };

  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register int key = http_hash (str, len);

      if (key <= MAX_HASH_VALUE && key >= 0)
        {
          register int index = lookup[key];

          if (index >= 0)
            {
              register const char *s = wordlist[index].name;

              if ((((unsigned char)*str ^ (unsigned char)*s) & ~32) == 0 && !strncasecmp (str, s, len))
                return wordlist[index].index;
            }
        }
    }

    BUILD_BUG_ON(sizeof(wordlist)/sizeof(wordlist[0]) != HTTP_MAX-7);

    return HTTP_Priv;
}


#ifdef TEST_HASH
int main()
{
    const struct http_section *test_str;
    const struct http_section *result;
    int i;
    int err = 0;
    
    for (i = 0; i < sizeof(wordlist)/sizeof(wordlist[0]); ++i) {
        test_str = wordlist+i;
        result = http_lookup(test_str->name, strlen((test_str->name)));
        if (result->index != test_str->index) {
            printf("The %s failed\n", test_str->name);
            ++err;
        }
    }

    for (i = 0; i < sizeof(wordlist2)/sizeof(wordlist2[0]); ++i) {
        test_str = wordlist2+i;
        result = http_lookup(test_str->name, strlen((test_str->name)));
        if (!result || result->index != test_str->index) {
            printf("The %s failed\n", test_str->name);
            ++err;
        }
    }

    if (!err) {
    	printf("Test oK\n");
    }
    
    return 0;
}

#endif
