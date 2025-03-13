#ifndef RMW_GURUMDDS_CPP_TYPESUPPORT_OPS_HPP_
#define RMW_GURUMDDS_CPP_TYPESUPPORT_OPS_HPP_

#include <gurumdds/dcps.h>
#include <rosidl_runtime_c/message_type_support_struct.h>
#include <rosidl_runtime_c/service_type_support_struct.h>

dds_TypeSupport* create_dds_typesupport(const rosidl_message_type_support_t * rosidl_typesupport);

void init_message_typesupport_ops(dds_TypeSupport_ops& ops);

void init_service_request_typesupport_ops(dds_TypeSupport_ops& ops);

void init_service_reply_typesupport_ops(dds_TypeSupport_ops& ops);

void set_rosidl_typesupport(dds_DataWriter* entity, const rosidl_message_type_support_t * rosidl_typesupport);

void set_rosidl_typesupport(dds_DataReader* entity, const rosidl_message_type_support_t * rosidl_typesupport);

void set_service_typesupport(dds_DataWriter* writer, dds_DataReader* reader, const rosidl_service_type_support_t* rosidl_typesupport);

void set_client_typesupport(dds_DataWriter* writer, dds_DataReader* reader, const rosidl_service_type_support_t* rosidl_typesupport);

size_t gurumdds_ts_get_size(const rosidl_message_type_support_t* rosidl_typesupport);

size_t gurumdds_ts_msg_clear(const rosidl_message_type_support_t* rosidl_typesupport, void* ros_message);

size_t gurumdds_ts_request_msg_clear(const rosidl_service_type_support_t* rosidl_typesupport, void* ros_message);

size_t gurumdds_ts_response_msg_clear(const rosidl_service_type_support_t* rosidl_typesupport, void* ros_message);

#endif //RMW_GURUMDDS_CPP_TYPESUPPORT_OPS_HPP_
