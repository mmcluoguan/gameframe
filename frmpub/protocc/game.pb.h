// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: game.proto

#ifndef GOOGLE_PROTOBUF_INCLUDED_game_2eproto
#define GOOGLE_PROTOBUF_INCLUDED_game_2eproto

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
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
#include <google/protobuf/unknown_field_set.h>
#include "common.pb.h"
// @@protoc_insertion_point(includes)
#include <google/protobuf/port_def.inc>
#define PROTOBUF_INTERNAL_EXPORT_game_2eproto
PROTOBUF_NAMESPACE_OPEN
namespace internal {
	class AnyMetadata;
}  // namespace internal
PROTOBUF_NAMESPACE_CLOSE

// Internal implementation detail -- do not use these members.
struct TableStruct_game_2eproto {
	static const ::PROTOBUF_NAMESPACE_ID::internal::ParseTableField entries[]
		PROTOBUF_SECTION_VARIABLE(protodesc_cold);
	static const ::PROTOBUF_NAMESPACE_ID::internal::AuxiliaryParseTableField aux[]
		PROTOBUF_SECTION_VARIABLE(protodesc_cold);
	static const ::PROTOBUF_NAMESPACE_ID::internal::ParseTable schema[4]
		PROTOBUF_SECTION_VARIABLE(protodesc_cold);
	static const ::PROTOBUF_NAMESPACE_ID::internal::FieldMetadata field_metadata[];
	static const ::PROTOBUF_NAMESPACE_ID::internal::SerializationTable serialization_table[];
	static const ::PROTOBUF_NAMESPACE_ID::uint32 offsets[];
};
extern const ::PROTOBUF_NAMESPACE_ID::internal::DescriptorTable descriptor_table_game_2eproto;
::PROTOBUF_NAMESPACE_ID::Metadata descriptor_table_game_2eproto_metadata_getter(int index);
namespace frmpub {
	namespace protocc {
		class register_game_dbvisit_c;
		struct register_game_dbvisit_cDefaultTypeInternal;
		extern register_game_dbvisit_cDefaultTypeInternal _register_game_dbvisit_c_default_instance_;
		class register_game_dbvisit_s;
		struct register_game_dbvisit_sDefaultTypeInternal;
		extern register_game_dbvisit_sDefaultTypeInternal _register_game_dbvisit_s_default_instance_;
		class register_game_world_c;
		struct register_game_world_cDefaultTypeInternal;
		extern register_game_world_cDefaultTypeInternal _register_game_world_c_default_instance_;
		class register_game_world_s;
		struct register_game_world_sDefaultTypeInternal;
		extern register_game_world_sDefaultTypeInternal _register_game_world_s_default_instance_;
	}  // namespace protocc
}  // namespace frmpub
PROTOBUF_NAMESPACE_OPEN
template<> ::frmpub::protocc::register_game_dbvisit_c* Arena::CreateMaybeMessage<::frmpub::protocc::register_game_dbvisit_c>(Arena*);
template<> ::frmpub::protocc::register_game_dbvisit_s* Arena::CreateMaybeMessage<::frmpub::protocc::register_game_dbvisit_s>(Arena*);
template<> ::frmpub::protocc::register_game_world_c* Arena::CreateMaybeMessage<::frmpub::protocc::register_game_world_c>(Arena*);
template<> ::frmpub::protocc::register_game_world_s* Arena::CreateMaybeMessage<::frmpub::protocc::register_game_world_s>(Arena*);
PROTOBUF_NAMESPACE_CLOSE
namespace frmpub {
	namespace protocc {

		// ===================================================================

