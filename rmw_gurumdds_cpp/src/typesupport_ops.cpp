#include "typesupport_ops.hpp"
#include "type_support_common.hpp"
#include "type_support_service.hpp"

size_t gurumdds_ts_get_serialized_size(void* context, void* data) {
  const rosidl_message_type_support_t * rosidl_typesupport =
      reinterpret_cast<rosidl_message_type_support_t *>(context);

  return get_serialized_size(rosidl_typesupport->data, rosidl_typesupport->typesupport_identifier, data);
}

size_t gurumdds_ts_get_size(const rosidl_message_type_support_t* rosidl_typesupport) {
  if (rosidl_typesupport->typesupport_identifier == rosidl_typesupport_introspection_c__identifier) {
    auto members = static_cast<const rosidl_typesupport_introspection_c__MessageMembers *>(rosidl_typesupport->data);
    return members->size_of_;
  } else if (rosidl_typesupport->typesupport_identifier == rosidl_typesupport_introspection_cpp::typesupport_identifier) {
    auto members = static_cast<const rosidl_typesupport_introspection_cpp::MessageMembers *>(rosidl_typesupport->data);
    return members->size_of_;
  }

  return 0;
}

size_t gurumdds_ts_get_size(void* context) {
  auto rosidl_typesupport = reinterpret_cast<rosidl_message_type_support_t *>(context);

  if (rosidl_typesupport->typesupport_identifier == rosidl_typesupport_introspection_c__identifier) {
    auto members = static_cast<const rosidl_typesupport_introspection_c__MessageMembers *>(rosidl_typesupport->data);
    return members->size_of_;
  } else if (rosidl_typesupport->typesupport_identifier == rosidl_typesupport_introspection_cpp::typesupport_identifier) {
    auto members = static_cast<const rosidl_typesupport_introspection_cpp::MessageMembers *>(rosidl_typesupport->data);
    return members->size_of_;
  }

  return 0;
}

size_t gurumdds_ts_serialize_direct(void* context, void* data, void* buffer, size_t buffer_size) {
  auto rosidl_typesupport =
      reinterpret_cast<rosidl_message_type_support_t *>(context);

  serialize_ros_to_cdr(
      rosidl_typesupport->data,
      rosidl_typesupport->typesupport_identifier,
      data,
      buffer,
      buffer_size);

  return buffer_size;
}

bool gurumdds_ts_deserialize_direct(void* context, void* buffer, size_t buffer_size, void* data) {
  auto rosidl_typesupport = reinterpret_cast<rosidl_message_type_support_t  *>(context);

  return deserialize_cdr_to_ros(
      rosidl_typesupport->data,
      rosidl_typesupport->typesupport_identifier,
      data,
      buffer,
      buffer_size);
}

template<typename ServiceMembersT>
size_t get_request_enhanced_size_tmpl(const void * untyped_members)
{
  auto members = static_cast<const ServiceMembersT *>(untyped_members);
  if (members == nullptr) {
    RMW_SET_ERROR_MSG("Members handle is null");
    return 0;
  }

  return members->request_members_->size_of_;
}

size_t get_request_enhanced_size(const void * untyped_members, const char * identifier)
{
  if (identifier == rosidl_typesupport_introspection_c__identifier) {
    return get_request_enhanced_size_tmpl<rosidl_typesupport_introspection_c__ServiceMembers>(untyped_members);
  } else if (identifier == rosidl_typesupport_introspection_cpp::typesupport_identifier) {
    return get_request_enhanced_size_tmpl<rosidl_typesupport_introspection_cpp::ServiceMembers>(untyped_members);
  }

  RMW_SET_ERROR_MSG("Unknown typesupport identifier");
  return 0;
}

template<typename ServiceMembersT>
size_t get_response_enhanced_size_tmpl(const void * untyped_members)
{
  auto members = static_cast<const ServiceMembersT *>(untyped_members);
  if (members == nullptr) {
    RMW_SET_ERROR_MSG("Members handle is null");
    return 0;
  }

  return members->response_members_->size_of_;
}

size_t get_response_enhanced_size(const void * untyped_members, const char * identifier)
{
  if (identifier == rosidl_typesupport_introspection_c__identifier) {
    return get_response_enhanced_size_tmpl<rosidl_typesupport_introspection_c__ServiceMembers>(untyped_members);
  } else if (identifier == rosidl_typesupport_introspection_cpp::typesupport_identifier) {
    return get_response_enhanced_size_tmpl<rosidl_typesupport_introspection_cpp::ServiceMembers>(untyped_members);
  }

  RMW_SET_ERROR_MSG("Unknown typesupport identifier");
  return 0;
}

