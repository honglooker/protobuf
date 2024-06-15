# Copyright (c) 2009-2021, Google LLC
# All rights reserved.
#
# Use of this source code is governed by a BSD-style
# license that can be found in the LICENSE file or at
# https://developers.google.com/open-source/licenses/bsd

load("//upb/bazel:build_defs.bzl", "UPB_DEFAULT_COPTS")

cc_library(
    name = "text",
    srcs = [
        "encode.c",
    ],
    hdrs = [
        "encode.h",
    ],
    copts = UPB_DEFAULT_COPTS,
    visibility = ["//visibility:public"],
    deps = [
        ":text_internal",
        "//upb:base",
        "//upb:eps_copy_input_stream",
        "//upb:message",
        "//upb:port",
        "//upb:reflection",
        "//upb/lex",
        "//upb/message:internal",
        "//upb/message:types",
    ],
)

cc_library(
    name = "text_debug",
    srcs = [
        "encode_debug.c",
    ],
    hdrs = [
        "encode_debug.h",
    ],
    copts = UPB_DEFAULT_COPTS,
    visibility = ["//visibility:public"],
    deps = [
        ":text_internal",
        "//upb:base",
        "//upb:eps_copy_input_stream",
        "//upb:message",
        "//upb:mini_table",
        "//upb:port",
        "//upb/lex",
        "//upb/message:internal",
        "//upb/message:iterator",
        "//upb/message:types",
        "//upb/mini_table:internal",
    ],
)

cc_library(
    name = "text_internal",
    srcs = [
        "encode_internal.c",
    ],
    hdrs = [
        "encode_internal.h",
    ],
    copts = UPB_DEFAULT_COPTS,
    visibility = ["//visibility:public"],
    deps = [
        "//third_party/utf8_range",
        "//upb:base",
        "//upb:eps_copy_input_stream",
        "//upb:port",
        "//upb:reflection",
        "//upb:wire_reader",
        "//upb/message:internal",
    ],
)

cc_test(
    name = "encode_debug_test",
    srcs = [
        "encode_debug_test.cc",
        "encode_internal.c",
    ],
    deps = [
        ":text_debug",
        ":text_internal",
        "//upb:base",
        "//upb:eps_copy_input_stream",
        "//upb:mem",
        "//upb:message",
        "//upb:mini_table",
        "//upb:port",
        "//upb:wire_reader",
        "//upb/message:internal",
        "//upb/test:test_proto_upb_minitable",
        "//upb/test:test_upb_proto",
        "@com_google_absl//absl/log:absl_log",
        "@com_google_googletest//:gtest",
        "@com_google_googletest//:gtest_main",
    ],
)

# begin:github_only
filegroup(
    name = "source_files",
    srcs = glob(
        [
            "**/*.c",
            "**/*.h",
        ],
    ),
    visibility = [
        "//python/dist:__pkg__",
        "//upb/cmake:__pkg__",
    ],
)
# end:github_only