		class register_game_dbvisit_c PROTOBUF_FINAL :
			public ::PROTOBUF_NAMESPACE_ID::Message /* @@protoc_insertion_point(class_definition:frmpub.protocc.register_game_dbvisit_c) */ {
		public:
			inline register_game_dbvisit_c() : register_game_dbvisit_c(nullptr) {}
			virtual ~register_game_dbvisit_c();
			explicit constexpr register_game_dbvisit_c(::PROTOBUF_NAMESPACE_ID::internal::ConstantInitialized);

			register_game_dbvisit_c(const register_game_dbvisit_c& from);
			register_game_dbvisit_c(register_game_dbvisit_c&& from) noexcept
				: register_game_dbvisit_c() {
				*this = ::std::move(from);
			}

			inline register_game_dbvisit_c& operator=(const register_game_dbvisit_c& from) {
				CopyFrom(from);
				return *this;
			}
			inline register_game_dbvisit_c& operator=(register_game_dbvisit_c&& from) noexcept {
				if (GetArena() == from.GetArena()) {
					if (this != &from) InternalSwap(&from);
				}
				else {
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
			static const register_game_dbvisit_c& default_instance() {
				return *internal_default_instance();
			}
			static inline const register_game_dbvisit_c* internal_default_instance() {
				return reinterpret_cast<const register_game_dbvisit_c*>(
					&_register_game_dbvisit_c_default_instance_);
			}
			static constexpr int kIndexInFileMessages =
				0;

			friend void swap(register_game_dbvisit_c& a, register_game_dbvisit_c& b) {
				a.Swap(&b);
			}
			inline void Swap(register_game_dbvisit_c* other) {
				if (other == this) return;
				if (GetArena() == other->GetArena()) {
					InternalSwap(other);
				}
				else {
					::PROTOBUF_NAMESPACE_ID::internal::GenericSwap(this, other);
				}
			}
			void UnsafeArenaSwap(register_game_dbvisit_c* other) {
				if (other == this) return;
				GOOGLE_DCHECK(GetArena() == other->GetArena());
				InternalSwap(other);
			}

			// implements Message ----------------------------------------------

			inline register_game_dbvisit_c* New() const final {
				return CreateMaybeMessage<register_game_dbvisit_c>(nullptr);
			}

			register_game_dbvisit_c* New(::PROTOBUF_NAMESPACE_ID::Arena* arena) const final {
				return CreateMaybeMessage<register_game_dbvisit_c>(arena);
			}
			void CopyFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) final;
			void MergeFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) final;
			void CopyFrom(const register_game_dbvisit_c& from);
			void MergeFrom(const register_game_dbvisit_c& from);
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
			void InternalSwap(register_game_dbvisit_c* other);
			friend class ::PROTOBUF_NAMESPACE_ID::internal::AnyMetadata;
			static ::PROTOBUF_NAMESPACE_ID::StringPiece FullMessageName() {
				return "frmpub.protocc.register_game_dbvisit_c";
			}
		protected:
			explicit register_game_dbvisit_c(::PROTOBUF_NAMESPACE_ID::Arena* arena);
		private:
			static void ArenaDtor(void* object);
			inline void RegisterArenaDtor(::PROTOBUF_NAMESPACE_ID::Arena* arena);
		public:

			::PROTOBUF_NAMESPACE_ID::Metadata GetMetadata() const final;
		private:
			static ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadataStatic() {
				return ::descriptor_table_game_2eproto_metadata_getter(kIndexInFileMessages);
			}

		public:

			// nested types ----------------------------------------------------

			// accessors -------------------------------------------------------

			enum : int {
				kSifFieldNumber = 1,
			};
			// .frmpub.protocc.ServerInfo sif = 1;
			bool has_sif() const;
		private:
			bool _internal_has_sif() const;
		public:
			void clear_sif();
			const ::frmpub::protocc::ServerInfo& sif() const;
			::frmpub::protocc::ServerInfo* release_sif();
			::frmpub::protocc::ServerInfo* mutable_sif();
			void set_allocated_sif(::frmpub::protocc::ServerInfo* sif);
		private:
			const ::frmpub::protocc::ServerInfo& _internal_sif() const;
			::frmpub::protocc::ServerInfo* _internal_mutable_sif();
		public:
			void unsafe_arena_set_allocated_sif(
				::frmpub::protocc::ServerInfo* sif);
			::frmpub::protocc::ServerInfo* unsafe_arena_release_sif();

			// @@protoc_insertion_point(class_scope:frmpub.protocc.register_game_dbvisit_c)
		private:
			class _Internal;

			template <typename T> friend class ::PROTOBUF_NAMESPACE_ID::Arena::InternalHelper;
			typedef void InternalArenaConstructable_;
			typedef void DestructorSkippable_;
			::frmpub::protocc::ServerInfo* sif_;
			mutable ::PROTOBUF_NAMESPACE_ID::internal::CachedSize _cached_size_;
			friend struct ::TableStruct_game_2eproto;
		};
		// -------------------------------------------------------------------

