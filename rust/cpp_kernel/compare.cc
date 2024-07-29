#include "rust/cpp_kernel/compare.h"

#include "google/protobuf/message.h"
#include "google/protobuf/util/message_differencer.h"

extern "C" {

bool proto2_rust_equals(const google::protobuf::Message* msg1,
                        const google::protobuf::Message* msg2) {
  return google::protobuf::util::MessageDifferencer::Equals(*msg1, *msg2);
}

}  // extern "C"
