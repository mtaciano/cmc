use std::env;
use std::path::PathBuf;

fn main() {
    // Diz para o cargo recompilar caso algo em `wrapper.h` mude
    println!("cargo:rerun-if-changed=src/wrapper.h");

    // O bindgen::Builder é o ponto de entrada
    // para bindgen, e deixa construir opções para
    // as bindings resultantes.
    let bindings = bindgen::Builder::default()
        .header("src/wrapper.h")
        .rust_target(bindgen::RustTarget::Nightly)
        .generate_comments(true)
        .allowlist_recursively(true)
        .allowlist_type("Quad")
        .allowlist_var("g_trace_code")
        .parse_callbacks(Box::new(bindgen::CargoCallbacks))
        .generate()
        .expect("Não foi possível gerar as bindings!");

    // Mande as bindings para $OUT_DIR/bindings.rs
    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());

    bindings
        .write_to_file(out_path.join("bindings.rs"))
        .expect("Não foi possível escrever as bindings!");
}