		class register_game_dbvisit_s PROTOBUF_FINAL :
			public ::PROTOBUF_NAMESPACE_ID::Message /* @@protoc_insertion_point(class_definition:frmpub.protocc.register_game_dbvisit_s) */ {
		public:
			inline register_game_dbvisit_s() : register_game_dbvisit_s(nullptr) {}
			virtual ~register_game_dbvisit_s();
			explicit constexpr register_game_dbvisit_s(::PROTOBUF_NAMESPACE_ID::internal::ConstantInitialized);

			register_game_dbvisit_s(const register_game_dbvisit_s& from);
			register_game_dbvisit_s(register_game_dbvisit_s&& from) noexcept
				: register_game_dbvisit_s() {
				*this = ::std::move(from);
			}

			inline register_game_dbvisit_s& operator=(const register_game_dbvisit_s& from) {
				CopyFrom(from);
				return *this;
			}
			inline register_game_dbvisit_s& operator=(register_game_dbvisit_s&& from) noexcept {
				if (GetArena() == from.GetArena()) {
					if (this != &from) InternalSwap(&from);
				}
				else {
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
			static const register_game_dbvisit_s& default_instance() {
				return *internal_default_instance();
			}
			static inline const register_game_dbvisit_s* internal_default_instance() {
				return reinterpret_cast<const register_game_dbvisit_s*>(
					&_register_game_dbvisit_s_default_instance_);
			}
			static constexpr int kIndexInFileMessages =
				1;

			friend void swap(register_game_dbvisit_s& a, register_game_dbvisit_s& b) {
				a.Swap(&b);
			}
			inline void Swap(register_game_dbvisit_s* other) {
				if (other == this) return;
				if (GetArena() == other->GetArena()) {
					InternalSwap(other);
				}
				else {
					::PROTOBUF_NAMESPACE_ID::internal::GenericSwap(this, other);
				}
			}
			void UnsafeArenaSwap(register_game_dbvisit_s* other) {
				if (other == this) return;
				GOOGLE_DCHECK(GetArena() == other->GetArena());
				InternalSwap(other);
			}

			// implements Message ----------------------------------------------

			inline register_game_dbvisit_s* New() const final {
				return CreateMaybeMessage<register_game_dbvisit_s>(nullptr);
			}

			register_game_dbvisit_s* New(::PROTOBUF_NAMESPACE_ID::Arena* arena) const final {
				return CreateMaybeMessage<register_game_dbvisit_s>(arena);
			}
			void CopyFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) final;
			void MergeFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) final;
			void CopyFrom(const register_game_dbvisit_s& from);
			void MergeFrom(const register_game_dbvisit_s& from);
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
			void InternalSwap(register_game_dbvisit_s* other);
			friend class ::PROTOBUF_NAMESPACE_ID::internal::AnyMetadata;
			static ::PROTOBUF_NAMESPACE_ID::StringPiece FullMessageName() {
				return "frmpub.protocc.register_game_dbvisit_s";
			}
		protected:
			explicit register_game_dbvisit_s(::PROTOBUF_NAMESPACE_ID::Arena* arena);
		private:
			static void ArenaDtor(void* object);
			inline void RegisterArenaDtor(::PROTOBUF_NAMESPACE_ID::Arena* arena);
		public:

			::PROTOBUF_NAMESPACE_ID::Metadata GetMetadata() const final;
		private:
			static ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadataStatic() {
				return ::descriptor_table_game_2eproto_metadata_getter(kIndexInFileMessages);
			}

		public:

			// nested types ----------------------------------------------------

			// accessors -------------------------------------------------------

			enum : int {
				kResultFieldNumber = 1,
			};
			// int32 result = 1;
			void clear_result();
			::PROTOBUF_NAMESPACE_ID::int32 result() const;
			void set_result(::PROTOBUF_NAMESPACE_ID::int32 value);
		private:
			::PROTOBUF_NAMESPACE_ID::int32 _internal_result() const;
			void _internal_set_result(::PROTOBUF_NAMESPACE_ID::int32 value);
		public:

			// @@protoc_insertion_point(class_scope:frmpub.protocc.register_game_dbvisit_s)
		private:
			class _Internal;

			template <typename T> friend class ::PROTOBUF_NAMESPACE_ID::Arena::InternalHelper;
			typedef void InternalArenaConstructable_;
			typedef void DestructorSkippable_;
			::PROTOBUF_NAMESPACE_ID::int32 result_;
			mutable ::PROTOBUF_NAMESPACE_ID::internal::CachedSize _cached_size_;
			friend struct ::TableStruct_game_2eproto;
		};
		// -------------------------------------------------------------------

