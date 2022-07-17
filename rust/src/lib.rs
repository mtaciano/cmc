#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]
include!(concat!(env!("OUT_DIR"), "/bindings.rs"));

use std::ffi::CStr;

#[derive(Debug)]
pub struct RustQuad {
    pub cmd: &'static str,
    pub arg1: &'static str,
    pub arg2: &'static str,
    pub arg3: &'static str,
}

pub struct VecRustQuad {
    pub inner: Vec<RustQuad>,
}

impl VecRustQuad {
    fn from_quad(mut quad: Quad) -> Self {
        let mut inner = Vec::new();

        // Traverse the linked list until the end
        while let Some(q) = unsafe { quad.as_ref() } {
            let cmd = unsafe { CStr::from_ptr(q.command).to_str().unwrap() };
            let arg1 = unsafe { CStr::from_ptr(q.arg1).to_str().unwrap() };
            let arg2 = unsafe { CStr::from_ptr(q.arg2).to_str().unwrap() };
            let arg3 = unsafe { CStr::from_ptr(q.arg3).to_str().unwrap() };

            inner.push(RustQuad {
                cmd,
                arg1,
                arg2,
                arg3,
            });

            unsafe {
                quad = quad.as_ref().unwrap().next;
            };
        }

        VecRustQuad { inner }
    }
}

#[no_mangle]
pub extern "C" fn make_output(quad: Quad) -> i32 {
    let rquad = VecRustQuad::from_quad(quad);
    for rq in rquad.inner.into_iter() {
        println!("{:?}", rq);
    }
    make_assembly();
    make_binary();
    0
}

fn make_assembly() {
    todo!()
}

fn make_binary() {
    todo!()
}
