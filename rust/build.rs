extern crate bindgen;

use std::env;
use std::path::PathBuf;

fn main() {
    // Diz para o cargo invalidar a crate quando wrapper.h muda
    println!("cargo:rerun-if-changed=wrapper.h");

    // O bindgen::Builder é o ponto de entrada
    // para bindgen, e deixa construir opções para
    // os bindings resultantes.
    let bindings = bindgen::Builder::default()
        // O header para que queremos gerar headers
        .header("wrapper.h")
        .generate_comments(true)
        .allowlist_recursively(true)
        .allowlist_type("Quad")
        .allowlist_var("listing")
        .allowlist_var("TraceCode")
        // Diz para o cargo invalidar a crate quando qualquer
        // header incluído muda
        .parse_callbacks(Box::new(bindgen::CargoCallbacks))
        // Termina o Builder e gera os bindings
        .generate()
        // Caso aconteça um erro
        .expect("Não foi possível gerar as bindings!");

    // Mande as bindings para $OUT_DIR/bindings.rs
    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());
    bindings
        .write_to_file(out_path.join("bindings.rs"))
        .expect("Não foi possível escrever as bindings!");
}