		class register_game_world_c PROTOBUF_FINAL :
			public ::PROTOBUF_NAMESPACE_ID::Message /* @@protoc_insertion_point(class_definition:frmpub.protocc.register_game_world_c) */ {
		public:
			inline register_game_world_c() : register_game_world_c(nullptr) {}
			virtual ~register_game_world_c();
			explicit constexpr register_game_world_c(::PROTOBUF_NAMESPACE_ID::internal::ConstantInitialized);

			register_game_world_c(const register_game_world_c& from);
			register_game_world_c(register_game_world_c&& from) noexcept
				: register_game_world_c() {
				*this = ::std::move(from);
			}

			inline register_game_world_c& operator=(const register_game_world_c& from) {
				CopyFrom(from);
				return *this;
			}
			inline register_game_world_c& operator=(register_game_world_c&& from) noexcept {
				if (GetArena() == from.GetArena()) {
					if (this != &from) InternalSwap(&from);
				}
				else {
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
			static const register_game_world_c& default_instance() {
				return *internal_default_instance();
			}
			static inline const register_game_world_c* internal_default_instance() {
				return reinterpret_cast<const register_game_world_c*>(
					&_register_game_world_c_default_instance_);
			}
			static constexpr int kIndexInFileMessages =
				2;

			friend void swap(register_game_world_c& a, register_game_world_c& b) {
				a.Swap(&b);
			}
			inline void Swap(register_game_world_c* other) {
				if (other == this) return;
				if (GetArena() == other->GetArena()) {
					InternalSwap(other);
				}
				else {
					::PROTOBUF_NAMESPACE_ID::internal::GenericSwap(this, other);
				}
			}
			void UnsafeArenaSwap(register_game_world_c* other) {
				if (other == this) return;
				GOOGLE_DCHECK(GetArena() == other->GetArena());
				InternalSwap(other);
			}

			// implements Message ----------------------------------------------

			inline register_game_world_c* New() const final {
				return CreateMaybeMessage<register_game_world_c>(nullptr);
			}

			register_game_world_c* New(::PROTOBUF_NAMESPACE_ID::Arena* arena) const final {
				return CreateMaybeMessage<register_game_world_c>(arena);
			}
			void CopyFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) final;
			void MergeFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) final;
			void CopyFrom(const register_game_world_c& from);
			void MergeFrom(const register_game_world_c& from);
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
			void InternalSwap(register_game_world_c* other);
			friend class ::PROTOBUF_NAMESPACE_ID::internal::AnyMetadata;
			static ::PROTOBUF_NAMESPACE_ID::StringPiece FullMessageName() {
				return "frmpub.protocc.register_game_world_c";
			}
		protected:
			explicit register_game_world_c(::PROTOBUF_NAMESPACE_ID::Arena* arena);
		private:
			static void ArenaDtor(void* object);
			inline void RegisterArenaDtor(::PROTOBUF_NAMESPACE_ID::Arena* arena);
		public:

			::PROTOBUF_NAMESPACE_ID::Metadata GetMetadata() const final;
		private:
			static ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadataStatic() {
				return ::descriptor_table_game_2eproto_metadata_getter(kIndexInFileMessages);
			}

		public:

			// nested types ----------------------------------------------------

			// accessors -------------------------------------------------------

			enum : int {
				kSifFieldNumber = 1,
			};
			// .frmpub.protocc.ServerInfo sif = 1;
			bool has_sif() const;
		private:
			bool _internal_has_sif() const;
		public:
			void clear_sif();
			const ::frmpub::protocc::ServerInfo& sif() const;
			::frmpub::protocc::ServerInfo* release_sif();
			::frmpub::protocc::ServerInfo* mutable_sif();
			void set_allocated_sif(::frmpub::protocc::ServerInfo* sif);
		private:
			const ::frmpub::protocc::ServerInfo& _internal_sif() const;
			::frmpub::protocc::ServerInfo* _internal_mutable_sif();
		public:
			void unsafe_arena_set_allocated_sif(
				::frmpub::protocc::ServerInfo* sif);
			::frmpub::protocc::ServerInfo* unsafe_arena_release_sif();

			// @@protoc_insertion_point(class_scope:frmpub.protocc.register_game_world_c)
		private:
			class _Internal;

			template <typename T> friend class ::PROTOBUF_NAMESPACE_ID::Arena::InternalHelper;
			typedef void InternalArenaConstructable_;
			typedef void DestructorSkippable_;
			::frmpub::protocc::ServerInfo* sif_;
			mutable ::PROTOBUF_NAMESPACE_ID::internal::CachedSize _cached_size_;
			friend struct ::TableStruct_game_2eproto;
		};
		// -------------------------------------------------------------------

