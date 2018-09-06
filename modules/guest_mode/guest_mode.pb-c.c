/* Generated by the protocol buffer compiler.  DO NOT EDIT! */

/* Do not generate deprecated warnings for self */
#ifndef PROTOBUF_C_NO_DEPRECATED
#define PROTOBUF_C_NO_DEPRECATED
#endif

#include "guest_mode.pb-c.h"
void   subnet__init
                     (Subnet         *message)
{
  static Subnet init_value = SUBNET__INIT;
  *message = init_value;
}
size_t subnet__get_packed_size
                     (const Subnet *message)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &subnet__descriptor);
  return protobuf_c_message_get_packed_size ((const ProtobufCMessage*)(message));
}
size_t subnet__pack
                     (const Subnet *message,
                      uint8_t       *out)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &subnet__descriptor);
  return protobuf_c_message_pack ((const ProtobufCMessage*)message, out);
}
size_t subnet__pack_to_buffer
                     (const Subnet *message,
                      ProtobufCBuffer *buffer)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &subnet__descriptor);
  return protobuf_c_message_pack_to_buffer ((const ProtobufCMessage*)message, buffer);
}
Subnet *
       subnet__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
  return (Subnet *)
     protobuf_c_message_unpack (&subnet__descriptor,
                                allocator, len, data);
}
void   subnet__free_unpacked
                     (Subnet *message,
                      ProtobufCAllocator *allocator)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &subnet__descriptor);
  protobuf_c_message_free_unpacked ((ProtobufCMessage*)message, allocator);
}
void   guest_mode_rules__init
                     (GuestModeRules         *message)
{
  static GuestModeRules init_value = GUEST_MODE_RULES__INIT;
  *message = init_value;
}
size_t guest_mode_rules__get_packed_size
                     (const GuestModeRules *message)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &guest_mode_rules__descriptor);
  return protobuf_c_message_get_packed_size ((const ProtobufCMessage*)(message));
}
size_t guest_mode_rules__pack
                     (const GuestModeRules *message,
                      uint8_t       *out)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &guest_mode_rules__descriptor);
  return protobuf_c_message_pack ((const ProtobufCMessage*)message, out);
}
size_t guest_mode_rules__pack_to_buffer
                     (const GuestModeRules *message,
                      ProtobufCBuffer *buffer)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &guest_mode_rules__descriptor);
  return protobuf_c_message_pack_to_buffer ((const ProtobufCMessage*)message, buffer);
}
GuestModeRules *
       guest_mode_rules__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
  return (GuestModeRules *)
     protobuf_c_message_unpack (&guest_mode_rules__descriptor,
                                allocator, len, data);
}
void   guest_mode_rules__free_unpacked
                     (GuestModeRules *message,
                      ProtobufCAllocator *allocator)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &guest_mode_rules__descriptor);
  protobuf_c_message_free_unpacked ((ProtobufCMessage*)message, allocator);
}
static const ProtobufCFieldDescriptor subnet__field_descriptors[2] =
{
  {
    "ip",
    1,
    PROTOBUF_C_LABEL_REQUIRED,
    PROTOBUF_C_TYPE_UINT32,
    0,   /* quantifier_offset */
    PROTOBUF_C_OFFSETOF(Subnet, ip),
    NULL,
    NULL,
    0,            /* packed */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "mask",
    2,
    PROTOBUF_C_LABEL_REQUIRED,
    PROTOBUF_C_TYPE_UINT32,
    0,   /* quantifier_offset */
    PROTOBUF_C_OFFSETOF(Subnet, mask),
    NULL,
    NULL,
    0,            /* packed */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
};
static const unsigned subnet__field_indices_by_name[] = {
  0,   /* field[0] = ip */
  1,   /* field[1] = mask */
};
static const ProtobufCIntRange subnet__number_ranges[1 + 1] =
{
  { 1, 0 },
  { 0, 2 }
};
const ProtobufCMessageDescriptor subnet__descriptor =
{
  PROTOBUF_C_MESSAGE_DESCRIPTOR_MAGIC,
  "Subnet",
  "Subnet",
  "Subnet",
  "",
  sizeof(Subnet),
  2,
  subnet__field_descriptors,
  subnet__field_indices_by_name,
  1,  subnet__number_ranges,
  (ProtobufCMessageInit) subnet__init,
  NULL,NULL,NULL    /* reserved[123] */
};
static const ProtobufCFieldDescriptor guest_mode_rules__field_descriptors[3] =
{
  {
    "ifnames",
    1,
    PROTOBUF_C_LABEL_REPEATED,
    PROTOBUF_C_TYPE_STRING,
    PROTOBUF_C_OFFSETOF(GuestModeRules, n_ifnames),
    PROTOBUF_C_OFFSETOF(GuestModeRules, ifnames),
    NULL,
    NULL,
    0,            /* packed */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "ports",
    2,
    PROTOBUF_C_LABEL_REPEATED,
    PROTOBUF_C_TYPE_UINT32,
    PROTOBUF_C_OFFSETOF(GuestModeRules, n_ports),
    PROTOBUF_C_OFFSETOF(GuestModeRules, ports),
    NULL,
    NULL,
    0,            /* packed */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "snets",
    3,
    PROTOBUF_C_LABEL_REPEATED,
    PROTOBUF_C_TYPE_MESSAGE,
    PROTOBUF_C_OFFSETOF(GuestModeRules, n_snets),
    PROTOBUF_C_OFFSETOF(GuestModeRules, snets),
    &subnet__descriptor,
    NULL,
    0,            /* packed */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
};
static const unsigned guest_mode_rules__field_indices_by_name[] = {
  0,   /* field[0] = ifnames */
  1,   /* field[1] = ports */
  2,   /* field[2] = snets */
};
static const ProtobufCIntRange guest_mode_rules__number_ranges[1 + 1] =
{
  { 1, 0 },
  { 0, 3 }
};
const ProtobufCMessageDescriptor guest_mode_rules__descriptor =
{
  PROTOBUF_C_MESSAGE_DESCRIPTOR_MAGIC,
  "GuestModeRules",
  "GuestModeRules",
  "GuestModeRules",
  "",
  sizeof(GuestModeRules),
  3,
  guest_mode_rules__field_descriptors,
  guest_mode_rules__field_indices_by_name,
  1,  guest_mode_rules__number_ranges,
  (ProtobufCMessageInit) guest_mode_rules__init,
  NULL,NULL,NULL    /* reserved[123] */
};