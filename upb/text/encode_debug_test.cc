#include "upb/text/encode_debug.h"

#include <stddef.h>

#include <string>

#include <gtest/gtest.h>
#include "absl/log/absl_log.h"
#include "upb/base/upcast.h"
#include "upb/mem/arena.h"
#include "upb/message/message.h"
#include "upb/mini_table/message.h"
#include "upb/test/test.upb.h"
#include "upb/test/test.upb_minitable.h"

TEST(TextNoReflection, Extensions) {
  const upb_MiniTable* mt_sub = upb_0test__ModelWithSubMessages_msg_init_ptr;
  upb_Arena* arena = upb_Arena_New();
  upb_test_ModelWithSubMessages* input_msg =
      upb_test_ModelWithSubMessages_new(arena);
  upb_test_ModelWithExtensions* sub_message =
      upb_test_ModelWithExtensions_new(arena);
  upb_test_ModelWithSubMessages_set_id(input_msg, 11);
  upb_test_ModelWithExtensions_set_random_int32(sub_message, 12);
  upb_test_ModelWithSubMessages_set_optional_child(input_msg, sub_message);
  // Convert to a type of upb_Message*
  upb_Message* input = UPB_UPCAST(input_msg);
  // Resizing/reallocation of the buffer is not necessary since we're not
  // expecting a huge output out of this message.
  char* buf = new char[1024];
  int options = 4;  // Does not matter, but maps will not be sorted.
  size_t size = 1024;
  size_t real_size = upb_TextEncode_Debug(input, mt_sub, options, buf, size);
  ABSL_LOG(INFO) << "Buffer: " << buf << "\n"
                 << "Size:" << real_size << "\n";
  std::string golden = "4: 11\n5 {\n  3: 12\n}\n";
  std::string str(buf);
  ASSERT_EQ(buf, golden);
  delete[] buf;
  upb_Arena_Free(arena);
}