		class register_game_world_s PROTOBUF_FINAL :
			public ::PROTOBUF_NAMESPACE_ID::Message /* @@protoc_insertion_point(class_definition:frmpub.protocc.register_game_world_s) */ {
		public:
			inline register_game_world_s() : register_game_world_s(nullptr) {}
			virtual ~register_game_world_s();
			explicit constexpr register_game_world_s(::PROTOBUF_NAMESPACE_ID::internal::ConstantInitialized);

			register_game_world_s(const register_game_world_s& from);
			register_game_world_s(register_game_world_s&& from) noexcept
				: register_game_world_s() {
				*this = ::std::move(from);
			}

			inline register_game_world_s& operator=(const register_game_world_s& from) {
				CopyFrom(from);
				return *this;
			}
			inline register_game_world_s& operator=(register_game_world_s&& from) noexcept {
				if (GetArena() == from.GetArena()) {
					if (this != &from) InternalSwap(&from);
				}
				else {
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
			static const register_game_world_s& default_instance() {
				return *internal_default_instance();
			}
			static inline const register_game_world_s* internal_default_instance() {
				return reinterpret_cast<const register_game_world_s*>(
					&_register_game_world_s_default_instance_);
			}
			static constexpr int kIndexInFileMessages =
				3;

			friend void swap(register_game_world_s& a, register_game_world_s& b) {
				a.Swap(&b);
			}
			inline void Swap(register_game_world_s* other) {
				if (other == this) return;
				if (GetArena() == other->GetArena()) {
					InternalSwap(other);
				}
				else {
					::PROTOBUF_NAMESPACE_ID::internal::GenericSwap(this, other);
				}
			}
			void UnsafeArenaSwap(register_game_world_s* other) {
				if (other == this) return;
				GOOGLE_DCHECK(GetArena() == other->GetArena());
				InternalSwap(other);
			}

			// implements Message ----------------------------------------------

			inline register_game_world_s* New() const final {
				return CreateMaybeMessage<register_game_world_s>(nullptr);
			}

			register_game_world_s* New(::PROTOBUF_NAMESPACE_ID::Arena* arena) const final {
				return CreateMaybeMessage<register_game_world_s>(arena);
			}
			void CopyFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) final;
			void MergeFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) final;
			void CopyFrom(const register_game_world_s& from);
			void MergeFrom(const register_game_world_s& from);
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
			void InternalSwap(register_game_world_s* other);
			friend class ::PROTOBUF_NAMESPACE_ID::internal::AnyMetadata;
			static ::PROTOBUF_NAMESPACE_ID::StringPiece FullMessageName() {
				return "frmpub.protocc.register_game_world_s";
			}
		protected:
			explicit register_game_world_s(::PROTOBUF_NAMESPACE_ID::Arena* arena);
		private:
			static void ArenaDtor(void* object);
			inline void RegisterArenaDtor(::PROTOBUF_NAMESPACE_ID::Arena* arena);
		public:

			::PROTOBUF_NAMESPACE_ID::Metadata GetMetadata() const final;
		private:
			static ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadataStatic() {
				return ::descriptor_table_game_2eproto_metadata_getter(kIndexInFileMessages);
			}

		public:

			// nested types ----------------------------------------------------

			// accessors -------------------------------------------------------

			enum : int {
				kResultFieldNumber = 1,
			};
			// int32 result = 1;
			void clear_result();
			::PROTOBUF_NAMESPACE_ID::int32 result() const;
			void set_result(::PROTOBUF_NAMESPACE_ID::int32 value);
		private:
			::PROTOBUF_NAMESPACE_ID::int32 _internal_result() const;
			void _internal_set_result(::PROTOBUF_NAMESPACE_ID::int32 value);
		public:

			// @@protoc_insertion_point(class_scope:frmpub.protocc.register_game_world_s)
		private:
			class _Internal;

			template <typename T> friend class ::PROTOBUF_NAMESPACE_ID::Arena::InternalHelper;
			typedef void InternalArenaConstructable_;
			typedef void DestructorSkippable_;
			::PROTOBUF_NAMESPACE_ID::int32 result_;
			mutable ::PROTOBUF_NAMESPACE_ID::internal::CachedSize _cached_size_;
			friend struct ::TableStruct_game_2eproto;
		};
		// ===================================================================


