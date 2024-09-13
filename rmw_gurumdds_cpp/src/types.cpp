// Copyright 2019 GurumNetworks, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <cstdint>

#include "rmw/impl/cpp/key_value.hpp"

#include "rmw_gurumdds_cpp/event_converter.hpp"
#include "rmw_gurumdds_cpp/gid.hpp"
#include "rmw_gurumdds_cpp/graph_cache.hpp"
#include "rmw_gurumdds_cpp/guid.hpp"
#include "rmw_gurumdds_cpp/qos.hpp"
#include "rmw_gurumdds_cpp/rmw_context_impl.hpp"
#include "rmw_gurumdds_cpp/types.hpp"

#define ENTITYID_PARTICIPANT 0x000001C1

void GurumddsPublisherInfo::update_inconsistent_topic(int32_t total_count, int32_t total_count_change)
{
  std::lock_guard guard_callback{mutex_cb};
  inconsistent_topic_status.total_count_change += total_count_change;
  inconsistent_topic_status.total_count = total_count;
  inconsistent_topic_changed = true;

  auto callback = on_new_event_cb[RMW_EVENT_SUBSCRIPTION_INCOMPATIBLE_TYPE];
  auto user_data = user_data_cb[RMW_EVENT_SUBSCRIPTION_INCOMPATIBLE_TYPE];
  if(nullptr == callback) {
    callback(user_data, total_count_change);
  }
}

rmw_ret_t GurumddsPublisherInfo::set_on_new_event_callback(
  rmw_event_type_t event_type,
  const void * user_data,
  rmw_event_callback_t callback) {
  // mask는 RMW 측에서 보관하게 만든다.
  std::lock_guard guard{mutex_cb};
  dds_StatusMask event_status_type = get_status_kind_from_rmw(event_type);
  if(callback != nullptr) {
    int32_t changes;
    switch(event_type) {
      case RMW_EVENT_LIVELINESS_LOST:
      {
        dds_LivelinessLostStatus status{};
        dds_DataWriter_get_liveliness_lost_status(topic_writer, &status);
        changes = status.total_count_change;
      }
        break;
      case RMW_EVENT_OFFERED_DEADLINE_MISSED:
      {
        dds_OfferedDeadlineMissedStatus status{};
        dds_DataWriter_get_offered_deadline_missed_status(topic_writer, &status);
        changes = status.total_count_change;
      }
        break;
      case RMW_EVENT_OFFERED_QOS_INCOMPATIBLE:
      {
        dds_OfferedIncompatibleQosStatus status{};
        dds_DataWriter_get_offered_incompatible_qos_status(topic_writer, &status);
        changes = status.total_count_change;
      }
        break;
      case RMW_EVENT_PUBLISHER_INCOMPATIBLE_TYPE:
      {
        dds_Topic* topic = dds_DataWriter_get_topic(topic_writer);
        if(inconsistent_topic_changed) {
          inconsistent_topic_changed = false;
        } else {
          dds_Topic_get_inconsistent_topic_status(topic, &inconsistent_topic_status);
        }

        changes = inconsistent_topic_status.total_count_change;
        inconsistent_topic_status.total_count_change = 0;
      }
        break;
      case RMW_EVENT_PUBLICATION_MATCHED:
      {
        dds_PublicationMatchedStatus status{};
        dds_DataWriter_get_publication_matched_status(topic_writer, &status);
        changes = status.total_count_change;
      }

        break;
      default:
          return RMW_RET_UNSUPPORTED;
    }

    if(changes > 0) {
      callback(user_data, event_type);
    }

    mask |= event_status_type;
    on_new_event_cb[event_type] = callback;
    user_data_cb[event_type] = user_data;
  } else {
    mask &= ~event_status_type;
    on_new_event_cb[event_type] = nullptr;
    user_data_cb[event_type] = nullptr;
  }

  dds_DataWriter_set_listener(topic_writer, &topic_listener, mask);

  return RMW_RET_OK;
}

