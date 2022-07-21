#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]
include!(concat!(env!("OUT_DIR"), "/bindings.rs"));

use std::ffi::CStr;

// HACK: &'static str para acalmar o compilador
// TODO: apurar futuramente se o lifetime é verdade
#[derive(Debug)]
#[allow(dead_code)]
struct RustQuad {
    cmd: &'static str,
    arg1: &'static str,
    arg2: &'static str,
    arg3: &'static str,
}

// TODO: implementar
#[allow(dead_code)]
struct RustAsm {
    inner: i32,
}

// TODO: implementar
#[allow(dead_code)]
struct RustBin {
    inner: i32,
}

impl From<Quad> for RustQuad {
    fn from(q: Quad) -> Self {
        if q.is_null() {
            panic!();
        }

        // SEGURANÇA: Não há como haver uma referência mutável em outro lugar
        // portanto é seguro obter uma referência da quadrupla original.
        // Além disso o ponteiro sempre está alinhado, por ter sido criado
        // durante `make_code()` no arquvio `code.c`
        let q = unsafe { q.as_ref().unwrap() };

        // SEGURANÇA: As mesmas regras acima se aplicam
        RustQuad {
            cmd: unsafe { CStr::from_ptr(q.command).to_str().unwrap().clone() },
            arg1: unsafe { CStr::from_ptr(q.arg1).to_str().unwrap().clone() },
            arg2: unsafe { CStr::from_ptr(q.arg2).to_str().unwrap().clone() },
            arg3: unsafe { CStr::from_ptr(q.arg3).to_str().unwrap().clone() },
        }
    }
}
trait FromQuad {
    fn from_quad(quad: Quad) -> Self;
}

impl<T> FromQuad for Vec<T>
where
    T: From<Quad>,
{
    fn from_quad(quad: Quad) -> Vec<T> {
        let mut quad = quad;
        let mut vec = Vec::new();

        while !quad.is_null() {
            vec.push(T::from(quad));

            // SEGURANÇA: Há pelo menos um elemento para quadruplas,
            // logo há pelo menos um `q.next`, então o ponteiro é verificado
            // como nulo sempre, além de estar alinhado há que foi criado
            // corretamente durante `make_code()`
            quad = unsafe { quad.as_ref().unwrap().next };
        }

        // `vec` não vai mudar de tamanho
        // então é possível liberar espaço que não vai ser usado
        vec.shrink_to_fit();

        vec
    }
}

#[allow(dead_code)]
fn make_assembly(quad: Vec<RustQuad>) -> Vec<RustAsm> {
    // SEGURANÇA: No momento temos acesso único a variável `TraceCode`
    // justamente pelo código ser _single-thread_
    unsafe {
        if TraceCode == 1 {
            // TODO: usar `listing`
            println!("\nGerando código assembly\n");
        }
    }

    let _ = quad;

    // SEGURANÇA: No momento temos acesso único a variável `TraceCode`
    // justamente pelo código ser _single-thread_
    unsafe {
        if TraceCode == 1 {
            // TODO: usar `listing`
            println!("\nGeração do código assembly concluída.");
        }
    };

    let asm = RustAsm { inner: 0 };

    let mut vec = Vec::new();
    vec.push(asm);

    vec
}

#[allow(dead_code)]
fn make_binary(asm: Vec<RustAsm>) -> Vec<RustBin> {
    // SEGURANÇA: No momento temos acesso único a variável `TraceCode`
    // justamente pelo código ser _single-thread_
    unsafe {
        if TraceCode == 1 {
            // TODO: usar `listing`
            println!("\nGerando binário\n");
        }
    }

    let _ = asm;

    // SEGURANÇA: No momento temos acesso único a variável `TraceCode`
    // justamente pelo código ser _single-thread_
    unsafe {
        if TraceCode == 1 {
            // TODO: usar `listing`
            println!("\nGeração do binário concluída.");
        }
    };

    let bin = RustBin { inner: 0 };

    let mut vec = Vec::new();
    vec.push(bin);

    vec
}

#[no_mangle]
pub extern "C" fn make_output(quad: Quad) -> libc::c_int {
    let quad_vec = Vec::<RustQuad>::from_quad(quad);

    let asm_vec = make_assembly(quad_vec);

    let _ = make_binary(asm_vec);

    0
}
