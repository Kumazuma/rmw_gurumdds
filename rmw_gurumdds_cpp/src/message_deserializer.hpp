#ifndef MESSAGE_DESERIALIZER_HPP
#define MESSAGE_DESERIALIZER_HPP

#include "rosidl_runtime_cpp/bounded_vector.hpp"

#include "rosidl_runtime_c/primitives_sequence.h"
#include "rosidl_runtime_c/primitives_sequence_functions.h"
#include "rosidl_runtime_c/string.h"
#include "rosidl_runtime_c/string_functions.h"
#include "rosidl_runtime_c/u16string.h"
#include "rosidl_runtime_c/u16string_functions.h"

#include "rosidl_typesupport_introspection_cpp/field_types.hpp"
#include "rosidl_typesupport_introspection_cpp/message_introspection.hpp"

#include "rosidl_typesupport_introspection_c/message_introspection.h"

#include "cdr_buffer.hpp"

#include <utility>

template<typename MessageMembersT>
class MessageDeserializer {
public:
  using MessageMemberT = typename std::remove_cv_t<std::remove_pointer_t<typename std::remove_all_extents<decltype(std::declval<MessageMembersT>().members_)>::type>>;
  static constexpr LanguageKind LANGUAGE_KIND = (std::is_same_v<MessageMemberT, rosidl_typesupport_introspection_c__MessageMember> ?
                                                  LanguageKind::C : (std::is_same_v<MessageMemberT, rosidl_typesupport_introspection_cpp::MessageMember>
                                                    ? LanguageKind::CXX : LanguageKind::UNKNOWN));
    explicit MessageDeserializer(cdr::DeserializationBuffer & buffer);

  void deserialize(const MessageMembersT * members, uint8_t * output);
private:
  void read_boolean(
    const MessageMemberT * member,
    uint8_t * output);

  void read_wchar(
    const MessageMemberT * member,
    uint8_t * output);

  void read_string(
    const MessageMemberT * member,
    uint8_t * output);

  void read_wstring(
    const MessageMemberT * member,
    uint8_t * output);

  template<typename PrimitiveT>
  void read_primitive(
    const MessageMemberT * member,
    uint8_t * output);

  void read_struct_arr(
    const MessageMemberT * member,
    uint8_t * output);

  cdr::DeserializationBuffer & buffer_;
};

#include "message_deserializer.inl"

#endif  // MESSAGE_DESERIALIZER_HPP