void GurumddsSubscriberInfo::update_inconsistent_topic(int32_t total_count, int32_t total_count_change) {
  inconsistent_topic_status.total_count_change += total_count_change;
  inconsistent_topic_status.total_count = total_count;
  inconsistent_topic_changed = true;

  auto callback = on_new_event_cb[RMW_EVENT_SUBSCRIPTION_INCOMPATIBLE_TYPE];
  auto ctx = user_data_cb[RMW_EVENT_SUBSCRIPTION_INCOMPATIBLE_TYPE];
  if(nullptr == callback) {
    callback(ctx, total_count_change);
  }
}

rmw_ret_t GurumddsSubscriberInfo::set_on_new_event_callback(
  rmw_event_type_t event_type,
  const void * user_data,
  rmw_event_callback_t callback)
{
  // mask는 RMW 측에서 보관하게 만든다.
  std::lock_guard guard{mutex_cb};
  dds_StatusMask event_status_type = get_status_kind_from_rmw(event_type);
  if(callback != nullptr) {
    int32_t changes;
    switch(event_type) {
      case RMW_EVENT_LIVELINESS_CHANGED:
      {
        dds_LivelinessChangedStatus status{};
        dds_DataReader_get_liveliness_changed_status(topic_reader, &status);
        changes = status.alive_count_change;
      }
        break;
      case RMW_EVENT_REQUESTED_DEADLINE_MISSED:
      {
        dds_RequestedDeadlineMissedStatus status{};
        dds_DataReader_get_requested_deadline_missed_status(topic_reader, &status);
        changes = status.total_count_change;
      }
        break;
      case RMW_EVENT_REQUESTED_QOS_INCOMPATIBLE:
      {
        dds_RequestedDeadlineMissedStatus status{};
        dds_DataReader_get_requested_deadline_missed_status(topic_reader, &status);
        changes = status.total_count_change;
      }
        break;
      case RMW_EVENT_MESSAGE_LOST:
      {
        dds_SampleLostStatus status{};
        dds_DataReader_get_sample_lost_status(topic_reader, &status);
        changes = status.total_count_change;
      }
        break;
      case RMW_EVENT_SUBSCRIPTION_INCOMPATIBLE_TYPE:
      {
        dds_Topic* topic = reinterpret_cast<dds_Topic*>(dds_DataReader_get_topicdescription(topic_reader));
        if(inconsistent_topic_changed) {
          inconsistent_topic_changed = false;
        } else {
          dds_Topic_get_inconsistent_topic_status(topic, &inconsistent_topic_status);
        }

        changes = inconsistent_topic_status.total_count_change;
        inconsistent_topic_status.total_count_change = 0;
      }
        break;
      case RMW_EVENT_SUBSCRIPTION_MATCHED:
      {
        dds_SubscriptionMatchedStatus status{};
        dds_DataReader_get_subscription_matched_status(topic_reader, &status);
        changes = status.total_count_change;
      }

        break;
      default:
        return RMW_RET_UNSUPPORTED;
    }

    if(changes > 0) {
      callback(user_data, event_type);
    }

    mask |= event_status_type;
    on_new_event_cb[event_type] = callback;
    user_data_cb[event_type] = user_data;
  } else {
    mask &= ~event_status_type;
    on_new_event_cb[event_type] = nullptr;
    user_data_cb[event_type] = nullptr;
  }

  dds_DataReader_set_listener(topic_reader, &topic_listener, mask);

  return RMW_RET_OK;
}

static std::map<std::string, std::vector<uint8_t>>
__parse_map(uint8_t * const data, const uint32_t data_len)
{
  std::vector<uint8_t> data_vec(data, data + data_len);
  std::map<std::string, std::vector<uint8_t>> map =
    rmw::impl::cpp::parse_key_value(data_vec);

  return map;
}

