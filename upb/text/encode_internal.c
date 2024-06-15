#include "upb/text/encode_internal.h"

#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>

#include "upb/base/string_view.h"
#include "upb/wire/eps_copy_input_stream.h"
#include "upb/wire/internal/reader.h"
#include "upb/wire/reader.h"
#include "upb/wire/types.h"

// Must be last.
#include "upb/port/def.inc"

#define CHK(x)      \
  do {              \
    if (!(x)) {     \
      return false; \
    }               \
  } while (0)

/*
 * Unknown fields are printed by number.
 *
 * 1001: 123
 * 1002: "hello"
 * 1006: 0xdeadbeef
 * 1003: {
 *   1: 111
 * }
 */
const char* txtenc_unknown(txtenc* e, const char* ptr,
                           upb_EpsCopyInputStream* stream, int groupnum) {
  // We are guaranteed that the unknown data is valid wire format, and will not
  // contain tag zero.
  uint32_t end_group = groupnum > 0
                           ? ((groupnum << kUpb_WireReader_WireTypeBits) |
                              kUpb_WireType_EndGroup)
                           : 0;

  while (!upb_EpsCopyInputStream_IsDone(stream, &ptr)) {
    uint32_t tag;
    CHK(ptr = upb_WireReader_ReadTag(ptr, &tag));
    if (tag == end_group) return ptr;

    UPB_PRIVATE(txtenc_indent)(e);
    UPB_PRIVATE(txtenc_printf)
    (e, "%d: ", (int)upb_WireReader_GetFieldNumber(tag));

    switch (upb_WireReader_GetWireType(tag)) {
      case kUpb_WireType_Varint: {
        uint64_t val;
        CHK(ptr = upb_WireReader_ReadVarint(ptr, &val));
        UPB_PRIVATE(txtenc_printf)(e, "%" PRIu64, val);
        break;
      }
      case kUpb_WireType_32Bit: {
        uint32_t val;
        ptr = upb_WireReader_ReadFixed32(ptr, &val);
        UPB_PRIVATE(txtenc_printf)(e, "0x%08" PRIu32, val);
        break;
      }
      case kUpb_WireType_64Bit: {
        uint64_t val;
        ptr = upb_WireReader_ReadFixed64(ptr, &val);
        UPB_PRIVATE(txtenc_printf)(e, "0x%016" PRIu64, val);
        break;
      }
      case kUpb_WireType_Delimited: {
        int size;
        char* start = e->ptr;
        size_t start_overflow = e->overflow;
        CHK(ptr = upb_WireReader_ReadSize(ptr, &size));
        CHK(upb_EpsCopyInputStream_CheckDataSizeAvailable(stream, ptr, size));

        // Speculatively try to parse as message.
        UPB_PRIVATE(txtenc_putstr)(e, "{");
        UPB_PRIVATE(txtenc_endfield)(e);

        // EpsCopyInputStream can't back up, so create a sub-stream for the
        // speculative parse.
        upb_EpsCopyInputStream sub_stream;
        const char* sub_ptr = upb_EpsCopyInputStream_GetAliasedPtr(stream, ptr);
        upb_EpsCopyInputStream_Init(&sub_stream, &sub_ptr, size, true);

        e->indent_depth++;
        if (txtenc_unknown(e, sub_ptr, &sub_stream, -1)) {
          ptr = upb_EpsCopyInputStream_Skip(stream, ptr, size);
          e->indent_depth--;
          UPB_PRIVATE(txtenc_indent)(e);
          UPB_PRIVATE(txtenc_putstr)(e, "}");
        } else {
          // Didn't work out, print as raw bytes.
          e->indent_depth--;
          e->ptr = start;
          e->overflow = start_overflow;
          const char* str = ptr;
          ptr = upb_EpsCopyInputStream_ReadString(stream, &str, size, NULL);
          UPB_ASSERT(ptr);
          UPB_PRIVATE(txtenc_bytes)
          (e, (upb_StringView){.data = str, .size = size});
        }
        break;
      }
      case kUpb_WireType_StartGroup:
        UPB_PRIVATE(txtenc_putstr)(e, "{");
        UPB_PRIVATE(txtenc_endfield)(e);
        e->indent_depth++;
        CHK(ptr = txtenc_unknown(e, ptr, stream,
                                 upb_WireReader_GetFieldNumber(tag)));
        e->indent_depth--;
        UPB_PRIVATE(txtenc_indent)(e);
        UPB_PRIVATE(txtenc_putstr)(e, "}");
        break;
      default:
        return NULL;
    }
    UPB_PRIVATE(txtenc_endfield)(e);
  }

  return end_group == 0 && !upb_EpsCopyInputStream_IsError(stream) ? ptr : NULL;
}

#undef CHK