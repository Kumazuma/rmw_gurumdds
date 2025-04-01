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

#include "rmw/allocators.h"
#include "rmw/error_handling.h"
#include "rmw/rmw.h"
#include "rmw/impl/cpp/macros.hpp"

#include "rmw_gurumdds_cpp/dds_include.hpp"
#include "rmw_gurumdds_cpp/identifier.hpp"
#include "rmw_gurumdds_cpp/rmw_wait.hpp"

rmw_ret_t
gather_event_conditions(
  rmw_events_t * events,
  std::vector<dds_StatusCondition *> & status_conditions)
{
  RMW_CHECK_ARGUMENT_FOR_NULL(events, RMW_RET_INVALID_ARGUMENT);
  std::unordered_map<dds_StatusCondition *, dds_StatusMask> status_map;
  status_map.reserve(events->event_count * 2);
  for (size_t i = 0; i < events->event_count; i++) {
    auto now = static_cast<rmw_event_t *>(events->events[i]);
    RMW_CHECK_ARGUMENT_FOR_NULL(events, RMW_RET_INVALID_ARGUMENT);

    auto event_info = static_cast<GurumddsEventInfo *>(now->data);
    if (event_info == nullptr) {
      RMW_SET_ERROR_MSG("event handle is null");
      return RMW_RET_ERROR;
    }

    dds_StatusCondition * status_condition = event_info->get_statuscondition();
    if (status_condition == nullptr) {
      RMW_SET_ERROR_MSG("failed to get status condition");
      return RMW_RET_ERROR;
    }

    if (is_event_supported(now->event_type)) {
      auto map_pair = status_map.insert(std::make_pair(status_condition, 0));
      auto it = map_pair.first;
      status_map[status_condition] = get_status_kind_from_rmw(now->event_type) | it->second;
    } else {
      RMW_SET_ERROR_MSG_WITH_FORMAT_STRING("unsupported event: %d", now->event_type);
    }
  }

  for (auto & map_pair : status_map) {
    dds_StatusCondition_set_enabled_statuses(map_pair.first, map_pair.second);
    status_conditions.push_back(map_pair.first);
  }

  return RMW_RET_OK;
}

rmw_ret_t
handle_active_event_conditions(rmw_events_t * events)
{
  if (events == nullptr) {
    return RMW_RET_OK;
  }

  for (size_t i = 0; i < events->event_count; i++) {
    auto now = static_cast<rmw_event_t *>(events->events[i]);
    auto event_info = static_cast<GurumddsEventInfo *>(now->data);
    if (event_info == nullptr) {
      RMW_SET_ERROR_MSG("event handle is null");
      return RMW_RET_ERROR;
    }

    dds_StatusMask mask = event_info->get_status_changes();
    bool is_active = false;

    if (is_event_supported(now->event_type)) {
      is_active = ((mask & get_status_kind_from_rmw(now->event_type)) != 0);
    }

    if (!is_active) {
      events->events[i] = nullptr;
    }
  }

  return RMW_RET_OK;
}

template<typename T>
bool check_reattached(const std::vector<T*> & cached_subscription, void** array, size_t count) {
  if(array == nullptr || count == 0)
    return !cached_subscription.empty();

  if(cached_subscription.size() != count)
    return true;

  return memcmp(cached_subscription.data(), array, sizeof(void*) * count) != 0;
}

template<typename T>
void detach_conditions(dds_WaitSet* wait_set, const T & cached_conditions) {
  for(auto condition : cached_conditions) {
    dds_WaitSet_detach_condition(wait_set, reinterpret_cast<dds_Condition*>(condition));
  }
}

