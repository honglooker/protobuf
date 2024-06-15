// Protocol Buffers - Google's data interchange format
// Copyright 2023 Google LLC.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

#include "upb/text/encode_debug.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "upb/base/descriptor_constants.h"
#include "upb/base/string_view.h"
#include "upb/lex/round_trip.h"
#include "upb/message/array.h"
#include "upb/message/internal/iterator.h"
#include "upb/message/internal/map_entry.h"
#include "upb/message/internal/map_sorter.h"
#include "upb/message/map.h"
#include "upb/message/message.h"
#include "upb/message/value.h"
#include "upb/mini_table/extension.h"
#include "upb/mini_table/field.h"
#include "upb/mini_table/internal/field.h"
#include "upb/mini_table/internal/message.h"
#include "upb/mini_table/message.h"
#include "upb/text/encode_internal.h"
#include "upb/wire/eps_copy_input_stream.h"

// Must be last.
#include "upb/port/def.inc"

static void txtenc_msg(txtenc* e, const upb_Message* msg,
                       const upb_MiniTable* mt);

static void txtenc_field(txtenc* e, upb_MessageValue val,
                         const upb_MiniTableField* f, const upb_MiniTable* mt,
                         const char* optional) {
  // optional is to pass down whether we're dealing with a "key" of a map or
  // a "value" of a map.

  UPB_PRIVATE(txtenc_indent)(e);
  const upb_CType ctype = upb_MiniTableField_CType(f);
  const bool is_ext = upb_MiniTableField_IsExtension(f);
  char number[10];  // A 32-bit integer can hold up to 10 digits.
  sprintf(number, "%" PRIu32, upb_MiniTableField_Number(f));

  if (ctype == kUpb_CType_Message) {
    if (is_ext) {
      // when we have a map we want to print out either "key" or "value", not a
      // field number
      if (optional != NULL) {
        UPB_PRIVATE(txtenc_printf)(e, "[%s] {", optional);
      } else {
        UPB_PRIVATE(txtenc_printf)(e, "[%s] {", number);
      }
    } else {
      if (optional != NULL) {
        UPB_PRIVATE(txtenc_printf)(e, "%s {", optional);
      } else {
        UPB_PRIVATE(txtenc_printf)(e, "%s {", number);
      }
    }

    UPB_PRIVATE(txtenc_endfield)(e);
    e->indent_depth++;
    txtenc_msg(e, val.msg_val, upb_MiniTable_SubMessage(mt, f));
    e->indent_depth--;
    UPB_PRIVATE(txtenc_indent)(e);
    UPB_PRIVATE(txtenc_putstr)(e, "}");
    UPB_PRIVATE(txtenc_endfield)(e);
    return;
  }

  if (is_ext) {
    if (optional != NULL) {
      UPB_PRIVATE(txtenc_printf)(e, "[%s]: ", optional);
    } else {
      UPB_PRIVATE(txtenc_printf)(e, "[%s]: ", number);
    }
  } else {
    if (optional != NULL) {
      UPB_PRIVATE(txtenc_printf)(e, "%s: ", optional);
    } else {
      UPB_PRIVATE(txtenc_printf)(e, "%s: ", number);
    }
  }

  switch (ctype) {
    case kUpb_CType_Bool:
      UPB_PRIVATE(txtenc_putstr)(e, val.bool_val ? "true" : "false");
      break;
    case kUpb_CType_Float: {
      char buf[32];
      _upb_EncodeRoundTripFloat(val.float_val, buf, sizeof(buf));
      UPB_PRIVATE(txtenc_putstr)(e, buf);
      break;
    }
    case kUpb_CType_Double: {
      char buf[32];
      _upb_EncodeRoundTripDouble(val.double_val, buf, sizeof(buf));
      UPB_PRIVATE(txtenc_putstr)(e, buf);
      break;
    }
    case kUpb_CType_Int32:
      UPB_PRIVATE(txtenc_printf)(e, "%" PRId32, val.int32_val);
      break;
    case kUpb_CType_UInt32:
      UPB_PRIVATE(txtenc_printf)(e, "%" PRIu32, val.uint32_val);
      break;
    case kUpb_CType_Int64:
      UPB_PRIVATE(txtenc_printf)(e, "%" PRId64, val.int64_val);
      break;
    case kUpb_CType_UInt64:
      UPB_PRIVATE(txtenc_printf)(e, "%" PRIu64, val.uint64_val);
      break;
    case kUpb_CType_String:
      UPB_PRIVATE(upb_HardenedPrintString)
      (e, val.str_val.data, val.str_val.size);
      break;
    case kUpb_CType_Bytes:
      UPB_PRIVATE(txtenc_bytes)(e, val.str_val);
      break;
    case kUpb_CType_Enum:
      UPB_PRIVATE(txtenc_printf)(e, "%" PRId32, val.int32_val);
      break;
    default:
      UPB_UNREACHABLE();
  }

  UPB_PRIVATE(txtenc_endfield)(e);
}

/*
 * Arrays print as simple repeated elements, eg.
 *
 *    5: 1
 *    5: 2
 *    5: 3
 */
static void txtenc_array(txtenc* e, const upb_Array* arr,
                         const upb_MiniTableField* f, const upb_MiniTable* mt) {
  size_t i;
  size_t size = upb_Array_Size(arr);

  for (i = 0; i < size; i++) {
    txtenc_field(e, upb_Array_Get(arr, i), f, mt, NULL);
  }
}

