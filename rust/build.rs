extern crate bindgen;

use std::env;
use std::path::PathBuf;

fn main() {
    let path = env::current_dir().unwrap().pop();
    // Diz para o cargo olhar por bibliotecas nestes caminho
    println!("cargo:rustc-link-search={}", path.to_string());

    // Tell cargo to tell rustc to link the system bzip2
    // shared library.
    // println!("cargo:rustc-link-lib=bz2");

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
        // Diz para o cargo invalidar a crate quando qualquer
        // header incluído muda
        .parse_callbacks(Box::new(bindgen::CargoCallbacks))
        // Termina o Builder e gera os bindings
        .generate()
        // Caso aconteça um erro
        .expect("Não foi possível geraar as bindings");

    // Mande as bindings para $OUT_DIR/bindings.rs
    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());
    bindings
        .write_to_file(out_path.join("bindings.rs"))
        .expect("Não foi possível escrever as bindings!");
}
