#[allow(non_upper_case_globals)]
#[allow(non_camel_case_types)]
#[allow(non_snake_case)]
mod ffi {
    // Declarações para ligação entre C e Rust
    include!(concat!(env!("OUT_DIR"), "/bindings.rs"));
}

use std::ffi::CStr;
use std::fs::OpenOptions;
use std::io::Write;

// Gerador de assembly e binário
mod assembly;
mod binary;

// Conversor de quádrupla única de C para Rust
impl From<ffi::Quad> for Quad {
    fn from(quad: ffi::Quad) -> Self {
        if quad.is_null() {
            panic!("Quádrupla não existe!");
        }

        // SEGURANÇA: Não há como haver uma referência mutável em outro lugar
        // portanto é seguro obter uma referência da quadrupla original.
        // Além disso o ponteiro sempre está alinhado, por ter sido criado
        // durante `make_code()` no arquvio `code.c`
        let q = unsafe { quad.as_ref().unwrap() };

        // SEGURANÇA: As mesmas regras acima se aplicam
        unsafe {
            Quad {
                cmd: CStr::from_ptr(q.command).to_str().unwrap().to_owned(),
                arg1: CStr::from_ptr(q.arg1).to_str().unwrap().to_owned(),
                arg2: CStr::from_ptr(q.arg2).to_str().unwrap().to_owned(),
                arg3: CStr::from_ptr(q.arg3).to_str().unwrap().to_owned(),
            }
        }
    }
}

// Conversor de lista encadeada para vetor de quádruplas
trait FromQuad {
    fn from_quad(quad: ffi::Quad) -> Self;
}

impl<T> FromQuad for Vec<T>
where
    T: From<ffi::Quad>,
{
    fn from_quad(mut quad: ffi::Quad) -> Vec<T> {
        let mut vec = Vec::new();

        while !quad.is_null() {
            vec.push(T::from(quad));

            // SEGURANÇA: Há pelo menos um elemento para quadruplas,
            // logo há pelo menos um `q.next`, então o ponteiro é verificado
            // como nulo sempre, além de estar alinhado já que foi criado
            // corretamente durante `make_intermediate()`
            unsafe { quad = quad.as_ref().unwrap().next; };
        }

        // `vec` não vai mudar de tamanho
        // então é possível liberar espaço que não vai ser usado
        vec.shrink_to_fit();

        vec
    }
}

// Struct de quádruplas
#[derive(Debug)]
struct Quad {
    cmd: String,
    arg1: String,
    arg2: String,
    arg3: String,
}

// Struct para o Assembly
#[derive(Debug, Clone)]
struct Asm {
    cmd: String,
    arg1: String,
    arg2: String,
    arg3: String,
}

// Struct para o Binário
#[derive(Debug)]
struct Bin {
    inner: u32,
}

#[no_mangle]
pub extern "C" fn make_assembly_and_binary(quad: ffi::Quad) {
    let quads = Vec::<Quad>::from_quad(quad);
    let asm = crate::assembly::make_assembly(quads);
    let bin = crate::binary::make_binary(asm);

    let mut file = OpenOptions::new()
        .create(true)
        .write(true)
        .truncate(true)
        .open("build/out_bin.txt")
        .expect("Não foi possível criar um arquivo de saída para o binário.");

    for bin in bin.iter() {
        writeln!(&mut file, "{:032b}", bin.inner)
            .expect("Erro escrevendo binário em arquivo.");
    }

    println!("\nArquivo out_bin.txt criado.");
}