size_t gurumdds_ts_get_service_request_size(void* context) {
  auto rosidl_typesupport = reinterpret_cast<rosidl_service_type_support_t *>(context);
  return get_request_enhanced_size(rosidl_typesupport->data, rosidl_typesupport->typesupport_identifier);
}

size_t gurumdds_ts_get_service_request_serialized_size(void* context, void* data) {
  auto rosidl_typesupport =
      reinterpret_cast<rosidl_service_type_support_t *>(context);
  if (rosidl_typesupport->typesupport_identifier == rosidl_typesupport_introspection_c__identifier) {
    auto untyped_member =
        static_cast<const rosidl_typesupport_introspection_c__ServiceMembers*>(rosidl_typesupport->data)->request_members_;
    return get_serialized_size(untyped_member, rosidl_typesupport->typesupport_identifier, data);
  } else if (rosidl_typesupport->typesupport_identifier == rosidl_typesupport_introspection_cpp::typesupport_identifier) {
    auto untyped_member =
        static_cast<const rosidl_typesupport_introspection_cpp::ServiceMembers*>(rosidl_typesupport->data)->request_members_;
    return get_serialized_size(untyped_member, rosidl_typesupport->typesupport_identifier, data);
  }

  RMW_SET_ERROR_MSG("Unknown typesupport identifier");
  return 0;
}

size_t gurumdds_ts_get_service_request_serialize_direct(void* context, void* data, void* buffer, size_t buffer_size) {
  bool ret = false;
  auto rosidl_typesupport =
      reinterpret_cast<rosidl_service_type_support_t *>(context);
  ret = serialize_request_enhanced(rosidl_typesupport->data, rosidl_typesupport->typesupport_identifier, static_cast<uint8_t*>(data), static_cast<uint8_t*>(buffer), buffer_size);
  return ret ? buffer_size : 0;
}

bool gurumdds_ts_service_request_deserialize_direct(void* context, void* buffer, size_t buffer_size, void* data) {
  bool ret = false;
  auto rosidl_typesupport =
      reinterpret_cast<rosidl_service_type_support_t *>(context);
  ret = deserialize_request_enhanced(rosidl_typesupport->data, rosidl_typesupport->typesupport_identifier, static_cast<uint8_t*>(data), static_cast<uint8_t*>(buffer), buffer_size);
  return ret;
}

size_t gurumdds_ts_get_service_response_size(void* context) {
  auto rosidl_typesupport = reinterpret_cast<rosidl_service_type_support_t *>(context);
  return get_response_enhanced_size(rosidl_typesupport->data, rosidl_typesupport->typesupport_identifier);
}

size_t gurumdds_ts_get_service_response_serialized_size(void* context, void* data) {
  auto rosidl_typesupport =
      reinterpret_cast<rosidl_service_type_support_t *>(context);
  if (rosidl_typesupport->typesupport_identifier == rosidl_typesupport_introspection_c__identifier) {
    auto untyped_member =
        static_cast<const rosidl_typesupport_introspection_c__ServiceMembers*>(rosidl_typesupport->data)->response_members_;
    return get_serialized_size(untyped_member, rosidl_typesupport->typesupport_identifier, data);
  } else if (rosidl_typesupport->typesupport_identifier == rosidl_typesupport_introspection_cpp::typesupport_identifier) {
    auto untyped_member =
        static_cast<const rosidl_typesupport_introspection_cpp::ServiceMembers*>(rosidl_typesupport->data)->response_members_;
    return get_serialized_size(untyped_member, rosidl_typesupport->typesupport_identifier, data);
  }

  RMW_SET_ERROR_MSG("Unknown typesupport identifier");
  return 0;
}

size_t gurumdds_ts_get_service_response_serialize_direct(void* context, void* data, void* buffer, size_t buffer_size) {
  bool ret = false;
  auto rosidl_typesupport =
      reinterpret_cast<rosidl_service_type_support_t *>(context);
  ret = serialize_response_enhanced(rosidl_typesupport->data, rosidl_typesupport->typesupport_identifier, static_cast<uint8_t*>(data), static_cast<uint8_t*>(buffer), buffer_size);
  return ret ? buffer_size : 0;
}

bool gurumdds_ts_service_response_deserialize_direct(void* context, void* buffer, size_t buffer_size, void* data) {
  bool ret = false;
  auto rosidl_typesupport =
      reinterpret_cast<rosidl_service_type_support_t *>(context);
  ret = deserialize_response_enhanced(rosidl_typesupport->data, rosidl_typesupport->typesupport_identifier, static_cast<uint8_t*>(data), static_cast<uint8_t*>(buffer), buffer_size);
  return ret;
}

