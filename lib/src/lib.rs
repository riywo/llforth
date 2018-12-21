extern crate rustyline;
extern crate atty;
extern crate libc;

use libc::c_char;
use std::ffi::{CString, CStr};
use std::slice;
use std::mem::transmute;

mod reader;
use reader::{Reader, Input};

//#[no_mangle]
//pub extern fn set_file(file_name_ptr: *const c_char) {
//    let file_name = unsafe { CStr::from_ptr(file_name_ptr).to_str().unwrap() };
//    READER.lock().unwrap().set_file(file_name);
//}

#[no_mangle]
pub extern fn create_reader() -> *mut Reader {
    let _reader = unsafe { transmute(Box::new(Reader::new())) };
    return _reader;
}

#[no_mangle]
pub extern fn read_word_from_reader(ptr: *mut Reader, inbuf_ptr: *mut c_char, max_len: i64) -> i64 {
    let mut _reader = unsafe { &mut *ptr };
    let inbuf = unsafe { slice::from_raw_parts_mut(inbuf_ptr, max_len as usize) };
    let input = _reader.read();
    match input {
        Input::Word(word) | Input::Quote(word) => {
            let len = word.len();
            let c_word = CString::new(word).unwrap();
            let c_word_bytes = c_word.as_bytes_with_nul();
            let c_word_bytes = unsafe { &*(c_word_bytes as *const [u8] as *const [i8]) };
            inbuf[..=len].clone_from_slice(c_word_bytes);
            return len as i64;
        },
        Input::Interrupted | Input::Eof => {
            inbuf[0] = -1;
            return 0;
        },
        Input::Newline => {
            inbuf[0] = 10;
            return 0;
        }
    }
}

#[no_mangle]
pub extern fn destroy_reader(ptr: *mut Reader) {
    let _reader: Box<Reader> = unsafe { transmute(ptr) };
}