static rmw_ret_t
__get_user_data_key(
  dds_ParticipantBuiltinTopicData * data,
  const std::string& key,
  std::string & value,
  bool & found)
{
  found = false;
  uint8_t * user_data =
    static_cast<uint8_t *>(data->user_data.value);
  const uint32_t user_data_len = data->user_data.size;
  if (user_data_len == 0) {
    return RMW_RET_OK;
  }

  auto map = __parse_map(user_data, user_data_len);
  auto name_found = map.find(key);
  if (name_found != map.end()) {
    value = std::string(name_found->second.begin(), name_found->second.end());
    found = true;
  }

  return RMW_RET_OK;
}

void on_participant_changed(
  const dds_DomainParticipant * a_participant,
  const dds_ParticipantBuiltinTopicData * data,
  dds_InstanceHandle_t handle)
{
  dds_DomainParticipant * participant = const_cast<dds_DomainParticipant *>(a_participant);
  rmw_context_impl_t * ctx =
    reinterpret_cast<rmw_context_impl_t *>(
    dds_Entity_get_context(reinterpret_cast<dds_Entity *>(participant), 0));

  if (ctx == nullptr) {
    return;
  }

  dds_GUID_t dp_guid;
  GuidPrefix_t dp_guid_prefix;
  dds_BuiltinTopicKey_to_GUID(&dp_guid_prefix, data->key);
  memcpy(dp_guid.prefix, dp_guid_prefix.value, sizeof(dp_guid.prefix));
  dp_guid.entityId = ENTITYID_PARTICIPANT;

  if (handle == dds_HANDLE_NIL) {
    graph_remove_participant(ctx, &dp_guid);
  } else {
    std::string enclave_str;
    bool enclave_found;
    dds_ReturnCode_t rc =
      __get_user_data_key(
      const_cast<dds_ParticipantBuiltinTopicData *>(data),
      "securitycontext", enclave_str, enclave_found);
    if (RMW_RET_OK != rc) {
      RMW_SET_ERROR_MSG("failed to parse user data for enclave");
    }

    const char * enclave = nullptr;
    if (enclave_found) {
      enclave = enclave_str.c_str();
    }

    if (RMW_RET_OK != graph_add_participant(ctx, &dp_guid, enclave)) {
      RMW_SET_ERROR_MSG("failed to assert remote participant in graph");
    }
  }
}

void on_publication_changed(
  const dds_DomainParticipant * a_participant,
  const dds_PublicationBuiltinTopicData * data,
  dds_InstanceHandle_t handle)
{
  dds_DomainParticipant * participant = const_cast<dds_DomainParticipant *>(a_participant);
  rmw_context_impl_t * ctx =
    reinterpret_cast<rmw_context_impl_t *>(
    dds_Entity_get_context(reinterpret_cast<dds_Entity *>(participant), 0));

  if (ctx == nullptr) {
    return;
  }

  dds_GUID_t endp_guid;
  GuidPrefix_t dp_guid_prefix, endp_guid_prefix;
  dds_BuiltinTopicKey_to_GUID(&dp_guid_prefix, data->participant_key);
  memcpy(endp_guid.prefix, dp_guid_prefix.value, sizeof(endp_guid.prefix));
  dds_BuiltinTopicKey_to_GUID(&endp_guid_prefix, data->key);
  memcpy(&endp_guid.entityId, endp_guid_prefix.value, sizeof(endp_guid.entityId));

  if (handle == dds_HANDLE_NIL) {
    RCUTILS_LOG_DEBUG_NAMED(
      "pub on data available",
      "[ud] endp_gid=0x%08X.0x%08X.0x%08X.0x%08X ",
      reinterpret_cast<const uint32_t *>(endp_guid.prefix)[0],
      reinterpret_cast<const uint32_t *>(endp_guid.prefix)[1],
      reinterpret_cast<const uint32_t *>(endp_guid.prefix)[2],
      endp_guid.entityId);
    graph_remove_entity(ctx, &endp_guid, false);
  } else {
    dds_GUID_t dp_guid;
    memcpy(dp_guid.prefix, dp_guid_prefix.value, sizeof(dp_guid.prefix));
    dp_guid.entityId = ENTITYID_PARTICIPANT;

    graph_add_remote_entity(
      ctx,
      &endp_guid,
      &dp_guid,
      data->topic_name,
      data->type_name,
      data->user_data,
      &data->reliability,
      &data->durability,
      &data->deadline,
      &data->liveliness,
      &data->lifespan,
      false);

    RCUTILS_LOG_DEBUG_NAMED(
      "pub on data available",
      "dp_gid=0x%08X.0x%08X.0x%08X.0x%08X, "
      "gid=0x%08X.0x%08X.0x%08X.0x%08X, ",
      reinterpret_cast<const uint32_t *>(dp_guid.prefix)[0],
      reinterpret_cast<const uint32_t *>(dp_guid.prefix)[1],
      reinterpret_cast<const uint32_t *>(dp_guid.prefix)[2],
      dp_guid.entityId,
      reinterpret_cast<const uint32_t *>(endp_guid.prefix)[0],
      reinterpret_cast<const uint32_t *>(endp_guid.prefix)[1],
      reinterpret_cast<const uint32_t *>(endp_guid.prefix)[2],
      endp_guid.entityId);
  }
}