		// ===================================================================

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
// register_game_dbvisit_c

// .frmpub.protocc.ServerInfo sif = 1;
		inline bool register_game_dbvisit_c::_internal_has_sif() const {
			return this != internal_default_instance() && sif_ != nullptr;
		}
		inline bool register_game_dbvisit_c::has_sif() const {
			return _internal_has_sif();
		}
		inline const ::frmpub::protocc::ServerInfo& register_game_dbvisit_c::_internal_sif() const {
			const ::frmpub::protocc::ServerInfo* p = sif_;
			return p != nullptr ? *p : reinterpret_cast<const ::frmpub::protocc::ServerInfo&>(
				::frmpub::protocc::_ServerInfo_default_instance_);
		}
		inline const ::frmpub::protocc::ServerInfo& register_game_dbvisit_c::sif() const {
			// @@protoc_insertion_point(field_get:frmpub.protocc.register_game_dbvisit_c.sif)
			return _internal_sif();
		}
		inline void register_game_dbvisit_c::unsafe_arena_set_allocated_sif(
			::frmpub::protocc::ServerInfo* sif) {
			if (GetArena() == nullptr) {
				delete reinterpret_cast<::PROTOBUF_NAMESPACE_ID::MessageLite*>(sif_);
			}
			sif_ = sif;
			if (sif) {

			}
			else {

			}
			// @@protoc_insertion_point(field_unsafe_arena_set_allocated:frmpub.protocc.register_game_dbvisit_c.sif)
		}
		inline ::frmpub::protocc::ServerInfo* register_game_dbvisit_c::release_sif() {

			::frmpub::protocc::ServerInfo* temp = sif_;
			sif_ = nullptr;
			if (GetArena() != nullptr) {
				temp = ::PROTOBUF_NAMESPACE_ID::internal::DuplicateIfNonNull(temp);
			}
			return temp;
		}
		inline ::frmpub::protocc::ServerInfo* register_game_dbvisit_c::unsafe_arena_release_sif() {
			// @@protoc_insertion_point(field_release:frmpub.protocc.register_game_dbvisit_c.sif)

			::frmpub::protocc::ServerInfo* temp = sif_;
			sif_ = nullptr;
			return temp;
		}
		inline ::frmpub::protocc::ServerInfo* register_game_dbvisit_c::_internal_mutable_sif() {

			if (sif_ == nullptr) {
				auto* p = CreateMaybeMessage<::frmpub::protocc::ServerInfo>(GetArena());
				sif_ = p;
			}
			return sif_;
		}
		inline ::frmpub::protocc::ServerInfo* register_game_dbvisit_c::mutable_sif() {
			// @@protoc_insertion_point(field_mutable:frmpub.protocc.register_game_dbvisit_c.sif)
			return _internal_mutable_sif();
		}
		inline void register_game_dbvisit_c::set_allocated_sif(::frmpub::protocc::ServerInfo* sif) {
			::PROTOBUF_NAMESPACE_ID::Arena* message_arena = GetArena();
			if (message_arena == nullptr) {
				delete reinterpret_cast<::PROTOBUF_NAMESPACE_ID::MessageLite*>(sif_);
			}
			if (sif) {
				::PROTOBUF_NAMESPACE_ID::Arena* submessage_arena =
					reinterpret_cast<::PROTOBUF_NAMESPACE_ID::MessageLite*>(sif)->GetArena();
				if (message_arena != submessage_arena) {
					sif = ::PROTOBUF_NAMESPACE_ID::internal::GetOwnedMessage(
						message_arena, sif, submessage_arena);
				}

			}
			else {

			}
			sif_ = sif;
			// @@protoc_insertion_point(field_set_allocated:frmpub.protocc.register_game_dbvisit_c.sif)
		}

