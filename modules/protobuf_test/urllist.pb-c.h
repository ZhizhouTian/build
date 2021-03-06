/* Generated by the protocol buffer compiler.  DO NOT EDIT! */

#ifndef PROTOBUF_C_urllist_2eproto__INCLUDED
#define PROTOBUF_C_urllist_2eproto__INCLUDED

#include <google/protobuf-c/protobuf-c.h>

PROTOBUF_C_BEGIN_DECLS


typedef struct _URLList URLList;


/* --- enums --- */


/* --- messages --- */

struct  _URLList
{
  ProtobufCMessage base;
  size_t n_url;
  char **url;
};
#define URLLIST__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&urllist__descriptor) \
    , 0,NULL }


/* URLList methods */
void   urllist__init
                     (URLList         *message);
size_t urllist__get_packed_size
                     (const URLList   *message);
size_t urllist__pack
                     (const URLList   *message,
                      uint8_t             *out);
size_t urllist__pack_to_buffer
                     (const URLList   *message,
                      ProtobufCBuffer     *buffer);
URLList *
       urllist__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   urllist__free_unpacked
                     (URLList *message,
                      ProtobufCAllocator *allocator);
/* --- per-message closures --- */

typedef void (*URLList_Closure)
                 (const URLList *message,
                  void *closure_data);

/* --- services --- */


/* --- descriptors --- */

extern const ProtobufCMessageDescriptor urllist__descriptor;

PROTOBUF_C_END_DECLS


#endif  /* PROTOBUF_urllist_2eproto__INCLUDED */