void on_subscription_changed(
  const dds_DomainParticipant * a_participant,
  const dds_SubscriptionBuiltinTopicData * data,
  dds_InstanceHandle_t handle)
{
  dds_DomainParticipant * participant = const_cast<dds_DomainParticipant *>(a_participant);
  rmw_context_impl_t * ctx =
    reinterpret_cast<rmw_context_impl_t *>(
    dds_Entity_get_context(reinterpret_cast<dds_Entity *>(participant), 0));

  if (ctx == nullptr) {
    return;
  }

  dds_GUID_t endp_guid;
  GuidPrefix_t dp_guid_prefix, endp_guid_prefix;
  dds_BuiltinTopicKey_to_GUID(&dp_guid_prefix, data->participant_key);
  memcpy(endp_guid.prefix, dp_guid_prefix.value, sizeof(endp_guid.prefix));
  dds_BuiltinTopicKey_to_GUID(&endp_guid_prefix, data->key);
  memcpy(&endp_guid.entityId, endp_guid_prefix.value, sizeof(endp_guid.entityId));

  if (handle == dds_HANDLE_NIL) {
    RCUTILS_LOG_DEBUG_NAMED(
      "sub on data available",
      "[ud] endp_gid=0x%08X.0x%08X.0x%08X.0x%08X ",
      reinterpret_cast<const uint32_t *>(endp_guid.prefix)[0],
      reinterpret_cast<const uint32_t *>(endp_guid.prefix)[1],
      reinterpret_cast<const uint32_t *>(endp_guid.prefix)[2],
      endp_guid.entityId);
    graph_remove_entity(ctx, &endp_guid, false);
  } else {
    dds_GUID_t dp_guid;
    memcpy(dp_guid.prefix, dp_guid_prefix.value, sizeof(dp_guid.prefix));
    dp_guid.entityId = ENTITYID_PARTICIPANT;

    graph_add_remote_entity(
      ctx,
      &endp_guid,
      &dp_guid,
      data->topic_name,
      data->type_name,
      data->user_data,
      &data->reliability,
      &data->durability,
      &data->deadline,
      &data->liveliness,
      nullptr,
      true);

    RCUTILS_LOG_DEBUG_NAMED(
      "sub on data available",
      "dp_gid=0x%08X.0x%08X.0x%08X.0x%08X, "
      "gid=0x%08X.0x%08X.0x%08X.0x%08X, ",
      reinterpret_cast<const uint32_t *>(dp_guid.prefix)[0],
      reinterpret_cast<const uint32_t *>(dp_guid.prefix)[1],
      reinterpret_cast<const uint32_t *>(dp_guid.prefix)[2],
      dp_guid.entityId,
      reinterpret_cast<const uint32_t *>(endp_guid.prefix)[0],
      reinterpret_cast<const uint32_t *>(endp_guid.prefix)[1],
      reinterpret_cast<const uint32_t *>(endp_guid.prefix)[2],
      endp_guid.entityId);
  }
}

