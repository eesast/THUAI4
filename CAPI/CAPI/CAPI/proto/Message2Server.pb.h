// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: Message2Server.proto

#ifndef GOOGLE_PROTOBUF_INCLUDED_Message2Server_2eproto
#define GOOGLE_PROTOBUF_INCLUDED_Message2Server_2eproto

#include <limits>
#include <string>

#include <google/protobuf/port_def.inc>
#if PROTOBUF_VERSION < 3014000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers. Please update
#error your headers.
#endif
#if 3014000 < PROTOBUF_MIN_PROTOC_VERSION
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
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
#include <google/protobuf/unknown_field_set.h>
#include "MessageType.pb.h"
#include "Message2Client.pb.h"
// @@protoc_insertion_point(includes)
#include <google/protobuf/port_def.inc>
#define PROTOBUF_INTERNAL_EXPORT_Message2Server_2eproto
PROTOBUF_NAMESPACE_OPEN
namespace internal {
class AnyMetadata;
}  // namespace internal
PROTOBUF_NAMESPACE_CLOSE

// Internal implementation detail -- do not use these members.
struct TableStruct_Message2Server_2eproto {
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
extern const ::PROTOBUF_NAMESPACE_ID::internal::DescriptorTable descriptor_table_Message2Server_2eproto;
namespace Protobuf {
class MessageToServer;
class MessageToServerDefaultTypeInternal;
extern MessageToServerDefaultTypeInternal _MessageToServer_default_instance_;
}  // namespace Protobuf
PROTOBUF_NAMESPACE_OPEN
template<> ::Protobuf::MessageToServer* Arena::CreateMaybeMessage<::Protobuf::MessageToServer>(Arena*);
PROTOBUF_NAMESPACE_CLOSE
namespace Protobuf {

// ===================================================================

class MessageToServer PROTOBUF_FINAL :
    public ::PROTOBUF_NAMESPACE_ID::Message /* @@protoc_insertion_point(class_definition:Protobuf.MessageToServer) */ {
 public:
  inline MessageToServer() : MessageToServer(nullptr) {}
  virtual ~MessageToServer();

  MessageToServer(const MessageToServer& from);
  MessageToServer(MessageToServer&& from) noexcept
    : MessageToServer() {
    *this = ::std::move(from);
  }

  inline MessageToServer& operator=(const MessageToServer& from) {
    CopyFrom(from);
    return *this;
  }
  inline MessageToServer& operator=(MessageToServer&& from) noexcept {
    if (GetArena() == from.GetArena()) {
      if (this != &from) InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }

  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* descriptor() {
    return GetDescriptor();
  }
  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* GetDescriptor() {
    return GetMetadataStatic().descriptor;
  }
  static const ::PROTOBUF_NAMESPACE_ID::Reflection* GetReflection() {
    return GetMetadataStatic().reflection;
  }
  static const MessageToServer& default_instance();

  static inline const MessageToServer* internal_default_instance() {
    return reinterpret_cast<const MessageToServer*>(
               &_MessageToServer_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    0;

  friend void swap(MessageToServer& a, MessageToServer& b) {
    a.Swap(&b);
  }
  inline void Swap(MessageToServer* other) {
    if (other == this) return;
    if (GetArena() == other->GetArena()) {
      InternalSwap(other);
    } else {
      ::PROTOBUF_NAMESPACE_ID::internal::GenericSwap(this, other);
    }
  }
  void UnsafeArenaSwap(MessageToServer* other) {
    if (other == this) return;
    GOOGLE_DCHECK(GetArena() == other->GetArena());
    InternalSwap(other);
  }

  // implements Message ----------------------------------------------

  inline MessageToServer* New() const final {
    return CreateMaybeMessage<MessageToServer>(nullptr);
  }

  MessageToServer* New(::PROTOBUF_NAMESPACE_ID::Arena* arena) const final {
    return CreateMaybeMessage<MessageToServer>(arena);
  }
  void CopyFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) final;
  void MergeFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) final;
  void CopyFrom(const MessageToServer& from);
  void MergeFrom(const MessageToServer& from);
  PROTOBUF_ATTRIBUTE_REINITIALIZES void Clear() final;
  bool IsInitialized() const final;

  size_t ByteSizeLong() const final;
  const char* _InternalParse(const char* ptr, ::PROTOBUF_NAMESPACE_ID::internal::ParseContext* ctx) final;
  ::PROTOBUF_NAMESPACE_ID::uint8* _InternalSerialize(
      ::PROTOBUF_NAMESPACE_ID::uint8* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const final;
  int GetCachedSize() const final { return _cached_size_.Get(); }

  private:
  inline void SharedCtor();
  inline void SharedDtor();
  void SetCachedSize(int size) const final;
  void InternalSwap(MessageToServer* other);
  friend class ::PROTOBUF_NAMESPACE_ID::internal::AnyMetadata;
  static ::PROTOBUF_NAMESPACE_ID::StringPiece FullMessageName() {
    return "Protobuf.MessageToServer";
  }
  protected:
  explicit MessageToServer(::PROTOBUF_NAMESPACE_ID::Arena* arena);
  private:
  static void ArenaDtor(void* object);
  inline void RegisterArenaDtor(::PROTOBUF_NAMESPACE_ID::Arena* arena);
  public:

  ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadata() const final;
  private:
  static ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadataStatic() {
    ::PROTOBUF_NAMESPACE_ID::internal::AssignDescriptors(&::descriptor_table_Message2Server_2eproto);
    return ::descriptor_table_Message2Server_2eproto.file_level_metadata[kIndexInFileMessages];
  }

  public:

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  enum : int {
    kMessageFieldNumber = 9,
    kPlayerIDFieldNumber = 2,
    kMessageTypeFieldNumber = 1,
    kJobTypeFieldNumber = 4,
    kTeamIDFieldNumber = 3,
    kPropTypeFieldNumber = 5,
    kTimeInMillisecondsFieldNumber = 6,
    kAngleFieldNumber = 7,
    kToPlayerIDFieldNumber = 8,
  };
  // string message = 9;
  void clear_message();
  const std::string& message() const;
  void set_message(const std::string& value);
  void set_message(std::string&& value);
  void set_message(const char* value);
  void set_message(const char* value, size_t size);
  std::string* mutable_message();
  std::string* release_message();
  void set_allocated_message(std::string* message);
  private:
  const std::string& _internal_message() const;
  void _internal_set_message(const std::string& value);
  std::string* _internal_mutable_message();
  public:

  // int64 playerID = 2;
  void clear_playerid();
  ::PROTOBUF_NAMESPACE_ID::int64 playerid() const;
  void set_playerid(::PROTOBUF_NAMESPACE_ID::int64 value);
  private:
  ::PROTOBUF_NAMESPACE_ID::int64 _internal_playerid() const;
  void _internal_set_playerid(::PROTOBUF_NAMESPACE_ID::int64 value);
  public:

  // .Protobuf.MessageType messageType = 1;
  void clear_messagetype();
  ::Protobuf::MessageType messagetype() const;
  void set_messagetype(::Protobuf::MessageType value);
  private:
  ::Protobuf::MessageType _internal_messagetype() const;
  void _internal_set_messagetype(::Protobuf::MessageType value);
  public:

  // .Protobuf.JobType jobType = 4;
  void clear_jobtype();
  ::Protobuf::JobType jobtype() const;
  void set_jobtype(::Protobuf::JobType value);
  private:
  ::Protobuf::JobType _internal_jobtype() const;
  void _internal_set_jobtype(::Protobuf::JobType value);
  public:

  // int64 teamID = 3;
  void clear_teamid();
  ::PROTOBUF_NAMESPACE_ID::int64 teamid() const;
  void set_teamid(::PROTOBUF_NAMESPACE_ID::int64 value);
  private:
  ::PROTOBUF_NAMESPACE_ID::int64 _internal_teamid() const;
  void _internal_set_teamid(::PROTOBUF_NAMESPACE_ID::int64 value);
  public:

  // .Protobuf.PropType propType = 5;
  void clear_proptype();
  ::Protobuf::PropType proptype() const;
  void set_proptype(::Protobuf::PropType value);
  private:
  ::Protobuf::PropType _internal_proptype() const;
  void _internal_set_proptype(::Protobuf::PropType value);
  public:

  // int32 timeInMilliseconds = 6;
  void clear_timeinmilliseconds();
  ::PROTOBUF_NAMESPACE_ID::int32 timeinmilliseconds() const;
  void set_timeinmilliseconds(::PROTOBUF_NAMESPACE_ID::int32 value);
  private:
  ::PROTOBUF_NAMESPACE_ID::int32 _internal_timeinmilliseconds() const;
  void _internal_set_timeinmilliseconds(::PROTOBUF_NAMESPACE_ID::int32 value);
  public:

  // double angle = 7;
  void clear_angle();
  double angle() const;
  void set_angle(double value);
  private:
  double _internal_angle() const;
  void _internal_set_angle(double value);
  public:

  // int64 ToPlayerID = 8;
  void clear_toplayerid();
  ::PROTOBUF_NAMESPACE_ID::int64 toplayerid() const;
  void set_toplayerid(::PROTOBUF_NAMESPACE_ID::int64 value);
  private:
  ::PROTOBUF_NAMESPACE_ID::int64 _internal_toplayerid() const;
  void _internal_set_toplayerid(::PROTOBUF_NAMESPACE_ID::int64 value);
  public:

  // @@protoc_insertion_point(class_scope:Protobuf.MessageToServer)
 private:
  class _Internal;

  template <typename T> friend class ::PROTOBUF_NAMESPACE_ID::Arena::InternalHelper;
  typedef void InternalArenaConstructable_;
  typedef void DestructorSkippable_;
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr message_;
  ::PROTOBUF_NAMESPACE_ID::int64 playerid_;
  int messagetype_;
  int jobtype_;
  ::PROTOBUF_NAMESPACE_ID::int64 teamid_;
  int proptype_;
  ::PROTOBUF_NAMESPACE_ID::int32 timeinmilliseconds_;
  double angle_;
  ::PROTOBUF_NAMESPACE_ID::int64 toplayerid_;
  mutable ::PROTOBUF_NAMESPACE_ID::internal::CachedSize _cached_size_;
  friend struct ::TableStruct_Message2Server_2eproto;
};
// ===================================================================


// ===================================================================

#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
// MessageToServer

// .Protobuf.MessageType messageType = 1;
inline void MessageToServer::clear_messagetype() {
  messagetype_ = 0;
}
inline ::Protobuf::MessageType MessageToServer::_internal_messagetype() const {
  return static_cast< ::Protobuf::MessageType >(messagetype_);
}
inline ::Protobuf::MessageType MessageToServer::messagetype() const {
  // @@protoc_insertion_point(field_get:Protobuf.MessageToServer.messageType)
  return _internal_messagetype();
}
inline void MessageToServer::_internal_set_messagetype(::Protobuf::MessageType value) {
  
  messagetype_ = value;
}
inline void MessageToServer::set_messagetype(::Protobuf::MessageType value) {
  _internal_set_messagetype(value);
  // @@protoc_insertion_point(field_set:Protobuf.MessageToServer.messageType)
}

// int64 playerID = 2;
inline void MessageToServer::clear_playerid() {
  playerid_ = PROTOBUF_LONGLONG(0);
}
inline ::PROTOBUF_NAMESPACE_ID::int64 MessageToServer::_internal_playerid() const {
  return playerid_;
}
inline ::PROTOBUF_NAMESPACE_ID::int64 MessageToServer::playerid() const {
  // @@protoc_insertion_point(field_get:Protobuf.MessageToServer.playerID)
  return _internal_playerid();
}
inline void MessageToServer::_internal_set_playerid(::PROTOBUF_NAMESPACE_ID::int64 value) {
  
  playerid_ = value;
}
inline void MessageToServer::set_playerid(::PROTOBUF_NAMESPACE_ID::int64 value) {
  _internal_set_playerid(value);
  // @@protoc_insertion_point(field_set:Protobuf.MessageToServer.playerID)
}

// int64 teamID = 3;
inline void MessageToServer::clear_teamid() {
  teamid_ = PROTOBUF_LONGLONG(0);
}
inline ::PROTOBUF_NAMESPACE_ID::int64 MessageToServer::_internal_teamid() const {
  return teamid_;
}
inline ::PROTOBUF_NAMESPACE_ID::int64 MessageToServer::teamid() const {
  // @@protoc_insertion_point(field_get:Protobuf.MessageToServer.teamID)
  return _internal_teamid();
}
inline void MessageToServer::_internal_set_teamid(::PROTOBUF_NAMESPACE_ID::int64 value) {
  
  teamid_ = value;
}
inline void MessageToServer::set_teamid(::PROTOBUF_NAMESPACE_ID::int64 value) {
  _internal_set_teamid(value);
  // @@protoc_insertion_point(field_set:Protobuf.MessageToServer.teamID)
}

// .Protobuf.JobType jobType = 4;
inline void MessageToServer::clear_jobtype() {
  jobtype_ = 0;
}
inline ::Protobuf::JobType MessageToServer::_internal_jobtype() const {
  return static_cast< ::Protobuf::JobType >(jobtype_);
}
inline ::Protobuf::JobType MessageToServer::jobtype() const {
  // @@protoc_insertion_point(field_get:Protobuf.MessageToServer.jobType)
  return _internal_jobtype();
}
inline void MessageToServer::_internal_set_jobtype(::Protobuf::JobType value) {
  
  jobtype_ = value;
}
inline void MessageToServer::set_jobtype(::Protobuf::JobType value) {
  _internal_set_jobtype(value);
  // @@protoc_insertion_point(field_set:Protobuf.MessageToServer.jobType)
}

// .Protobuf.PropType propType = 5;
inline void MessageToServer::clear_proptype() {
  proptype_ = 0;
}
inline ::Protobuf::PropType MessageToServer::_internal_proptype() const {
  return static_cast< ::Protobuf::PropType >(proptype_);
}
inline ::Protobuf::PropType MessageToServer::proptype() const {
  // @@protoc_insertion_point(field_get:Protobuf.MessageToServer.propType)
  return _internal_proptype();
}
inline void MessageToServer::_internal_set_proptype(::Protobuf::PropType value) {
  
  proptype_ = value;
}
inline void MessageToServer::set_proptype(::Protobuf::PropType value) {
  _internal_set_proptype(value);
  // @@protoc_insertion_point(field_set:Protobuf.MessageToServer.propType)
}

// int32 timeInMilliseconds = 6;
inline void MessageToServer::clear_timeinmilliseconds() {
  timeinmilliseconds_ = 0;
}
inline ::PROTOBUF_NAMESPACE_ID::int32 MessageToServer::_internal_timeinmilliseconds() const {
  return timeinmilliseconds_;
}
inline ::PROTOBUF_NAMESPACE_ID::int32 MessageToServer::timeinmilliseconds() const {
  // @@protoc_insertion_point(field_get:Protobuf.MessageToServer.timeInMilliseconds)
  return _internal_timeinmilliseconds();
}
inline void MessageToServer::_internal_set_timeinmilliseconds(::PROTOBUF_NAMESPACE_ID::int32 value) {
  
  timeinmilliseconds_ = value;
}
inline void MessageToServer::set_timeinmilliseconds(::PROTOBUF_NAMESPACE_ID::int32 value) {
  _internal_set_timeinmilliseconds(value);
  // @@protoc_insertion_point(field_set:Protobuf.MessageToServer.timeInMilliseconds)
}

// double angle = 7;
inline void MessageToServer::clear_angle() {
  angle_ = 0;
}
inline double MessageToServer::_internal_angle() const {
  return angle_;
}
inline double MessageToServer::angle() const {
  // @@protoc_insertion_point(field_get:Protobuf.MessageToServer.angle)
  return _internal_angle();
}
inline void MessageToServer::_internal_set_angle(double value) {
  
  angle_ = value;
}
inline void MessageToServer::set_angle(double value) {
  _internal_set_angle(value);
  // @@protoc_insertion_point(field_set:Protobuf.MessageToServer.angle)
}

// int64 ToPlayerID = 8;
inline void MessageToServer::clear_toplayerid() {
  toplayerid_ = PROTOBUF_LONGLONG(0);
}
inline ::PROTOBUF_NAMESPACE_ID::int64 MessageToServer::_internal_toplayerid() const {
  return toplayerid_;
}
inline ::PROTOBUF_NAMESPACE_ID::int64 MessageToServer::toplayerid() const {
  // @@protoc_insertion_point(field_get:Protobuf.MessageToServer.ToPlayerID)
  return _internal_toplayerid();
}
inline void MessageToServer::_internal_set_toplayerid(::PROTOBUF_NAMESPACE_ID::int64 value) {
  
  toplayerid_ = value;
}
inline void MessageToServer::set_toplayerid(::PROTOBUF_NAMESPACE_ID::int64 value) {
  _internal_set_toplayerid(value);
  // @@protoc_insertion_point(field_set:Protobuf.MessageToServer.ToPlayerID)
}

// string message = 9;
inline void MessageToServer::clear_message() {
  message_.ClearToEmpty();
}
inline const std::string& MessageToServer::message() const {
  // @@protoc_insertion_point(field_get:Protobuf.MessageToServer.message)
  return _internal_message();
}
inline void MessageToServer::set_message(const std::string& value) {
  _internal_set_message(value);
  // @@protoc_insertion_point(field_set:Protobuf.MessageToServer.message)
}
inline std::string* MessageToServer::mutable_message() {
  // @@protoc_insertion_point(field_mutable:Protobuf.MessageToServer.message)
  return _internal_mutable_message();
}
inline const std::string& MessageToServer::_internal_message() const {
  return message_.Get();
}
inline void MessageToServer::_internal_set_message(const std::string& value) {
  
  message_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, value, GetArena());
}
inline void MessageToServer::set_message(std::string&& value) {
  
  message_.Set(
    ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, ::std::move(value), GetArena());
  // @@protoc_insertion_point(field_set_rvalue:Protobuf.MessageToServer.message)
}
inline void MessageToServer::set_message(const char* value) {
  GOOGLE_DCHECK(value != nullptr);
  
  message_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, ::std::string(value), GetArena());
  // @@protoc_insertion_point(field_set_char:Protobuf.MessageToServer.message)
}
inline void MessageToServer::set_message(const char* value,
    size_t size) {
  
  message_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, ::std::string(
      reinterpret_cast<const char*>(value), size), GetArena());
  // @@protoc_insertion_point(field_set_pointer:Protobuf.MessageToServer.message)
}
inline std::string* MessageToServer::_internal_mutable_message() {
  
  return message_.Mutable(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, GetArena());
}
inline std::string* MessageToServer::release_message() {
  // @@protoc_insertion_point(field_release:Protobuf.MessageToServer.message)
  return message_.Release(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArena());
}
inline void MessageToServer::set_allocated_message(std::string* message) {
  if (message != nullptr) {
    
  } else {
    
  }
  message_.SetAllocated(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), message,
      GetArena());
  // @@protoc_insertion_point(field_set_allocated:Protobuf.MessageToServer.message)
}

#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif  // __GNUC__

// @@protoc_insertion_point(namespace_scope)

}  // namespace Protobuf

// @@protoc_insertion_point(global_scope)

#include <google/protobuf/port_undef.inc>
#endif  // GOOGLE_PROTOBUF_INCLUDED_GOOGLE_PROTOBUF_INCLUDED_Message2Server_2eproto
