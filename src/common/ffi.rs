use ::libc::c_char;
use ::libc::c_int;
use ::libc::FILE;

extern "C" {
    /// A variável `g_trace_code` faz com que informações adicionais sejam printadas
    /// durante a fase de geração do código intermediário
    pub(crate) static mut g_trace_code: c_int;

    /// Descritor de _output_ para saída padrão
    pub(crate) static mut listing: *mut FILE;
}

/// Lista encadeada de quádruplas
#[repr(C)]
pub struct QuadRec {
    pub cmd: *mut c_char,
    pub arg1: *mut c_char,
    pub arg2: *mut c_char,
    pub arg3: *mut c_char,
    pub next: *mut QuadRec,
}
pub type Quad = *mut QuadRec;