rmw_ret_t GurumddsPublisherInfo::get_status(
  dds_StatusMask mask,
  void * event)
{
  if (mask == dds_LIVELINESS_LOST_STATUS) {
    dds_LivelinessLostStatus status;
    dds_ReturnCode_t dds_ret =
      dds_DataWriter_get_liveliness_lost_status(this->topic_writer, &status);
    rmw_ret_t rmw_ret = check_dds_ret_code(dds_ret);
    if (rmw_ret != RMW_RET_OK) {
      return rmw_ret;
    }

    auto rmw_status = static_cast<rmw_liveliness_lost_status_t *>(event);
    rmw_status->total_count = status.total_count;
    rmw_status->total_count_change = status.total_count_change;
  } else if (mask == dds_OFFERED_DEADLINE_MISSED_STATUS) {
    dds_OfferedDeadlineMissedStatus status;
    dds_ReturnCode_t dds_ret =
      dds_DataWriter_get_offered_deadline_missed_status(this->topic_writer, &status);
    rmw_ret_t rmw_ret = check_dds_ret_code(dds_ret);
    if (rmw_ret != RMW_RET_OK) {
      return rmw_ret;
    }

    auto rmw_status = static_cast<rmw_offered_deadline_missed_status_t *>(event);
    rmw_status->total_count = status.total_count;
    rmw_status->total_count_change = status.total_count_change;
  } else if (mask == dds_OFFERED_INCOMPATIBLE_QOS_STATUS) {
    dds_OfferedIncompatibleQosStatus status;
    dds_ReturnCode_t dds_ret =
      dds_DataWriter_get_offered_incompatible_qos_status(this->topic_writer, &status);
    rmw_ret_t rmw_ret = check_dds_ret_code(dds_ret);
    if (rmw_ret != RMW_RET_OK) {
      return rmw_ret;
    }

    auto rmw_status = static_cast<rmw_offered_qos_incompatible_event_status_t *>(event);
    rmw_status->total_count = status.total_count;
    rmw_status->total_count_change = status.total_count_change;
    rmw_status->last_policy_kind = convert_qos_policy(status.last_policy_id);
  } else if(mask == dds_INCONSISTENT_TOPIC_STATUS) {
    dds_Topic* const topic = dds_DataWriter_get_topic(this->topic_writer);
    std::lock_guard guard{mutex_cb};
    if(inconsistent_topic_changed) {
      inconsistent_topic_changed = false;
    } else {
      auto dds_ret = dds_Topic_get_inconsistent_topic_status(topic, &inconsistent_topic_status);
      if (dds_ret != dds_RETCODE_OK) {
        return check_dds_ret_code(dds_ret);
      }
    }

    auto const rmw_status = static_cast<rmw_incompatible_type_status_t *>(event);
    rmw_status->total_count = inconsistent_topic_status.total_count;
    rmw_status->total_count_change = inconsistent_topic_status.total_count_change;
    inconsistent_topic_status.total_count_change = 0;
  } else if (mask == dds_PUBLICATION_MATCHED_STATUS) {
    dds_PublicationMatchedStatus status{};
    dds_ReturnCode_t dds_ret =
      dds_DataWriter_get_publication_matched_status(this->topic_writer, &status);

    if(dds_ret != dds_RETCODE_OK) {
      return check_dds_ret_code(dds_ret);
    }

    auto const rmw_status = static_cast<rmw_matched_status_t *>(event);
    rmw_status->current_count = status.current_count;
    rmw_status->current_count_change = status.current_count_change;
    rmw_status->total_count = status.total_count;
    rmw_status->total_count_change = status.total_count_change;
  } else {
    return RMW_RET_UNSUPPORTED;
  }
  return RMW_RET_OK;
}