inline rmw_ret_t
rmw_wait(
  const char * implementation_identifier,
  rmw_subscriptions_t * subscriptions,
  rmw_guard_conditions_t * guard_conditions,
  rmw_services_t * services,
  rmw_clients_t * clients,
  rmw_events_t * events,
  rmw_wait_set_t * wait_set,
  const rmw_time_t * wait_timeout)
{
  using ServiceInfo = GurumddsServiceInfo;
  using ClientInfo = GurumddsClientInfo;
  using SubscriberInfo = GurumddsSubscriberInfo;

  RMW_CHECK_ARGUMENT_FOR_NULL(wait_set, RMW_RET_INVALID_ARGUMENT);
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    wait set handle, wait_set->implementation_identifier,
    implementation_identifier, return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  GurumddsWaitSetInfo * wait_set_info = static_cast<GurumddsWaitSetInfo *>(wait_set->data);
  if (wait_set_info == nullptr) {
    RMW_SET_ERROR_MSG("WaitSet implementation struct is null");
    return RMW_RET_ERROR;
  }

  std::lock_guard<std::mutex> lock_guard{wait_set_info->lock};
  dds_WaitSet * dds_wait_set = static_cast<dds_WaitSet *>(wait_set_info->wait_set);
  if (dds_wait_set == nullptr) {
    RMW_SET_ERROR_MSG("DDS wait set handle is null");
    return RMW_RET_ERROR;
  }

  struct atexit_t
  {
    ~atexit_t()
    {
      if(!error_set)
        return;

      if (wait_set == nullptr) {
        RMW_SET_ERROR_MSG("wait set handle is null");
        return;
      }

      RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
          wait set handle, wait_set->implementation_identifier,
          implementation_identifier, return )

      GurumddsWaitSetInfo * wait_set_info = static_cast<GurumddsWaitSetInfo *>(wait_set->data);
      if (wait_set_info == nullptr) {
        RMW_SET_ERROR_MSG("WaitSet implementation struct is null");
        return;
      }

      dds_WaitSet * dds_wait_set = static_cast<dds_WaitSet *>(wait_set_info->wait_set);
      if (dds_wait_set == nullptr) {
        RMW_SET_ERROR_MSG("DDS wait set handle is null");
        return;
      }

      dds_ConditionSeq * attached_conditions = wait_set_info->attached_conditions;
      dds_ReturnCode_t ret = dds_WaitSet_get_conditions(dds_wait_set, attached_conditions);
      if (ret != dds_RETCODE_OK) {
        RMW_SET_ERROR_MSG("failed to get attached conditions for wait set");
        return;
      }

      for (uint32_t i = 0; i < dds_ConditionSeq_length(attached_conditions); ++i) {
        ret = dds_WaitSet_detach_condition(
            dds_wait_set, dds_ConditionSeq_get(attached_conditions, i));
        if (ret != dds_RETCODE_OK) {
          RMW_SET_ERROR_MSG("failed to detach condition from wait set");
        }
      }

      while (dds_ConditionSeq_length(attached_conditions) > 0) {
        dds_ConditionSeq_remove(attached_conditions, 0);
      }

      wait_set_info->cached_subscription.clear();
      wait_set_info->cached_guard_conditions.clear();
      wait_set_info->cached_service_conditions.clear();
      wait_set_info->cached_client_conditions.clear();
      wait_set_info->cached_event_conditions.clear();
      wait_set_info->cached_status_conditions.clear();
      wait_set_info->cached_subscription.shrink_to_fit();
      wait_set_info->cached_guard_conditions.shrink_to_fit();
      wait_set_info->cached_service_conditions.shrink_to_fit();
      wait_set_info->cached_client_conditions.shrink_to_fit();
      wait_set_info->cached_event_conditions.shrink_to_fit();
      wait_set_info->cached_status_conditions.shrink_to_fit();
    }

    rmw_wait_set_t * wait_set;
    const char * implementation_identifier;
    bool error_set;
  } atexit { wait_set, implementation_identifier, true};

  dds_ConditionSeq * active_conditions = wait_set_info->active_conditions;
  if (active_conditions == nullptr) {
    RMW_SET_ERROR_MSG("DDS condition sequence handle is null");
    return RMW_RET_ERROR;
  }

  bool need_reattached = false;
  need_reattached = check_reattached(wait_set_info->cached_subscription, (subscriptions != nullptr) ? subscriptions->subscribers : nullptr,
                   subscriptions != nullptr ? subscriptions->subscriber_count : 0);

  if(!need_reattached)
    need_reattached = check_reattached(wait_set_info->cached_guard_conditions, (guard_conditions != nullptr) ? guard_conditions->guard_conditions : nullptr,
                                       guard_conditions != nullptr ? guard_conditions->guard_condition_count : 0);

  if(!need_reattached)
    need_reattached = check_reattached(wait_set_info->cached_service_conditions, (services != nullptr) ? services->services : nullptr,
                                       services != nullptr ? services->service_count : 0);

  if(!need_reattached)
    need_reattached = check_reattached(wait_set_info->cached_client_conditions, (clients != nullptr) ? clients->clients : nullptr,
                                       clients != nullptr ? clients->client_count : 0);

  if(!need_reattached)
    need_reattached = check_reattached(wait_set_info->cached_event_conditions, events != nullptr ? events->events : nullptr, events != nullptr ? events->event_count : 0);
  if(need_reattached) {
    dds_ConditionSeq * attached_conditions =
        static_cast<dds_ConditionSeq *>(wait_set_info->attached_conditions);
    dds_ReturnCode_t ret = dds_WaitSet_get_conditions(dds_wait_set, attached_conditions);
    uint32_t length = dds_ConditionSeq_length(attached_conditions);
    for (uint32_t i = 0; i < length; ++i) {
      ret = dds_WaitSet_detach_condition(
          dds_wait_set, dds_ConditionSeq_get(attached_conditions, i));
      if (ret != dds_RETCODE_OK) {
        RMW_SET_ERROR_MSG("failed to detach condition from wait set");
      }
    }

    if(length != 0) {
      do {
        length -= 1;
        dds_ConditionSeq_remove(attached_conditions, length);

      } while(length != 0);
    }

    wait_set_info->cached_event_conditions.clear();
    wait_set_info->cached_status_conditions.clear();
    wait_set_info->cached_client_conditions.clear();
    wait_set_info->cached_service_conditions.clear();
    wait_set_info->cached_guard_conditions.clear();
    wait_set_info->cached_subscription.clear();
    if(subscriptions != nullptr) {
      for(uint32_t i = 0; i < subscriptions->subscriber_count; ++i) {
        auto it = static_cast<SubscriberInfo*>(subscriptions->subscribers[i]);
        if(it != nullptr)
          dds_WaitSet_attach_condition(wait_set_info->wait_set, reinterpret_cast<dds_Condition*>(it->read_condition));

        wait_set_info->cached_subscription.push_back(it);
      }
    }

    if(guard_conditions != nullptr) {
      for(uint32_t i = 0; i < guard_conditions->guard_condition_count; ++i) {
        auto it = static_cast<dds_GuardCondition*>(guard_conditions->guard_conditions[i]);
        if(it != nullptr)
          dds_WaitSet_attach_condition(wait_set_info->wait_set, reinterpret_cast<dds_Condition*>(it));

        wait_set_info->cached_guard_conditions.push_back(it);
      }
    }

    if(services != nullptr) {
      for(uint32_t i = 0; i < services->service_count; ++i) {
        auto it = static_cast<ServiceInfo*>(services->services[i]);
        if(it != nullptr)
          dds_WaitSet_attach_condition(wait_set_info->wait_set, reinterpret_cast<dds_Condition*>(it->read_condition));

        wait_set_info->cached_service_conditions.push_back(it);
      }
    }

    if(clients != nullptr) {
      for(uint32_t i = 0; i < clients->client_count; ++i) {
        auto it = static_cast<ClientInfo*>(clients->clients[i]);
        if(it != nullptr)
          dds_WaitSet_attach_condition(wait_set_info->wait_set, reinterpret_cast<dds_Condition*>(it->read_condition));

        wait_set_info->cached_client_conditions.push_back(it);
      }
    }

    if(events != nullptr) {
      for(uint32_t i = 0; i < events->event_count; ++i) {
        auto it = static_cast<GurumddsEventInfo*>(events->events[i]);
        if(it != nullptr)
          wait_set_info->cached_event_conditions.push_back(it);
      }

      gather_event_conditions(events, wait_set_info->cached_status_conditions);
      for (auto status_condition : wait_set_info->cached_status_conditions) {
        dds_WaitSet_attach_condition(
            dds_wait_set,
            reinterpret_cast<dds_Condition *>(status_condition));
      }
    }

    wait_set_info->cached_subscription.shrink_to_fit();
    wait_set_info->cached_guard_conditions.shrink_to_fit();
    wait_set_info->cached_service_conditions.shrink_to_fit();
    wait_set_info->cached_client_conditions.shrink_to_fit();
    wait_set_info->cached_event_conditions.shrink_to_fit();
    wait_set_info->cached_status_conditions.shrink_to_fit();
  }

  rmw_ret_t rret = RMW_RET_OK;
  dds_Duration_t timeout;
  if (wait_timeout == nullptr) {
    timeout.sec = dds_DURATION_INFINITE_SEC;
    timeout.nanosec = dds_DURATION_ZERO_NSEC;
  } else {
    timeout.sec = static_cast<int32_t>(wait_timeout->sec);
    timeout.nanosec = static_cast<uint32_t>(wait_timeout->nsec);
  }

  dds_ReturnCode_t status = dds_WaitSet_wait(dds_wait_set, active_conditions, &timeout);
  if (status != dds_RETCODE_OK && status != dds_RETCODE_TIMEOUT) {
    RMW_SET_ERROR_MSG("failed to wait on wait set");
    return RMW_RET_ERROR;
  }

  if (status == dds_RETCODE_TIMEOUT) {
    rret = RMW_RET_TIMEOUT;
    if (subscriptions != nullptr) {
      memset(subscriptions->subscribers, 0, sizeof(void*) * subscriptions->subscriber_count);
    }

    if (guard_conditions != nullptr) {
      memset(guard_conditions->guard_conditions, 0, sizeof(void*) * guard_conditions->guard_condition_count);
    }

    if (services != nullptr) {
      memset(services->services, 0, sizeof(void*) * services->service_count);
    }

    if (clients != nullptr) {
      memset(clients->clients, 0, sizeof(void*) * clients->client_count);
    }

    if (events != nullptr) {
      memset(events->events, 0, sizeof(void*) * events->event_count);
    }


  } else {
    if (subscriptions != nullptr) {
    for (size_t i = 0; i < subscriptions->subscriber_count; ++i) {
      auto * subscriber_info = static_cast<SubscriberInfo *>(subscriptions->subscribers[i]);
        if (subscriber_info == nullptr) {
          RMW_SET_ERROR_MSG("subscriber info handle is null");
          return RMW_RET_ERROR;
        }

        dds_ReadCondition * read_condition = subscriber_info->read_condition;
        if (!read_condition) {
          RMW_SET_ERROR_MSG("read condition handle is null");
          return RMW_RET_ERROR;
        }

        uint32_t j = 0;
        const uint32_t active_conditions_count = dds_ConditionSeq_length(active_conditions);
        for (; j < active_conditions_count; ++j) {
          if (
            dds_ConditionSeq_get(active_conditions, j) ==
            reinterpret_cast<dds_Condition *>(read_condition))
          {
            break;
          }
        }

        if (j >= active_conditions_count) {
          subscriptions->subscribers[i] = nullptr;
        }
      }
    }

    if (guard_conditions != nullptr) {
      for (size_t i = 0; i < guard_conditions->guard_condition_count; ++i) {
        auto * condition =
          static_cast<dds_Condition *>(guard_conditions->guard_conditions[i]);
        if (condition == nullptr) {
          RMW_SET_ERROR_MSG("condition handle is null");
          return RMW_RET_ERROR;
        }

        uint32_t j = 0;
        const uint32_t active_conditions_count = dds_ConditionSeq_length(active_conditions);
        for (; j < active_conditions_count; ++j) {
          if (dds_ConditionSeq_get(active_conditions, j) == condition) {
            dds_GuardCondition * guard = reinterpret_cast<dds_GuardCondition *>(condition);
            dds_ReturnCode_t ret = dds_GuardCondition_set_trigger_value(guard, false);
            if (ret != dds_RETCODE_OK) {
              RMW_SET_ERROR_MSG("failed to set trigger value");
              return RMW_RET_ERROR;
            }
            break;
          }
        }

        if (j >= active_conditions_count) {
          guard_conditions->guard_conditions[i] = nullptr;
        }
      }
    }

    if (services != nullptr) {
      for (size_t i = 0; i < services->service_count; ++i) {
        auto * service_info = static_cast<ServiceInfo *>(services->services[i]);
        if (service_info == nullptr) {
          RMW_SET_ERROR_MSG("service info handle is null");
          return RMW_RET_ERROR;
        }

        dds_ReadCondition * read_condition = service_info->read_condition;
        if (read_condition == nullptr) {
          RMW_SET_ERROR_MSG("read condition handle is null");
          return RMW_RET_ERROR;
        }

        uint32_t j = 0;
        const uint32_t active_conditions_count = dds_ConditionSeq_length(active_conditions);
        for (; j < active_conditions_count; ++j) {
          if (
            dds_ConditionSeq_get(active_conditions, j) ==
            reinterpret_cast<dds_Condition *>(read_condition))
          {
            break;
          }
        }

        if (j >= active_conditions_count) {
          services->services[i] = nullptr;
        }
      }
    }

    if (clients != nullptr) {
      for (size_t i = 0; i < clients->client_count; ++i) {
        ClientInfo * client_info = static_cast<ClientInfo *>(clients->clients[i]);
        if (client_info == nullptr) {
          RMW_SET_ERROR_MSG("client info handle is null");
          return RMW_RET_ERROR;
        }

        dds_ReadCondition * read_condition = client_info->read_condition;
        if (read_condition == nullptr) {
          RMW_SET_ERROR_MSG("read condition handle is null");
          return RMW_RET_ERROR;
        }

        uint32_t j = 0;
        const uint32_t active_conditions_count = dds_ConditionSeq_length(active_conditions);
        for (; j < active_conditions_count; ++j) {
          if (
            dds_ConditionSeq_get(active_conditions, j) ==
            reinterpret_cast<dds_Condition *>(read_condition))
          {
            break;
          }
        }

        if (j >= active_conditions_count) {
          clients->clients[i] = nullptr;
        }
      }
    }

    {
      rmw_ret_t rmw_ret_code = handle_active_event_conditions(events);
      if (rmw_ret_code != RMW_RET_OK) {
        return rmw_ret_code;
      }
    }
  }

  atexit.error_set = false;
  return rret;
}