void init_message_typeSupport_ops(dds_TypeSupport_ops& ops) {
  ops.get_size = gurumdds_ts_get_size;
  ops.get_serialized_size = gurumdds_ts_get_serialized_size;
  ops.serialize_direct = gurumdds_ts_serialize_direct;
  ops.deserialize_direct = gurumdds_ts_deserialize_direct;
}

void init_service_request_typesupport_ops(dds_TypeSupport_ops& ops) {
  ops.get_size = gurumdds_ts_get_service_request_size;
  ops.get_serialized_size = gurumdds_ts_get_service_request_serialized_size;
  ops.serialize_direct = gurumdds_ts_get_service_request_serialize_direct;
  ops.deserialize_direct = gurumdds_ts_service_request_deserialize_direct;
}

void init_service_reply_typesupport_ops(dds_TypeSupport_ops& ops) {
  ops.get_size = gurumdds_ts_get_service_response_size;
  ops.get_serialized_size = gurumdds_ts_get_service_response_serialized_size;
  ops.serialize_direct = gurumdds_ts_get_service_response_serialize_direct;
  ops.deserialize_direct = gurumdds_ts_service_response_deserialize_direct;
}

dds_TypeSupport* create_dds_typesupport(const rosidl_message_type_support_t* rosidl_typesupport) {
  dds_TypeSupport_ops dds_ops{};
  dds_ops.context = const_cast<rosidl_message_type_support_t*>(rosidl_typesupport);
  init_message_typeSupport_ops(dds_ops);
  std::string metastring =
      create_metastring(rosidl_typesupport->data, rosidl_typesupport->typesupport_identifier);
  if (metastring.empty()) {
    // Error message is already set
    return nullptr;
  }

  auto dds_typesupport = dds_TypeSupport_create(metastring.c_str());
  if (dds_typesupport == nullptr)
    return nullptr;

  dds_TypeSupport_set_operations(dds_typesupport, &dds_ops);
  return dds_typesupport;
}

void set_rosidl_typesupport(dds_DataWriter* entity, const rosidl_message_type_support_t * rosidl_typesupport) {
  dds_TypeSupport_ops dds_ops{};
  dds_ops.context = const_cast<rosidl_message_type_support_t*>(rosidl_typesupport);
  init_message_typeSupport_ops(dds_ops);
  auto typesupport = dds_DataWriter_get_typesupport(entity);
  dds_TypeSupport_set_operations(typesupport, &dds_ops);
}

void set_rosidl_typesupport(dds_DataReader* entity, const rosidl_message_type_support_t * rosidl_typesupport) {
  dds_TypeSupport_ops dds_ops{};
  dds_ops.context = const_cast<rosidl_message_type_support_t*>(rosidl_typesupport);
  init_message_typeSupport_ops(dds_ops);
  auto typesupport = dds_DataReader_get_typesupport(entity);
  dds_TypeSupport_set_operations(typesupport, &dds_ops);
}

void set_service_typesupport(dds_DataWriter* writer, dds_DataReader* reader, const rosidl_service_type_support_t* rosidl_typesupport) {
  dds_TypeSupport_ops dds_request_ops{};
  dds_TypeSupport_ops dds_response_ops{};
  dds_request_ops.context = const_cast<rosidl_service_type_support_t *>(rosidl_typesupport);
  dds_response_ops.context = const_cast<rosidl_service_type_support_t *>(rosidl_typesupport);
  init_service_request_typesupport_ops(dds_request_ops);
  init_service_reply_typesupport_ops(dds_response_ops);
  auto request_typesupport = dds_DataReader_get_typesupport(reader);
  auto response_typesupport = dds_DataWriter_get_typesupport(writer);

  dds_TypeSupport_set_operations(request_typesupport, &dds_request_ops);
  dds_TypeSupport_set_operations(response_typesupport, &dds_response_ops);
}

void set_client_typesupport(dds_DataWriter* writer, dds_DataReader* reader, const rosidl_service_type_support_t* rosidl_typesupport) {
  dds_TypeSupport_ops dds_request_ops{};
  dds_TypeSupport_ops dds_response_ops{};
  dds_request_ops.context = const_cast<rosidl_service_type_support_t *>(rosidl_typesupport);
  dds_response_ops.context = const_cast<rosidl_service_type_support_t *>(rosidl_typesupport);
  init_service_request_typesupport_ops(dds_request_ops);
  init_service_reply_typesupport_ops(dds_response_ops);
  auto request_typesupport = dds_DataWriter_get_typesupport(writer);
  auto response_typesupport = dds_DataReader_get_typesupport(reader);

  dds_TypeSupport_set_operations(request_typesupport, &dds_request_ops);
  dds_TypeSupport_set_operations(response_typesupport, &dds_response_ops);
}