dds_StatusCondition * GurumddsPublisherInfo::get_statuscondition()
{
  return dds_DataWriter_get_statuscondition(this->topic_writer);
}

dds_StatusMask GurumddsPublisherInfo::get_status_changes()
{
  return dds_DataWriter_get_status_changes(this->topic_writer);
}

rmw_ret_t GurumddsSubscriberInfo::get_status(
  dds_StatusMask mask,
  void * event)
{
  if (mask == dds_LIVELINESS_CHANGED_STATUS) {
    dds_LivelinessChangedStatus status;
    dds_ReturnCode_t dds_ret =
      dds_DataReader_get_liveliness_changed_status(this->topic_reader, &status);
    rmw_ret_t rmw_ret = check_dds_ret_code(dds_ret);
    if (rmw_ret != RMW_RET_OK) {
      return rmw_ret;
    }

    auto rmw_status = static_cast<rmw_liveliness_changed_status_t *>(event);
    rmw_status->alive_count = status.alive_count;
    rmw_status->not_alive_count = status.not_alive_count;
    rmw_status->alive_count_change = status.alive_count_change;
    rmw_status->not_alive_count_change =
      status.not_alive_count_change;
  } else if (mask == dds_REQUESTED_DEADLINE_MISSED_STATUS) {
    dds_RequestedDeadlineMissedStatus status;
    dds_ReturnCode_t dds_ret =
      dds_DataReader_get_requested_deadline_missed_status(this->topic_reader, &status);
    rmw_ret_t rmw_ret = check_dds_ret_code(dds_ret);
    if (rmw_ret != RMW_RET_OK) {
      return rmw_ret;
    }

    auto rmw_status =
      static_cast<rmw_requested_deadline_missed_status_t *>(event);
    rmw_status->total_count = status.total_count;
    rmw_status->total_count_change = status.total_count_change;
  } else if (mask == dds_REQUESTED_INCOMPATIBLE_QOS_STATUS) {
    dds_RequestedIncompatibleQosStatus status;
    dds_ReturnCode_t dds_ret =
      dds_DataReader_get_requested_incompatible_qos_status(this->topic_reader, &status);
    rmw_ret_t rmw_ret = check_dds_ret_code(dds_ret);
    if (rmw_ret != RMW_RET_OK) {
      return rmw_ret;
    }

    auto rmw_status = static_cast<rmw_requested_qos_incompatible_event_status_t *>(event);
    rmw_status->total_count = status.total_count;
    rmw_status->total_count_change = status.total_count_change;
    rmw_status->last_policy_kind = convert_qos_policy(status.last_policy_id);
  } else if (mask == dds_SAMPLE_LOST_STATUS) {
    dds_SampleLostStatus status;
    dds_ReturnCode_t dds_ret =
      dds_DataReader_get_sample_lost_status(this->topic_reader, &status);
    rmw_ret_t rmw_ret = check_dds_ret_code(dds_ret);
    if (rmw_ret != RMW_RET_OK) {
      return rmw_ret;
    }

    auto rmw_status = static_cast<rmw_message_lost_status_t *>(event);
    rmw_status->total_count = status.total_count;
    rmw_status->total_count_change = status.total_count_change;
  } else if (mask == dds_INCONSISTENT_TOPIC_STATUS) {
    dds_Topic* const topic = reinterpret_cast<dds_Topic*>(dds_DataReader_get_topicdescription(this->topic_reader));
    dds_InconsistentTopicStatus status{};
    auto dds_ret = dds_Topic_get_inconsistent_topic_status(topic, &status);

    if(dds_ret != dds_RETCODE_OK) {
      return check_dds_ret_code(dds_ret);
    }

    auto const rmw_status = static_cast<rmw_incompatible_type_status_t *>(event);
    rmw_status->total_count = status.total_count;
    rmw_status->total_count_change = status.total_count_change;
  } else if (mask == dds_SUBSCRIPTION_MATCHED_STATUS) {
    dds_SubscriptionMatchedStatus status{};
    dds_ReturnCode_t dds_ret =
      dds_DataReader_get_subscription_matched_status(this->topic_reader, &status);

    if(dds_ret != dds_RETCODE_OK) {
      return check_dds_ret_code(dds_ret);
    }

    auto const rmw_status = static_cast<rmw_matched_status_t *>(event);
    rmw_status->current_count = status.current_count;
    rmw_status->current_count_change = status.current_count_change;
    rmw_status->total_count = status.total_count;
    rmw_status->total_count_change = status.total_count_change;
  } else {
    return RMW_RET_UNSUPPORTED;
  }
  return RMW_RET_OK;
}

