use ::libc::{c_char, c_int, FILE};
use ::std::ffi::CString;

extern "C" {
    /// A variável `g_trace_code` faz com que informações adicionais sejam printadas
    /// durante a fase de geração do código intermediário
    pub(crate) static mut g_trace_code: c_int;

    /// Descritor de _output_ para saída padrão
    pub(crate) static mut std_fd: *mut FILE;

    /// A variável `g_mem_start` serve para armazenar o começo da memória
    /// de dados do programa a ser compilado.
    pub(crate) static mut g_mem_start: c_int;

    /// A variável `g_mem_slot_end` serve para armazenar o fim da memória (não
    /// inclusiva) de dados que vai ser usada durante o processo de compilação.
    pub(crate) static mut g_mem_end: c_int;

    /// A variável `g_inst_start` serve para armazenar o começo da memória
    /// de instruções do programa a ser compilado.
    pub(crate) static mut g_inst_start: c_int;

    /// A variável `g_inst_end` serve para armazenar o fim da memória (não
    /// inclusivo) de instruções que vai ser usada durante o processo de compilação.
    pub(crate) static mut g_inst_end: c_int;
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
pub(crate) unsafe fn print_stream(stream: *mut FILE, string: &str) {
    let cstr = CString::new(string).unwrap();
    let ret = unsafe { libc::fprintf(stream, cstr.as_ptr()) };

    if ret < 0 {
        panic!("Erro durante print_stream().");
    }
}
