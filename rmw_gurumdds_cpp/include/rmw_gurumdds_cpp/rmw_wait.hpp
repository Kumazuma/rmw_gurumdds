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

#ifndef RMW_GURUMDDS_CPP__RMW_WAIT_HPP_
#define RMW_GURUMDDS_CPP__RMW_WAIT_HPP_

#include <chrono>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "rmw/allocators.h"
#include "rmw/error_handling.h"
#include "rmw/rmw.h"
#include "rmw/impl/cpp/macros.hpp"

#include "rmw_gurumdds_cpp/dds_include.hpp"
#include "rmw_gurumdds_cpp/event_converter.hpp"
#include "rmw_gurumdds_cpp/types.hpp"

#define CHECK_ATTACH(ret) \
  if (ret == dds_RETCODE_OK) { \
    continue; \
  } else if (ret == dds_RETCODE_OUT_OF_RESOURCES) { \
    RMW_SET_ERROR_MSG("failed to attach condition to wait set: out of resources"); \
    return RMW_RET_ERROR; \
  } else if (ret == dds_RETCODE_BAD_PARAMETER) { \
    RMW_SET_ERROR_MSG("failed to attach condition to wait set: condition pointer was invalid"); \
    return RMW_RET_ERROR; \
  } else { \
    RMW_SET_ERROR_MSG("failed to attach condition to wait set"); \
    return RMW_RET_ERROR; \
  }


#endif  // RMW_GURUMDDS_CPP__RMW_WAIT_HPP_
