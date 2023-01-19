use std::ffi::CStr;
use std::fs::OpenOptions;
use std::io::Write;

/* Bindings com C */
mod common;
use common::ffi::{self, g_inst_start};

/* Gerador de assembly */
mod assembly;

/* Gerador de binário */
mod binary;

/* Macro para facilitar a criação de structs `Asm` */
macro_rules! asm {
    ($cmd:literal, $arg1:literal, $arg2:literal, $arg3:literal) => {
        Asm {
            cmd: $cmd.to_string(),
            arg1: $arg1.to_string(),
            arg2: $arg2.to_string(),
            arg3: $arg3.to_string(),
        }
    };
}
pub(crate) use asm;

/* Struct de quádruplas */
#[derive(Debug)]
struct Quad {
    cmd: String,
    arg1: String,
    arg2: String,
    arg3: String,
}

/* Struct para o Assembly */
#[derive(Debug, Clone)]
struct Asm {
    cmd: String,
    arg1: String,
    arg2: String,
    arg3: String,
}

/* Struct para o Binário */
#[derive(Debug)]
struct Bin {
    word: u32,
    original_cmd: String,
}

/* Conversor de quádrupla única de C para Rust */
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
                cmd: CStr::from_ptr(q.cmd).to_str().unwrap().to_owned(),
                arg1: CStr::from_ptr(q.arg1).to_str().unwrap().to_owned(),
                arg2: CStr::from_ptr(q.arg2).to_str().unwrap().to_owned(),
                arg3: CStr::from_ptr(q.arg3).to_str().unwrap().to_owned(),
            }
        }
    }
}

/* Conversor de lista encadeada para vetor de quádruplas */
trait FromQuad {
    fn from_quad(quad: ffi::Quad) -> Self;
}

impl<Q> FromQuad for Vec<Q>
where
    Q: From<ffi::Quad>,
{
    fn from_quad(mut quad: ffi::Quad) -> Vec<Q> {
        let mut quadruple = Vec::new();

        while !quad.is_null() {
            quadruple.push(Q::from(quad));

            // SEGURANÇA: Há pelo menos um elemento para quadruplas,
            // logo há pelo menos um `q.next`, então o ponteiro é verificado
            // como nulo sempre, além de estar alinhado já que foi criado
            // corretamente durante `make_intermediate()`
            unsafe {
                quad = quad.as_ref().unwrap().next;
            };
        }

        // `vec` não vai mudar de tamanho
        // então é possível liberar espaço que não vai ser usado
        quadruple.shrink_to_fit();

        quadruple
    }
}

/* Função `make_assembly_and_binary` é responsável por criar
 * o assembly e binário para o compilador
*/
#[no_mangle]
pub extern "C" fn make_assembly_and_binary(quad: ffi::Quad) {
    let slot = unsafe { ffi::g_inst_start / (ffi::g_inst_end - ffi::g_inst_start) };
    let file_name = match slot {
        0 => "sistema_operacional".to_string(),
        num @ 1..=3 => format!("programa{num}"),
        _ => unreachable!(),
    };

    let quads = Vec::<Quad>::from_quad(quad);
    let asm = crate::assembly::make_assembly(quads);
    let bin = crate::binary::make_binary(asm);

    unsafe {
        let success = format!("\nCriando arquivo {file_name}.txt\n");
        ffi::print_stream(ffi::std_fd, &success);
    }

    let mut binary_file = OpenOptions::new()
        .create(true)
        .write(true)
        .truncate(true)
        .open(format!("build/{file_name}.txt"))
        .expect("Não foi possível criar um arquivo de saída para o binário.");

    for bin in bin.iter() {
        writeln!(
            &mut binary_file,
            "{:032b} // {}",
            bin.word, bin.original_cmd
        )
        .expect("Erro escrevendo binário em arquivo.");
    }

    unsafe {
        let success = format!("\nArquivo {file_name}.txt criado.\n");
        ffi::print_stream(ffi::std_fd, &success);
        let success = format!("\nCriando arquivo verilog_{file_name}.txt\n");
        ffi::print_stream(ffi::std_fd, &success);
    }

    let mut verilog_file = OpenOptions::new()
        .create(true)
        .write(true)
        .truncate(true)
        .open(format!("build/verilog_{file_name}.txt"))
        .expect("Não foi possível criar um arquivo de saída para o binário.");

    unsafe {
        for (i, bin) in bin.iter().enumerate() {
            writeln!(
                &mut verilog_file,
                "memory[{}] = 32'b{:032b}; // {}",
                i + g_inst_start as usize,
                bin.word,
                bin.original_cmd
            )
            .expect("Erro escrevendo binário em arquivo.");
        }
    }

    unsafe {
        let success = format!("\nArquivo verilog_{file_name}.txt criado.\n");
        ffi::print_stream(ffi::std_fd, &success);
    }
}
