/* Generated by the protocol buffer compiler.  DO NOT EDIT! */

/* Do not generate deprecated warnings for self */
#ifndef PROTOBUF_C_NO_DEPRECATED
#define PROTOBUF_C_NO_DEPRECATED
#endif

#include "urlwhitelist.pb-c.h"
void   urlwhitelist__init
                     (URLWhitelist         *message)
{
  static URLWhitelist init_value = URLWHITELIST__INIT;
  *message = init_value;
}
size_t urlwhitelist__get_packed_size
                     (const URLWhitelist *message)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &urlwhitelist__descriptor);
  return protobuf_c_message_get_packed_size ((const ProtobufCMessage*)(message));
}
size_t urlwhitelist__pack
                     (const URLWhitelist *message,
                      uint8_t       *out)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &urlwhitelist__descriptor);
  return protobuf_c_message_pack ((const ProtobufCMessage*)message, out);
}
size_t urlwhitelist__pack_to_buffer
                     (const URLWhitelist *message,
                      ProtobufCBuffer *buffer)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &urlwhitelist__descriptor);
  return protobuf_c_message_pack_to_buffer ((const ProtobufCMessage*)message, buffer);
}
URLWhitelist *
       urlwhitelist__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
  return (URLWhitelist *)
     protobuf_c_message_unpack (&urlwhitelist__descriptor,
                                allocator, len, data);
}
void   urlwhitelist__free_unpacked
                     (URLWhitelist *message,
                      ProtobufCAllocator *allocator)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &urlwhitelist__descriptor);
  protobuf_c_message_free_unpacked ((ProtobufCMessage*)message, allocator);
}
static const ProtobufCFieldDescriptor urlwhitelist__field_descriptors[1] =
{
  {
    "url",
    1,
    PROTOBUF_C_LABEL_REPEATED,
    PROTOBUF_C_TYPE_STRING,
    PROTOBUF_C_OFFSETOF(URLWhitelist, n_url),
    PROTOBUF_C_OFFSETOF(URLWhitelist, url),
    NULL,
    NULL,
    0,            /* packed */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
};
static const unsigned urlwhitelist__field_indices_by_name[] = {
  0,   /* field[0] = url */
};
static const ProtobufCIntRange urlwhitelist__number_ranges[1 + 1] =
{
  { 1, 0 },
  { 0, 1 }
};
const ProtobufCMessageDescriptor urlwhitelist__descriptor =
{
  PROTOBUF_C_MESSAGE_DESCRIPTOR_MAGIC,
  "URLWhitelist",
  "URLWhitelist",
  "URLWhitelist",
  "",
  sizeof(URLWhitelist),
  1,
  urlwhitelist__field_descriptors,
  urlwhitelist__field_indices_by_name,
  1,  urlwhitelist__number_ranges,
  (ProtobufCMessageInit) urlwhitelist__init,
  NULL,NULL,NULL    /* reserved[123] */
};
