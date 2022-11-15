use ::core::ffi::c_char;
use ::core::ffi::c_int;

/* A variável `g_trace_code` faz com que informações adicionais sejam printadas
 * durante a fase de geração do código intermediário
 */
extern "C" {
    pub(crate) static mut g_trace_code: c_int;
}

/* Struct de Quádruplas */
#[repr(C)]
pub struct QuadRec {
    pub cmd: *mut c_char,
    pub arg1: *mut c_char,
    pub arg2: *mut c_char,
    pub arg3: *mut c_char,
    pub next: *mut QuadRec,
}
pub type Quad = *mut QuadRec;
