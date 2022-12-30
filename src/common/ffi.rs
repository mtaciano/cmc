use ::libc::{c_char, c_int, FILE};
use ::std::ffi::CString;

extern "C" {
    /// A variável `g_trace_code` faz com que informações adicionais sejam printadas
    /// durante a fase de geração do código intermediário
    pub(crate) static mut g_trace_code: c_int;

    /// Descritor de _output_ para saída padrão
    pub(crate) static mut std_fd: *mut FILE;

    /// A variável `g_slot_start` serve para armazenar o começo do slot
    /// que vai ser usado durante o processo de compilação,
    /// assim mudando fatores como local na memória de dados e de instruções
    // NOTE: para facilitar a implementação, é assumido que o tamanho da memória
    // de dados é o mesmo que o da memória de instruções, assim a posição
    // dos dados de cada programa na memória vai ter um _offset_ igual
    // o da de instruções
    pub(crate) static mut g_slot_start: c_int;

    /// A variável `g_slot_end` serve para armazenar o fim do slot (não inclusivo)
    /// que vai ser usado durante o processo de compilação.
    #[allow(dead_code)]
    // TODO: verificar se tem uso
    pub(crate) static mut g_slot_end: c_int;
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
