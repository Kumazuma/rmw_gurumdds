#ifndef MESSAGE_DESERIALIZER_INL
#define MESSAGE_DESERIALIZER_INL

template<typename MessageMembersT>
inline MessageDeserializer<MessageMembersT>::MessageDeserializer(cdr::DeserializationBuffer & buffer)
  : buffer_(buffer) {
  static_assert(std::is_same_v<MessageMemberT, rosidl_typesupport_introspection_c__MessageMember> ||
    std::is_same_v<MessageMemberT, rosidl_typesupport_introspection_cpp::MessageMember>);
}

template<typename MessageMembersT>
inline void MessageDeserializer<MessageMembersT>::deserialize(const MessageMembersT *members, uint8_t *output) {
  for (uint32_t i = 0; i < members->member_count_; i++) {
    auto member = members->members_ + i;
    switch (member->type_id_) {
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_BOOLEAN:
        read_boolean(member, output);
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_CHAR:
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_OCTET:
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT8:
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_INT8:
        read_primitive<uint8_t>(member, output);
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT16:
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_INT16:
        read_primitive<uint16_t>(member, output);
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_FLOAT:
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT32:
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_INT32:
        read_primitive<uint32_t>(member, output);
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_DOUBLE:
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_LONG_DOUBLE:
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT64:
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_INT64:
        read_primitive<uint64_t>(member, output);
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_WCHAR:
        read_wchar(member, output);
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_STRING:
        read_string(member, output);
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_WSTRING:
        read_wstring(member, output);
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_MESSAGE:
        read_struct_arr(member, output);
        break;
      default:
        break;
    }
  }
}

template<typename MessageMembersT>
inline void MessageDeserializer<MessageMembersT>::read_boolean(const MessageMemberT * member, uint8_t * output) {
  if (member->is_array_) {
    if (!member->array_size_ || member->is_upper_bound_) {
      uint32_t size = 0;
      buffer_ >> size;

      auto seq_ptr =
        (reinterpret_cast<rosidl_runtime_c__boolean__Sequence *>(
          output + member->offset_));
      if (seq_ptr->data) {
        rosidl_runtime_c__boolean__Sequence__fini(seq_ptr);
      }
      bool res = rosidl_runtime_c__boolean__Sequence__init(seq_ptr, size);
      if (!res) {
        throw std::runtime_error("Failed to initialize sequence");
      }

      for (uint32_t i = 0; i < size; i++) {
        uint8_t data = 0;
        buffer_ >> data;
        seq_ptr->data[i] = (data != 0);
      }
    } else {
      auto arr = reinterpret_cast<bool *>(output + member->offset_);
      for (uint32_t i = 0; i < member->array_size_; i++) {
        uint8_t data = 0;
        buffer_ >> data;
        arr[i] = (data != 0);
      }
    }
  } else {
    auto dst = reinterpret_cast<bool *>(output + member->offset_);
    uint8_t data = 0;
    buffer_ >> data;
    *dst = (data != 0);
  }
}

template<typename MessageMembersT>
inline void MessageDeserializer<MessageMembersT>::read_wchar(const MessageMemberT * member, uint8_t * output) {

}

template<typename MessageMembersT>
inline void MessageDeserializer<MessageMembersT>::read_string(const MessageMemberT * member, uint8_t * output) {

}

template<typename MessageMembersT>
template<typename PrimitiveT>
inline void MessageDeserializer<MessageMembersT>::read_primitive(const MessageMemberT * member, uint8_t * output) {

}

template<typename MessageMembersT>
inline void MessageDeserializer<MessageMembersT>::read_wstring(const MessageMemberT * member, uint8_t * output) {

}

template<typename MessageMembersT>
inline void MessageDeserializer<MessageMembersT>::read_struct_arr(const MessageMemberT * member, uint8_t * output) {

}

#endif  // MESSAGE_DESERIALIZER_INL
