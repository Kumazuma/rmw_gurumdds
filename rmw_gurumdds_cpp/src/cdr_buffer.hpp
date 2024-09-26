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

#ifndef CDR_BUFFER_HPP_
#define CDR_BUFFER_HPP_

#include <cstring>
#include <string>
#include <stdexcept>
#include <limits>

#include "rosidl_runtime_c/string.h"
#include "rosidl_runtime_c/string_functions.h"
#include "rosidl_runtime_c/u16string.h"
#include "rosidl_runtime_c/u16string_functions.h"

#define CDR_BIG_ENDIAN 0
#define CDR_LITTLE_ENDIAN 1

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define system_endian CDR_LITTLE_ENDIAN
#else
#define system_endian CDR_BIG_ENDIAN
#endif

#define CDR_HEADER_SIZE 4
#define CDR_HEADER_ENDIAN_IDX 1
namespace cdr
{
class Buffer {
public:
  Buffer(uint8_t * buf, size_t size);

  size_t get_offset() const;

  void roundup(uint32_t align);

protected:

  void advance(size_t cnt);

  uint8_t * buf_;
  size_t offset_;
  size_t size_;
};

template<bool SERIALIZE>
class SerializationBuffer: public Buffer {
public:
  SerializationBuffer(uint8_t * buf, size_t size);

  void operator<<(uint8_t src);

  void operator<<(uint16_t src);

  void operator<<(uint32_t src);

  void operator<<(uint64_t src);

  void operator<<(const std::string & src);

  void operator<<(const std::u16string & src);

  void operator<<(const rosidl_runtime_c__String & src);

  void operator<<(const rosidl_runtime_c__U16String & src);

  void copy_arr(const uint8_t * arr, size_t cnt);

  void copy_arr(const uint16_t * arr, size_t cnt);

  void copy_arr(const uint32_t * arr, size_t cnt);

  void copy_arr(const uint64_t * arr, size_t cnt);
};

class DeserializationBuffer: public Buffer {
public:
  DeserializationBuffer(uint8_t * buf, size_t size);

  void operator>>(uint8_t & dst);

  void operator>>(uint16_t & dst);

  void operator>>(uint32_t & dst);

  void operator>>(uint64_t & dst);

  void operator>>(std::string & dst);

  void operator>>(std::u16string & dst);

  void operator>>(rosidl_runtime_c__String & dst);

  void operator>>(rosidl_runtime_c__U16String & dst);

  void copy_arr(uint8_t * arr, size_t cnt);

  void copy_arr(uint16_t * arr, size_t cnt);

  void copy_arr(uint32_t * arr, size_t cnt);

  void copy_arr(uint64_t * arr, size_t cnt);

private:
  bool swap_;
};
}

#include "cdr_serialization_buffer.inl"

#endif  // CDR_BUFFER_HPP_