		// -------------------------------------------------------------------

		// register_game_dbvisit_s

		// int32 result = 1;
		inline void register_game_dbvisit_s::clear_result() {
			result_ = 0;
		}
		inline ::PROTOBUF_NAMESPACE_ID::int32 register_game_dbvisit_s::_internal_result() const {
			return result_;
		}
		inline ::PROTOBUF_NAMESPACE_ID::int32 register_game_dbvisit_s::result() const {
			// @@protoc_insertion_point(field_get:frmpub.protocc.register_game_dbvisit_s.result)
			return _internal_result();
		}
		inline void register_game_dbvisit_s::_internal_set_result(::PROTOBUF_NAMESPACE_ID::int32 value) {

			result_ = value;
		}
		inline void register_game_dbvisit_s::set_result(::PROTOBUF_NAMESPACE_ID::int32 value) {
			_internal_set_result(value);
			// @@protoc_insertion_point(field_set:frmpub.protocc.register_game_dbvisit_s.result)
		}

		// -------------------------------------------------------------------

		// register_game_world_c

		// .frmpub.protocc.ServerInfo sif = 1;
		inline bool register_game_world_c::_internal_has_sif() const {
			return this != internal_default_instance() && sif_ != nullptr;
		}
		inline bool register_game_world_c::has_sif() const {
			return _internal_has_sif();
		}
		inline const ::frmpub::protocc::ServerInfo& register_game_world_c::_internal_sif() const {
			const ::frmpub::protocc::ServerInfo* p = sif_;
			return p != nullptr ? *p : reinterpret_cast<const ::frmpub::protocc::ServerInfo&>(
				::frmpub::protocc::_ServerInfo_default_instance_);
		}
		inline const ::frmpub::protocc::ServerInfo& register_game_world_c::sif() const {
			// @@protoc_insertion_point(field_get:frmpub.protocc.register_game_world_c.sif)
			return _internal_sif();
		}
		inline void register_game_world_c::unsafe_arena_set_allocated_sif(
			::frmpub::protocc::ServerInfo* sif) {
			if (GetArena() == nullptr) {
				delete reinterpret_cast<::PROTOBUF_NAMESPACE_ID::MessageLite*>(sif_);
			}
			sif_ = sif;
			if (sif) {

			}
			else {

			}
			// @@protoc_insertion_point(field_unsafe_arena_set_allocated:frmpub.protocc.register_game_world_c.sif)
		}
		inline ::frmpub::protocc::ServerInfo* register_game_world_c::release_sif() {

			::frmpub::protocc::ServerInfo* temp = sif_;
			sif_ = nullptr;
			if (GetArena() != nullptr) {
				temp = ::PROTOBUF_NAMESPACE_ID::internal::DuplicateIfNonNull(temp);
			}
			return temp;
		}
		inline ::frmpub::protocc::ServerInfo* register_game_world_c::unsafe_arena_release_sif() {
			// @@protoc_insertion_point(field_release:frmpub.protocc.register_game_world_c.sif)

			::frmpub::protocc::ServerInfo* temp = sif_;
			sif_ = nullptr;
			return temp;
		}
		inline ::frmpub::protocc::ServerInfo* register_game_world_c::_internal_mutable_sif() {

			if (sif_ == nullptr) {
				auto* p = CreateMaybeMessage<::frmpub::protocc::ServerInfo>(GetArena());
				sif_ = p;
			}
			return sif_;
		}
		inline ::frmpub::protocc::ServerInfo* register_game_world_c::mutable_sif() {
			// @@protoc_insertion_point(field_mutable:frmpub.protocc.register_game_world_c.sif)
			return _internal_mutable_sif();
		}
		inline void register_game_world_c::set_allocated_sif(::frmpub::protocc::ServerInfo* sif) {
			::PROTOBUF_NAMESPACE_ID::Arena* message_arena = GetArena();
			if (message_arena == nullptr) {
				delete reinterpret_cast<::PROTOBUF_NAMESPACE_ID::MessageLite*>(sif_);
			}
			if (sif) {
				::PROTOBUF_NAMESPACE_ID::Arena* submessage_arena =
					reinterpret_cast<::PROTOBUF_NAMESPACE_ID::MessageLite*>(sif)->GetArena();
				if (message_arena != submessage_arena) {
					sif = ::PROTOBUF_NAMESPACE_ID::internal::GetOwnedMessage(
						message_arena, sif, submessage_arena);
				}

			}
			else {

			}
			sif_ = sif;
			// @@protoc_insertion_point(field_set_allocated:frmpub.protocc.register_game_world_c.sif)
		}

