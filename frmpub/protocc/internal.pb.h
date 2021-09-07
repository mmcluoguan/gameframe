// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: internal.proto

#ifndef GOOGLE_PROTOBUF_INCLUDED_internal_2eproto
#define GOOGLE_PROTOBUF_INCLUDED_internal_2eproto

#include <limits>
#include <string>

#include <google/protobuf/port_def.inc>
#if PROTOBUF_VERSION < 3015000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers. Please update
#error your headers.
#endif
#if 3015006 < PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers. Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/port_undef.inc>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_table_driven.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/metadata_lite.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
#include <google/protobuf/generated_enum_reflection.h>
// @@protoc_insertion_point(includes)
#include <google/protobuf/port_def.inc>
#define PROTOBUF_INTERNAL_EXPORT_internal_2eproto
PROTOBUF_NAMESPACE_OPEN
namespace internal {
class AnyMetadata;
}  // namespace internal
PROTOBUF_NAMESPACE_CLOSE

// Internal implementation detail -- do not use these members.
struct TableStruct_internal_2eproto {
  static const ::PROTOBUF_NAMESPACE_ID::internal::ParseTableField entries[]
    PROTOBUF_SECTION_VARIABLE(protodesc_cold);
  static const ::PROTOBUF_NAMESPACE_ID::internal::AuxiliaryParseTableField aux[]
    PROTOBUF_SECTION_VARIABLE(protodesc_cold);
  static const ::PROTOBUF_NAMESPACE_ID::internal::ParseTable schema[1]
    PROTOBUF_SECTION_VARIABLE(protodesc_cold);
  static const ::PROTOBUF_NAMESPACE_ID::internal::FieldMetadata field_metadata[];
  static const ::PROTOBUF_NAMESPACE_ID::internal::SerializationTable serialization_table[];
  static const ::PROTOBUF_NAMESPACE_ID::uint32 offsets[];
};
extern const ::PROTOBUF_NAMESPACE_ID::internal::DescriptorTable descriptor_table_internal_2eproto;
::PROTOBUF_NAMESPACE_ID::Metadata descriptor_table_internal_2eproto_metadata_getter(int index);
PROTOBUF_NAMESPACE_OPEN
PROTOBUF_NAMESPACE_CLOSE
namespace frmpub {
namespace protocc {

enum InternalMsgId : int {
  INTERNALMSGID_BEGIN = 0,
  REGISTER_WORLD_DBVISIT_C = 100,
  REGISTER_WORLD_DBVISIT_S = 101,
  SERONLINE_WORLD_GATE_G = 102,
  REGISTER_LOGIN_DBVISIT_C = 201,
  REGISTER_LOGIN_DBVISIT_S = 202,
  REGISTER_LOGIN_WORLD_C = 203,
  REGISTER_LOGIN_WORLD_S = 204,
  GAMESID_LOGIN_WORLD_C = 205,
  GAMESID_LOGIN_WORLD_S = 206,
  REGISTER_GAME_DBVISIT_C = 301,
  REGISTER_GAME_DBVISIT_S = 302,
  REGISTER_GAME_WORLD_C = 303,
  REGISTER_GAME_WORLD_S = 304,
  REGISTER_GATE_DBVISIT_C = 401,
  REGISTER_GATE_DBVISIT_S = 402,
  REGISTER_GATE_WORLD_C = 403,
  REGISTER_GATE_WORLD_S = 404,
  REGISTER_GATE_LOGIN_C = 405,
  REGISTER_GATE_LOGIN_S = 406,
  REGISTER_GATE_GAME_C = 407,
  REGISTER_GATE_GAME_S = 408,
  CLIOFFLINE_GATE_ALL_C = 409,
  LOADDATA_FROM_DBVISIT_C = 501,
  LOADDATA_FROM_DBVISIT_S = 502,
  LOADDATA_MORE_FROM_DBVISIT_C = 503,
  LOADDATA_MORE_FROM_DBVISIT_S = 504,
  INSERTDATA_TO_DBVISIT_C = 505,
  INSERTDATA_TO_DBVISIT_S = 506,
  UPDATA_TO_DBVISIT_C = 507,
  UPDATA_TO_DBVISIT_S = 508,
  DELETEDATA_TO_DBVISIT_C = 509,
  DELETEDATA_TO_DBVISIT_S = 510,
  INTERNALMSGID_END = 999,
  InternalMsgId_INT_MIN_SENTINEL_DO_NOT_USE_ = std::numeric_limits<::PROTOBUF_NAMESPACE_ID::int32>::min(),
  InternalMsgId_INT_MAX_SENTINEL_DO_NOT_USE_ = std::numeric_limits<::PROTOBUF_NAMESPACE_ID::int32>::max()
};
bool InternalMsgId_IsValid(int value);
constexpr InternalMsgId InternalMsgId_MIN = INTERNALMSGID_BEGIN;
constexpr InternalMsgId InternalMsgId_MAX = INTERNALMSGID_END;
constexpr int InternalMsgId_ARRAYSIZE = InternalMsgId_MAX + 1;

const ::PROTOBUF_NAMESPACE_ID::EnumDescriptor* InternalMsgId_descriptor();
template<typename T>
inline const std::string& InternalMsgId_Name(T enum_t_value) {
  static_assert(::std::is_same<T, InternalMsgId>::value ||
    ::std::is_integral<T>::value,
    "Incorrect type passed to function InternalMsgId_Name.");
  return ::PROTOBUF_NAMESPACE_ID::internal::NameOfEnum(
    InternalMsgId_descriptor(), enum_t_value);
}
inline bool InternalMsgId_Parse(
    ::PROTOBUF_NAMESPACE_ID::ConstStringParam name, InternalMsgId* value) {
  return ::PROTOBUF_NAMESPACE_ID::internal::ParseNamedEnum<InternalMsgId>(
    InternalMsgId_descriptor(), name, value);
}
// ===================================================================


// ===================================================================


// ===================================================================

#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif  // __GNUC__

// @@protoc_insertion_point(namespace_scope)

}  // namespace protocc
}  // namespace frmpub

PROTOBUF_NAMESPACE_OPEN

template <> struct is_proto_enum< ::frmpub::protocc::InternalMsgId> : ::std::true_type {};
template <>
inline const EnumDescriptor* GetEnumDescriptor< ::frmpub::protocc::InternalMsgId>() {
  return ::frmpub::protocc::InternalMsgId_descriptor();
}

PROTOBUF_NAMESPACE_CLOSE

// @@protoc_insertion_point(global_scope)

#include <google/protobuf/port_undef.inc>
#endif  // GOOGLE_PROTOBUF_INCLUDED_GOOGLE_PROTOBUF_INCLUDED_internal_2eproto
