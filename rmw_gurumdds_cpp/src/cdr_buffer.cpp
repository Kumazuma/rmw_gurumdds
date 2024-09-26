#include <cassert>
#include "cdr_buffer.hpp"

namespace cdr {
Buffer::Buffer(uint8_t * buf, size_t size)
  : buf_{buf}
  , offset_{}
  , size_{size} {

}

size_t Buffer::get_offset() const {
  return offset_;
}

void Buffer::roundup(uint32_t align) {
  assert(align != 0);
  size_t count = -offset_ & (align - 1);
  if (offset_ + count > size_) {
    throw std::runtime_error("Out of buffer");
  }

  advance(count);
}

void Buffer::advance(size_t cnt) {
  offset_ += cnt;
}
}