		// -------------------------------------------------------------------

		// register_game_world_s

		// int32 result = 1;
		inline void register_game_world_s::clear_result() {
			result_ = 0;
		}
		inline ::PROTOBUF_NAMESPACE_ID::int32 register_game_world_s::_internal_result() const {
			return result_;
		}
		inline ::PROTOBUF_NAMESPACE_ID::int32 register_game_world_s::result() const {
			// @@protoc_insertion_point(field_get:frmpub.protocc.register_game_world_s.result)
			return _internal_result();
		}
		inline void register_game_world_s::_internal_set_result(::PROTOBUF_NAMESPACE_ID::int32 value) {

			result_ = value;
		}
		inline void register_game_world_s::set_result(::PROTOBUF_NAMESPACE_ID::int32 value) {
			_internal_set_result(value);
			// @@protoc_insertion_point(field_set:frmpub.protocc.register_game_world_s.result)
		}

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif  // __GNUC__
		// -------------------------------------------------------------------

		// -------------------------------------------------------------------

		// -------------------------------------------------------------------


		// @@protoc_insertion_point(namespace_scope)

	}  // namespace protocc
}  // namespace frmpub

// @@protoc_insertion_point(global_scope)

#include <google/protobuf/port_undef.inc>
#endif  // GOOGLE_PROTOBUF_INCLUDED_GOOGLE_PROTOBUF_INCLUDED_game_2eproto
