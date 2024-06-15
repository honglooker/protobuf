use crate::{upb_MiniTable, RawMessage};

extern "C" {
    /// SAFETY: No constraints.
    pub fn upb_TextEncode_Debug(
        msg: RawMessage,
        mt: *const upb_MiniTable,
        options: i32,
        buf: *mut u8,
        size: usize,
    ) -> usize;
}