dds_StatusCondition * GurumddsSubscriberInfo::get_statuscondition()
{
  return dds_DataReader_get_statuscondition(this->topic_reader);
}

dds_StatusMask GurumddsSubscriberInfo::get_status_changes()
{
  return dds_DataReader_get_status_changes(this->topic_reader);
}

inline size_t count_unread_(
  dds_DataReader * reader,
  dds_DataSeq * data_seq,
  dds_SampleInfoSeq * info_seq,
  dds_UnsignedLongSeq * raw_data_sizes)
{
  dds_ReturnCode_t rc = dds_DataReader_raw_read(
    reader,
    dds_HANDLE_NIL,
    data_seq,
    info_seq,
    raw_data_sizes,
    dds_LENGTH_UNLIMITED,
    dds_NOT_READ_SAMPLE_STATE,
    dds_ANY_VIEW_STATE,
    dds_ANY_INSTANCE_STATE
  );

  size_t count = 0;

  if (dds_RETCODE_OK != rc && dds_RETCODE_NO_DATA != rc) {
    RMW_SET_ERROR_MSG("failed to read raw data from DDS reader");
    return count;
  } else if(dds_RETCODE_OK == rc) {
    count = dds_SampleInfoSeq_length(info_seq);
  }

  rc = dds_DataReader_raw_return_loan(reader, data_seq, info_seq, raw_data_sizes);
  if (dds_RETCODE_OK != rc && dds_RETCODE_NO_DATA != rc) {
    RMW_SET_ERROR_MSG("failed to read raw data from DDS reader");
    return count;
  }

  return count;
}

size_t GurumddsSubscriberInfo::count_unread()
{
  return count_unread_(topic_reader, data_seq, info_seq, raw_data_sizes);
}

void _GurumddsSubscriberInfo::on_requested_deadline_missed(const dds_RequestedDeadlineMissedStatus & status)
{
  std::lock_guard<std::mutex> guard(mutex_cb);
  requested_deadline_missed_changed = true;
  requested_deadline_missed_status.total_count_change += status.total_count_change;
  requested_deadline_missed_status.total_count = status.total_count;
  auto callback = on_new_event_cb[RMW_EVENT_LIVELINESS_CHANGED];
  auto user_data = user_data_cb[RMW_EVENT_LIVELINESS_CHANGED];
  if(nullptr == callback) {
    callback(user_data, liveliness_changed_status.alive_count_change);
  }
}

void GurumddsSubscriberInfo::on_liveliness_changed(const dds_LivelinessChangedStatus & status)
{
  std::lock_guard<std::mutex> guard(mutex_cb);
  liveliness_changed = true;
  liveliness_changed_status.alive_count = status.alive_count;
  liveliness_changed_status.not_alive_count = status.not_alive_count;
  liveliness_changed_status.alive_count_change += status.alive_count_change;
  liveliness_changed_status.not_alive_count_change += status.not_alive_count_change;
  auto callback = on_new_event_cb[RMW_EVENT_LIVELINESS_CHANGED];
  auto user_data = user_data_cb[RMW_EVENT_LIVELINESS_CHANGED];
  if(nullptr == callback) {
    callback(user_data, liveliness_changed_status.alive_count_change);
  }
}

