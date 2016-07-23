// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: proto/client_link.proto

#ifndef PROTOBUF_proto_2fclient_5flink_2eproto__INCLUDED
#define PROTOBUF_proto_2fclient_5flink_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 2006000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 2006001 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)

namespace cli {

// Internal implementation detail -- do not call these.
void  protobuf_AddDesc_proto_2fclient_5flink_2eproto();
void protobuf_AssignDesc_proto_2fclient_5flink_2eproto();
void protobuf_ShutdownFile_proto_2fclient_5flink_2eproto();

class UserAuthReq;
class UserAuthRsp;

// ===================================================================

class UserAuthReq : public ::google::protobuf::Message {
 public:
  UserAuthReq();
  virtual ~UserAuthReq();

  UserAuthReq(const UserAuthReq& from);

  inline UserAuthReq& operator=(const UserAuthReq& from) {
    CopyFrom(from);
    return *this;
  }

  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _unknown_fields_;
  }

  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return &_unknown_fields_;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const UserAuthReq& default_instance();

  void Swap(UserAuthReq* other);

  // implements Message ----------------------------------------------

  UserAuthReq* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const UserAuthReq& from);
  void MergeFrom(const UserAuthReq& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  public:
  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // required uint64 token = 1;
  inline bool has_token() const;
  inline void clear_token();
  static const int kTokenFieldNumber = 1;
  inline ::google::protobuf::uint64 token() const;
  inline void set_token(::google::protobuf::uint64 value);

  // @@protoc_insertion_point(class_scope:cli.UserAuthReq)
 private:
  inline void set_has_token();
  inline void clear_has_token();

  ::google::protobuf::UnknownFieldSet _unknown_fields_;

  ::google::protobuf::uint32 _has_bits_[1];
  mutable int _cached_size_;
  ::google::protobuf::uint64 token_;
  friend void  protobuf_AddDesc_proto_2fclient_5flink_2eproto();
  friend void protobuf_AssignDesc_proto_2fclient_5flink_2eproto();
  friend void protobuf_ShutdownFile_proto_2fclient_5flink_2eproto();

  void InitAsDefaultInstance();
  static UserAuthReq* default_instance_;
};
// -------------------------------------------------------------------

class UserAuthRsp : public ::google::protobuf::Message {
 public:
  UserAuthRsp();
  virtual ~UserAuthRsp();

  UserAuthRsp(const UserAuthRsp& from);

  inline UserAuthRsp& operator=(const UserAuthRsp& from) {
    CopyFrom(from);
    return *this;
  }

  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _unknown_fields_;
  }

  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return &_unknown_fields_;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const UserAuthRsp& default_instance();

  void Swap(UserAuthRsp* other);

  // implements Message ----------------------------------------------

  UserAuthRsp* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const UserAuthRsp& from);
  void MergeFrom(const UserAuthRsp& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  public:
  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // @@protoc_insertion_point(class_scope:cli.UserAuthRsp)
 private:

  ::google::protobuf::UnknownFieldSet _unknown_fields_;

  ::google::protobuf::uint32 _has_bits_[1];
  mutable int _cached_size_;
  friend void  protobuf_AddDesc_proto_2fclient_5flink_2eproto();
  friend void protobuf_AssignDesc_proto_2fclient_5flink_2eproto();
  friend void protobuf_ShutdownFile_proto_2fclient_5flink_2eproto();

  void InitAsDefaultInstance();
  static UserAuthRsp* default_instance_;
};
// ===================================================================


// ===================================================================

// UserAuthReq

// required uint64 token = 1;
inline bool UserAuthReq::has_token() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void UserAuthReq::set_has_token() {
  _has_bits_[0] |= 0x00000001u;
}
inline void UserAuthReq::clear_has_token() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void UserAuthReq::clear_token() {
  token_ = GOOGLE_ULONGLONG(0);
  clear_has_token();
}
inline ::google::protobuf::uint64 UserAuthReq::token() const {
  // @@protoc_insertion_point(field_get:cli.UserAuthReq.token)
  return token_;
}
inline void UserAuthReq::set_token(::google::protobuf::uint64 value) {
  set_has_token();
  token_ = value;
  // @@protoc_insertion_point(field_set:cli.UserAuthReq.token)
}

// -------------------------------------------------------------------

// UserAuthRsp


// @@protoc_insertion_point(namespace_scope)

}  // namespace cli

#ifndef SWIG
namespace google {
namespace protobuf {


}  // namespace google
}  // namespace protobuf
#endif  // SWIG

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_proto_2fclient_5flink_2eproto__INCLUDED
