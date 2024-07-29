#ifndef GOOGLE_PROTOBUF_RUST_CPP_KERNEL_COMPARE_H__
#define GOOGLE_PROTOBUF_RUST_CPP_KERNEL_COMPARE_H__

#include "google/protobuf/message.h"

extern "C" {

bool proto2_rust_equals(const google::protobuf::Message* msg1,
                        const google::protobuf::Message* msg2);

}  // extern "C"

#endif  // GOOGLE_PROTOBUF_RUST_CPP_KERNEL_COMPARE_H__
