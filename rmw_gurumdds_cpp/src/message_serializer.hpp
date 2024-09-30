#ifndef MESSAGE_SERIALIZER_HPP
#define MESSAGE_SERIALIZER_HPP


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

namespace rmw_gurumdds
{
template<bool SERIALIZE, typename MessageMembersT>
class MessageSerializer {
public:
  using MessageMemberT = typename std::remove_cv_t<std::remove_pointer_t<typename std::remove_all_extents<decltype(std::declval<MessageMembersT>().members_)>::type>>;
  static constexpr LanguageKind LANGUAGE_KIND = (std::is_same_v<MessageMemberT, rosidl_typesupport_introspection_c__MessageMember> ?
                                                   LanguageKind::C : (std::is_same_v<MessageMemberT, rosidl_typesupport_introspection_cpp::MessageMember>
                                                      ? LanguageKind::CXX : LanguageKind::UNKNOWN));
  explicit MessageSerializer(CdrSerializationBuffer<SERIALIZE> & buffer);

  void serialize(const MessageMembersT * members, const uint8_t * input, bool roundup_);

private:
  void serialize_boolean(
    const MessageMemberT * member,
    const uint8_t * input);

  void serialize_wchar(
    const MessageMemberT * member,
    const uint8_t * input);

  void serialize_string(
    const MessageMemberT * member,
    const uint8_t * input);

  void serialize_wstring(
    const MessageMemberT * member,
    const uint8_t * input);


  void serialize_struct_arr(
    const MessageMemberT * member,
    const uint8_t * input);

  template<typename PrimitiveT>
  void serialize_primitive(
    const MessageMemberT * member,
    const uint8_t * input);
private:
  CdrSerializationBuffer<SERIALIZE> & buffer;
};
}

#include "message_serializer.inl"

#endif  // MESSAGE_SERIALIZER_HPP