extern "C"
{
rmw_wait_set_t *
rmw_create_wait_set(rmw_context_t * context, size_t max_conditions)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(context, nullptr);
  RMW_CHECK_FOR_NULL_WITH_MSG(
    context->impl,
    "expected initialized context",
    return nullptr);
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    context,
    context->implementation_identifier,
    RMW_GURUMDDS_ID,
    return nullptr);

  (void)max_conditions;
  rmw_wait_set_t * wait_set = rmw_wait_set_allocate();

  GurumddsWaitSetInfo * wait_set_info = nullptr;

  if (!wait_set) {
    RMW_SET_ERROR_MSG("failed to allocate wait set");
    goto fail;
  }

  wait_set->implementation_identifier = RMW_GURUMDDS_ID;
  wait_set->data = rmw_allocate(sizeof(GurumddsWaitSetInfo));
  wait_set_info = static_cast<GurumddsWaitSetInfo *>(wait_set->data);
  if(wait_set_info == nullptr) {
    RMW_SET_ERROR_MSG("failed to allocate wait set");
    goto fail;
  }

  RMW_TRY_PLACEMENT_NEW(wait_set_info, wait_set_info, goto fail, GurumddsWaitSetInfo);
  wait_set_info->wait_set = dds_WaitSet_create();
  if (wait_set_info->wait_set == nullptr) {
    RMW_SET_ERROR_MSG("failed to allocate wait set");
    goto fail;
  }

  wait_set_info->active_conditions = dds_ConditionSeq_create(4);
  if (wait_set_info->active_conditions == nullptr) {
    RMW_SET_ERROR_MSG("failed to allocate active_conditions sequence");
    goto fail;
  }

  wait_set_info->attached_conditions = dds_ConditionSeq_create(4);
  if (wait_set_info->attached_conditions == nullptr) {
    RMW_SET_ERROR_MSG("failed to allocate attached_conditions sequence");
    goto fail;
  }

  return wait_set;

