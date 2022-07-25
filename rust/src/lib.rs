#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]
include!(concat!(env!("OUT_DIR"), "/bindings.rs"));

use std::{ffi::CStr, fs::OpenOptions, io::Write};

mod assembly;
mod binary;

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
            cmd: unsafe {
                CStr::from_ptr(q.command).to_str().unwrap().to_owned()
            },
            arg1: unsafe {
                CStr::from_ptr(q.arg1).to_str().unwrap().to_owned()
            },
            arg2: unsafe {
                CStr::from_ptr(q.arg2).to_str().unwrap().to_owned()
            },
            arg3: unsafe {
                CStr::from_ptr(q.arg3).to_str().unwrap().to_owned()
            },
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

#[derive(Debug)]
struct RustQuad {
    cmd: String,
    arg1: String,
    arg2: String,
    arg3: String,
}

#[derive(Debug, Clone)]
struct RustAsm {
    cmd: String,
    arg1: String,
    arg2: String,
    arg3: String,
}

#[derive(Debug)]
struct RustBin {
    inner: u32,
}

#[no_mangle]
pub extern "C" fn make_output(quad: Quad) {
    let quad_vec = Vec::<RustQuad>::from_quad(quad);

    let asm_vec = crate::assembly::make_assembly(quad_vec);

    let bin_vec = crate::binary::make_binary(asm_vec);

    let mut file = OpenOptions::new()
        .create(true)
        .write(true)
        .truncate(true)
        .open("out_bin.txt")
        .expect("Não foi possível criar um arquivo de saída para o binário.");

    for bin in bin_vec.iter() {
        writeln!(&mut file, "{:032b}", bin.inner)
            .expect("Erro escrevendo binário em arquivo.");
    }
}
