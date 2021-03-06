// Copyright 2015 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tonic/dart_state.h"

#include "tonic/converter/dart_converter.h"
#include "tonic/dart_class_library.h"
#include "tonic/dart_message_handler.h"
#include "tonic/file_loader/file_loader.h"

namespace tonic {

DartState::Scope::Scope(DartState* dart_state)
    : scope_(dart_state->isolate()) {}

DartState::Scope::Scope(std::shared_ptr<DartState> dart_state)
    : scope_(dart_state->isolate()) {}

DartState::Scope::~Scope() {}

DartState::DartState(int dirfd, std::function<void(Dart_Handle)> message_epilogue)
    : isolate_(nullptr),
      class_library_(new DartClassLibrary),
      message_handler_(new DartMessageHandler()),
      file_loader_(new FileLoader(dirfd)),
      message_epilogue_(message_epilogue),
      has_set_return_code_(false) {}

DartState::~DartState() {}

void DartState::SetIsolate(Dart_Isolate isolate) {
  isolate_ = isolate;
  if (!isolate_)
    return;
  DidSetIsolate();
}

DartState* DartState::From(Dart_Isolate isolate) {
  auto isolate_data = static_cast<std::shared_ptr<DartState>*>(
      Dart_IsolateData(isolate));
  return isolate_data->get();
}

DartState* DartState::Current() {
  auto isolate_data = static_cast<std::shared_ptr<DartState>*>(
      Dart_CurrentIsolateData());
  return isolate_data->get();
}

std::weak_ptr<DartState> DartState::GetWeakPtr() {
  return shared_from_this();
}

void DartState::SetReturnCode(uint32_t return_code) {
  if (set_return_code_callback_) {
    set_return_code_callback_(return_code);
  }
  has_set_return_code_ = true;
}

void DartState::SetReturnCodeCallback(std::function<void(uint32_t)> callback) {
  set_return_code_callback_ = callback;
}

void DartState::DidSetIsolate() {}

Dart_Handle DartState::HandleLibraryTag(Dart_LibraryTag tag,
                                        Dart_Handle library,
                                        Dart_Handle url) {
  return Current()->file_loader().HandleLibraryTag(tag, library, url);
}

}  // namespace tonic