fail:
  if (wait_set_info != nullptr) {
    if (wait_set_info->active_conditions != nullptr) {
      dds_ConditionSeq_delete(wait_set_info->active_conditions);
    }

    if (wait_set_info->attached_conditions != nullptr) {
      dds_ConditionSeq_delete(wait_set_info->attached_conditions);
    }

    if (wait_set_info->wait_set != nullptr) {
      dds_WaitSet_delete(wait_set_info->wait_set);
    }

    wait_set_info = nullptr;
  }

  if (wait_set != nullptr) {
    if (wait_set->data != nullptr) {
      delete static_cast<GurumddsWaitSetInfo*>(wait_set->data);
      wait_set->data = nullptr;
    }
  }

  if (wait_set != nullptr) {
    rmw_wait_set_free(wait_set);
  }
  return nullptr;
}

rmw_ret_t
rmw_destroy_wait_set(rmw_wait_set_t * wait_set)
{
  RMW_CHECK_ARGUMENT_FOR_NULL(wait_set, RMW_RET_ERROR);
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    wait_set,
    wait_set->implementation_identifier,
    RMW_GURUMDDS_ID,
    return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  auto * wait_set_info = static_cast<GurumddsWaitSetInfo *>(wait_set->data);
  wait_set->data = nullptr;
  if(wait_set_info != nullptr) {
    if (wait_set_info->active_conditions != nullptr) {
      dds_ConditionSeq_delete(wait_set_info->active_conditions);
    }

    if (wait_set_info->attached_conditions != nullptr) {
      dds_ConditionSeq_delete(wait_set_info->attached_conditions);
    }

    if (wait_set_info->wait_set != nullptr) {
      dds_WaitSet_delete(wait_set_info->wait_set);
    }

    RMW_TRY_DESTRUCTOR(wait_set_info->~GurumddsWaitSetInfo(), ws, return RMW_RET_ERROR);
    rmw_free(wait_set_info);
  }

  rmw_wait_set_free(wait_set);
  return RMW_RET_OK;
}

rmw_ret_t
rmw_wait(
  rmw_subscriptions_t * subscriptions,
  rmw_guard_conditions_t * guard_conditions,
  rmw_services_t * services,
  rmw_clients_t * clients,
  rmw_events_t * events,
  rmw_wait_set_t * wait_set,
  const rmw_time_t * wait_timeout)
{
  return rmw_wait(
    RMW_GURUMDDS_ID, subscriptions, guard_conditions,
    services, clients, events, wait_set, wait_timeout);
}
}  // extern "C"
