#ifndef __NODE_GDAL_TYPED_ARRAY_CONTENTS_H__
#define __NODE_GDAL_TYPED_ARRAY_CONTENTS_H__

#include <cassert>
#include <cstddef>
#include <cstdint>

// nan
#include "../nan-wrapper.h"

namespace node_gdal {

// Drop-in replacement for Nan::TypedArrayContents<T>.
//
// Nan::TypedArrayContents obtains the data pointer through
// v8::ArrayBuffer::GetBackingStore(), which returns a std::shared_ptr.
// Electron's Windows binaries are built with clang/libc++, so that symbol is
// exported with a libc++ mangled name and an MSVC-built addon fails to link
// against it (LNK2001: unresolved external GetBackingStore).
// v8::ArrayBuffer::Data() returns a plain void* and has no C++ standard
// library types in its signature, so it links with any toolchain.
template <typename T> class TypedArrayContents {
    public:
  inline explicit TypedArrayContents(v8::Local<v8::Value> from) : length_(0), data_(nullptr) {
#if defined(V8_MAJOR_VERSION) && V8_MAJOR_VERSION >= 13
    Nan::HandleScope scope;
    void *data = nullptr;
    size_t length = 0;
    if (from->IsArrayBufferView()) {
      v8::Local<v8::ArrayBufferView> array = from.As<v8::ArrayBufferView>();
      length = array->ByteLength() / sizeof(T);
      data = static_cast<char *>(array->Buffer()->Data()) + array->ByteOffset();
    }
    assert(reinterpret_cast<uintptr_t>(data) % alignof(T) == 0);
    length_ = length;
    data_ = static_cast<T *>(data);
#else
    Nan::TypedArrayContents<T> contents(from);
    length_ = contents.length();
    data_ = *contents;
#endif
  }

  inline size_t length() const {
    return length_;
  }
  inline T *operator*() {
    return data_;
  }
  inline const T *operator*() const {
    return data_;
  }

  TypedArrayContents(const TypedArrayContents &) = delete;
  TypedArrayContents &operator=(const TypedArrayContents &) = delete;

    private:
  size_t length_;
  T *data_;
};

} // namespace node_gdal

#endif
