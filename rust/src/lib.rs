#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]
include!(concat!(env!("OUT_DIR"), "/bindings.rs"));

#[no_mangle]
pub extern "C" fn make_assembly(quad: Quad) -> i32 {
    let mut curr_quad = quad;
    while let Some(q) = unsafe { curr_quad.as_ref() } {
        let cmd = unsafe { std::ffi::CStr::from_ptr(q.command) };
        let arg1 = unsafe { std::ffi::CStr::from_ptr(q.arg1) };
        let arg2 = unsafe { std::ffi::CStr::from_ptr(q.arg2) };
        let arg3 = unsafe { std::ffi::CStr::from_ptr(q.arg3) };
        println!("{:?}, {:?}, {:?}, {:?}", cmd, arg1, arg2, arg3);
        unsafe {
            curr_quad = curr_quad.as_ref().unwrap().next;
        };
    }
    0
}

/* #[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn it_works() {
        unsafe {
            assert_eq!(make_assembly(magic), 0);
        }
    }
} */
