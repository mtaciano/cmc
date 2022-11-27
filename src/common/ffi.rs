use ::libc::{c_char, c_int, FILE};
use ::std::ffi::CString;

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

/// Função para usar `fprintf` em Rust
pub(crate) fn print_stream(stream: *mut libc::FILE, string: &str) {
    let cstr = CString::new(string).unwrap();
    let ret = unsafe { libc::fprintf(stream, cstr.as_ptr()) };

    if ret < 0 {
        panic!("Erro durante print_stream()");
    }
}
