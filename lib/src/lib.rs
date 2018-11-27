#[macro_use]
extern crate lazy_static;
extern crate rustyline;
extern crate atty;
extern crate libc;

use std::sync::Mutex;
use libc::c_char;
use std::ffi::CString;
use std::slice;

mod prompt;
use prompt::{Prompt, Input};

lazy_static! {
    static ref PROMPT: Mutex<Prompt> = Mutex::new(Prompt::new());
}


#[no_mangle]
pub extern fn read_word(inbuf_ptr: *mut c_char, max_len: i64) -> i64 {
    let inbuf = unsafe { slice::from_raw_parts_mut(inbuf_ptr, max_len as usize) };
    let input = PROMPT.lock().unwrap().read();
    match input {
        Input::Word(word) => {
            let len = word.len();
            let c_word = CString::new(word).unwrap();
            let c_word_bytes = c_word.as_bytes_with_nul();
            let c_word_bytes = unsafe { &*(c_word_bytes as *const [u8] as *const [i8]) };
            inbuf[..=len].clone_from_slice(c_word_bytes);
            return len as i64;
        },
        Input::Interrupted | Input::Eof => return -1,
        Input::Enter => return 0,
    }
}