size_t GurumddsClientInfo::count_unread()
{
  return count_unread_(response_reader, data_seq, info_seq, raw_data_sizes);
}

size_t GurumddsServiceInfo::count_unread()
{
  return count_unread_(request_reader, data_seq, info_seq, raw_data_sizes);
}

std::mutex GurumddsTopicEventListener::mutex_table_;
std::map<dds_Topic*, GurumddsTopicEventListener*> GurumddsTopicEventListener::table_;

rmw_ret_t GurumddsTopicEventListener::associate_listener(dds_Topic * topic) {
  auto event_listener = new(std::nothrow) GurumddsTopicEventListener{};
  if(nullptr == event_listener) {
    return RMW_RET_ERROR;
  }

  std::lock_guard guard{mutex_table_};
  if(!table_.emplace(topic, event_listener).second) {
    return RMW_RET_ERROR;
  }

  dds_TopicListener listener{};
  listener.on_inconsistent_topic = &GurumddsTopicEventListener::on_inconsistent_topic;
  dds_Topic_set_listener_context(topic, event_listener);
  dds_Topic_set_listener(topic, &listener, dds_INCONSISTENT_TOPIC_STATUS);
  return RMW_RET_OK;
}

rmw_ret_t GurumddsTopicEventListener::disassociate_Listener(dds_Topic * topic) {
  std::lock_guard guard{mutex_table_};
  auto it = table_.find(topic);
  if(table_.end() == it) {
    return RMW_RET_ERROR;
  }

  auto listener = it->second;
  std::unique_lock listener_guard{listener->mutex_};
  dds_Topic_set_listener_context(topic, nullptr);
  listener_guard.unlock();
  table_.erase(it);
  delete listener;
  return RMW_RET_OK;
}

void GurumddsTopicEventListener::on_inconsistent_topic(const dds_Topic* the_topic, const dds_InconsistentTopicStatus* status) {
  auto topic = const_cast<dds_Topic*>(the_topic);
  auto listener = static_cast<GurumddsTopicEventListener*>(dds_Topic_get_listener_context(topic));
  if(nullptr == listener) {
    return;
  }

  listener->on_inconsistent_topic(*status);
}

void GurumddsTopicEventListener::on_inconsistent_topic(const dds_InconsistentTopicStatus& status) {
  std::lock_guard guard{mutex_};
  for(auto it: event_list_) {
    it->update_inconsistent_topic(status.total_count, status.total_count_change);
  }

}

void GurumddsTopicEventListener::add_event(dds_Topic * topic, GurumddsEventInfo * event_info) {
  std::lock_guard guard{mutex_table_};
  auto it = table_.find(topic);
  if(table_.end() == it) {
    return;
  }

  auto listener = it->second;
  std::unique_lock listener_guard{listener->mutex_};
  auto list_it = std::find(listener->event_list_.begin(), listener->event_list_.end(), event_info);
  if(listener->event_list_.end() == list_it) {
    return;
  }

  listener->event_list_.emplace_back(event_info);
}

void GurumddsTopicEventListener::remove_event(dds_Topic * topic, GurumddsEventInfo * event_info) {
  std::lock_guard guard{mutex_table_};
  auto it = table_.find(topic);
  if(table_.end() == it) {
    return;
  }

    auto listener = it->second;
    std::unique_lock listener_guard{listener->mutex_};
    auto list_it = std::find(listener->event_list_.begin(), listener->event_list_.end(), event_info);
    if(listener->event_list_.end() == list_it) {
      return;
    }

    listener->event_list_.erase(list_it);
}