static void txtenc_mapentry(txtenc* e, upb_MessageValue key,
                            upb_MessageValue val, const upb_MiniTableField* f,
                            const upb_MiniTable* mt) {
  const upb_MiniTable* entry = upb_MiniTable_SubMessage(mt, f);
  const upb_MiniTableField* key_f = upb_MiniTable_MapKey(entry);
  const upb_MiniTableField* val_f = upb_MiniTable_MapValue(entry);

  UPB_PRIVATE(txtenc_indent)(e);
  UPB_PRIVATE(txtenc_printf)(e, "%u {", upb_MiniTableField_Number(f));
  UPB_PRIVATE(txtenc_endfield)(e);
  e->indent_depth++;

  txtenc_field(e, key, key_f, entry, "key");
  txtenc_field(e, val, val_f, entry, "value");

  e->indent_depth--;
  UPB_PRIVATE(txtenc_indent)(e);
  UPB_PRIVATE(txtenc_putstr)(e, "}");
  UPB_PRIVATE(txtenc_endfield)(e);
}

/*
 * Maps print as messages of key/value, etc.
 *
 *    1 {
 *      key: "abc"
 *      value: 123
 *    }
 *    2 {
 *      key: "def"
 *      value: 456
 *    }
 */
static void txtenc_map(txtenc* e, const upb_Map* map,
                       const upb_MiniTableField* f, const upb_MiniTable* mt) {
  if (e->options & UPB_TXTENC_NOSORT) {
    size_t iter = kUpb_Map_Begin;
    upb_MessageValue key, val;
    while (upb_Map_Next(map, &key, &val, &iter)) {
      txtenc_mapentry(e, key, val, f, mt);
    }
  } else {
    if (upb_Map_Size(map) == 0) return;

    const upb_MiniTable* entry = upb_MiniTable_SubMessage(mt, f);
    const upb_MiniTableField* key_f = upb_MiniTable_GetFieldByIndex(entry, 0);
    _upb_sortedmap sorted;
    upb_MapEntry ent;

    _upb_mapsorter_pushmap(&e->sorter, upb_MiniTableField_Type(key_f), map,
                           &sorted);
    while (_upb_sortedmap_next(&e->sorter, map, &sorted, &ent)) {
      upb_MessageValue key, val;
      memcpy(&key, &ent.k, sizeof(key));
      memcpy(&val, &ent.v, sizeof(val));
      txtenc_mapentry(e, key, val, f, mt);
    }
    _upb_mapsorter_popmap(&e->sorter, &sorted);
  }
}

static void txtenc_msg(txtenc* e, const upb_Message* msg,
                       const upb_MiniTable* mt) {
  size_t iter = kUpb_BaseField_Begin;
  const upb_MiniTableField* f;
  upb_MessageValue val;

  // Base fields will be printed out first, followed by extension fields, and
  // finally unknown fields.

  while (UPB_PRIVATE(_upb_Message_NextBaseField)(msg, mt, &f, &val, &iter)) {
    if (upb_MiniTableField_IsMap(f)) {
      txtenc_map(e, val.map_val, f, mt);
    } else if (upb_MiniTableField_IsArray(f)) {
      txtenc_array(e, val.array_val, f, mt);
    } else {
      txtenc_field(e, val, f, mt, NULL);
    }
  }

  const upb_MiniTableExtension* ext;
  upb_MessageValue val_ext;
  iter = kUpb_Extension_Begin;
  while (
      UPB_PRIVATE(_upb_Message_NextExtension)(msg, mt, &ext, &val_ext, &iter)) {
    const upb_MiniTableField* f = &ext->UPB_PRIVATE(field);
    if (upb_MiniTableField_IsMap(f)) {
      txtenc_map(e, val.map_val, f, mt);
    } else if (upb_MiniTableField_IsArray(f)) {
      txtenc_array(e, val.array_val, f, mt);
    } else {
      txtenc_field(e, val, f, mt, NULL);
    }
  }

  if ((e->options & UPB_TXTENC_SKIPUNKNOWN) == 0) {
    size_t size;
    const char* ptr = upb_Message_GetUnknown(msg, &size);
    if (size != 0) {
      char* start = e->ptr;
      upb_EpsCopyInputStream stream;
      upb_EpsCopyInputStream_Init(&stream, &ptr, size, true);
      if (!txtenc_unknown(e, ptr, &stream, -1)) {
        /* Unknown failed to parse, back up and don't print it at all. */
        e->ptr = start;
      }
    }
  }
}

size_t upb_TextEncode_Debug(const upb_Message* msg, const upb_MiniTable* mt,
                            int options, char* buf, size_t size) {
  txtenc e;

  e.buf = buf;
  e.ptr = buf;
  e.end = UPB_PTRADD(buf, size);
  e.overflow = 0;
  e.indent_depth = 0;
  e.options = options;
  e.ext_pool = NULL;
  _upb_mapsorter_init(&e.sorter);

  txtenc_msg(&e, msg, mt);
  _upb_mapsorter_destroy(&e.sorter);
  return UPB_PRIVATE(txtenc_nullz)(&e, size);
}
