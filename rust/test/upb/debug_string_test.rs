extern crate protobuf_upb as __pb;
use googletest::prelude::*;
use map_unittest_rust_proto::TestMapWithMessages;
use unittest_rust_proto::{
    test_all_types::NestedEnum as NestedEnumProto2,
    test_all_types::NestedMessage as NestedMessageProto2, TestAllTypes as TestAllTypesProto2,
};

#[test]
fn test_debug_string() {
    let mut msg = __pb::proto!(TestAllTypesProto2 {
        optional_int32: 42,
        optional_string: "Hello World",
        optional_nested_enum: NestedEnumProto2::Bar,
        oneof_uint32: 452235,
        optional_nested_message: __pb::proto!(NestedMessageProto2 { bb: 100 }),
    });
    let mut repeated_string = msg.repeated_string_mut();
    repeated_string.push("Hello World".into());
    repeated_string.push("Hello World".into());
    repeated_string.push("Hello World".into());

    let mut msg_map = TestMapWithMessages::new();
    msg_map.map_string_all_types_mut().insert("hello", msg.as_view());
    msg_map.map_string_all_types_mut().insert("fizz", msg.as_view());
    msg_map.map_string_all_types_mut().insert("boo", msg.as_view());

    println!("{:?}", msg_map);
    println!("{:?}", msg_map.as_view());
    println!("{:?}", msg_map.as_mut());
    let golden = "12 {\n  key: \"hello\"\n  value {\n    1: 42\n    14: \"Hello World\"\n    18 {\n      1: 100\n    }\n    21: 2\n    44: \"Hello World\"\n    44: \"Hello World\"\n    44: \"Hello World\"\n    111: 452235\n  }\n}\n12 {\n  key: \"fizz\"\n  value {\n    1: 42\n    14: \"Hello World\"\n    18 {\n      1: 100\n    }\n    21: 2\n    44: \"Hello World\"\n    44: \"Hello World\"\n    44: \"Hello World\"\n    111: 452235\n  }\n}\n12 {\n  key: \"boo\"\n  value {\n    1: 42\n    14: \"Hello World\"\n    18 {\n      1: 100\n    }\n    21: 2\n    44: \"Hello World\"\n    44: \"Hello World\"\n    44: \"Hello World\"\n    111: 452235\n  }\n}\n";
    assert_that!(format!("{:?}", msg_map), eq(golden));
}
