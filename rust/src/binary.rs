use crate::*;

const NOP: u32 = 0b00000 << 27;
const HLT: u32 = 0b00001 << 27;
const IN: u32 = 0b00010 << 27; // TODO: fazer
const OUT: u32 = 0b00011 << 27;
const MULT: u32 = 0b01000 << 27;
const DIV: u32 = 0b01001 << 27;
const ADD: u32 = 0b01011 << 27;
const ADDI: u32 = 0b01100 << 27;
const SUB: u32 = 0b01101 << 27;
const SUBI: u32 = 0b01110 << 27;
const STORE: u32 = 0b01111 << 27;
const MOVE: u32 = 0b10000 << 27;
const LOAD: u32 = 0b10001 << 27;
const LOADI: u32 = 0b10010 << 27;
const J: u32 = 0b10011 << 27;
const JI: u32 = 0b10100 << 27;
const JZ: u32 = 0b10101 << 27;
const JN: u32 = 0b10111 << 27;
const JP: u32 = 0b11001 << 27;

// TODO: implementar futuramente
#[allow(dead_code)]
const NOT: u32 = 0b01010 << 27;
#[allow(dead_code)]
const JZI: u32 = 0b10110 << 27; // TODO: trocar ou JZI ou JNI para JP
#[allow(dead_code)]
const JNI: u32 = 0b11000 << 27; // TODO: trocar ou JZI ou JNI para JP
#[allow(dead_code)]
const AND: u32 = 0b00100 << 27;
#[allow(dead_code)]
const ANDI: u32 = 0b00101 << 27;
#[allow(dead_code)]
const OR: u32 = 0b00110 << 27;
#[allow(dead_code)]
const ORI: u32 = 0b00111 << 27;

// $r_call == REG31
// $r_jmp  == REG30
// $r_lab  == REG29
// $r_ret  == REG28
pub(crate) fn make_binary(bin: Vec<RustAsm>) -> Vec<RustBin> {
    // SEGURANÇA: No momento temos acesso único a variável `TraceCode`
    // justamente pelo código ser _single-thread_
    unsafe {
        if TraceCode == 1 {
            // TODO: usar `listing`
            println!("\nGerando binário\n");
        }
    }

    let mut vec = Vec::new();

    for a in bin.iter().filter(|&a| !a.cmd.eq(&a.cmd.to_lowercase())) {
        match a.cmd.as_str() {
            "NOP" => {
                let bin = RustBin { inner: NOP };
                vec.push(bin);
            }
            "HLT" => {
                let bin = RustBin { inner: HLT };
                vec.push(bin);
            }
            "JI" => {
                let mut inner = 0;
                inner |= JI;
                if a.arg1.parse::<u32>().unwrap() >= (1 << 22) {
                    panic!("overflow");
                }
                inner |= a.arg1.parse::<u32>().unwrap();
                let bin = RustBin { inner };
                vec.push(bin);
            }
            "LOAD" => {
                let mut inner = 0;
                inner |= LOAD;
                if a.arg2.parse::<u32>().unwrap() >= (1 << 22) {
                    panic!("overflow");
                }
                inner |= a.arg2.parse::<u32>().unwrap();

                let reg = a.arg1[2..].parse::<u32>().unwrap();
                if reg >= (1 << 6) {
                    panic!("overflow");
                }

                inner |= reg << 22;
                let bin = RustBin { inner };
                vec.push(bin);
            }
            "LOADI" => {
                let mut inner = 0;
                inner |= LOADI;
                if a.arg2.parse::<u32>().unwrap() >= (1 << 22) {
                    panic!("overflow");
                }
                inner |= a.arg2.parse::<u32>().unwrap();

                let reg;
                if a.arg1 == "$r_call" {
                    reg = 31;
                } else if a.arg1 == "$r_jmp" {
                    reg = 30;
                } else if a.arg1 == "$r_lab" {
                    reg = 29;
                } else {
                    reg = a.arg1[2..].parse::<u32>().unwrap();
                }
                if reg >= (1 << 6) {
                    panic!("overflow");
                }
                inner |= reg << 22;

                let bin = RustBin { inner };
                vec.push(bin);
            }
            "STORE" => {
                let mut inner = 0;
                inner |= STORE;
                if a.arg2.parse::<u32>().unwrap() >= (1 << 22) {
                    panic!("overflow");
                }
                inner |= a.arg2.parse::<u32>().unwrap();

                let reg = a.arg1[2..].parse::<u32>().unwrap();
                if reg >= (1 << 6) {
                    panic!("overflow");
                }

                inner |= reg << 22;
                let bin = RustBin { inner };
                vec.push(bin);
            }
            "MOVE" => {
                let mut inner = 0;
                inner |= MOVE;
                let mut reg;
                if a.arg1 == "$r_ret" {
                    reg = 28;
                } else {
                    if a.arg1[2..].parse::<u32>().unwrap() >= (1 << 6) {
                        panic!("overflow");
                    }
                    reg = a.arg1[2..].parse::<u32>().unwrap();
                }
                inner |= reg << 22;

                if a.arg2 == "$r_ret" {
                    reg = 28;
                } else {
                    if a.arg2[2..].parse::<u32>().unwrap() >= (1 << 6) {
                        panic!("overflow");
                    }
                    reg = a.arg2[2..].parse::<u32>().unwrap();
                }
                inner |= reg << 17;

                let bin = RustBin { inner };
                vec.push(bin);
            }
            "J" => {
                let mut inner = 0;
                inner |= J;

                let reg;
                if a.arg1 == "$r_call" {
                    reg = 31;
                } else if a.arg1 == "$r_jmp" {
                    reg = 30;
                } else if a.arg1 == "$r_lab" {
                    reg = 29;
                } else {
                    reg = a.arg1[2..].parse::<u32>().unwrap();
                }
                if reg >= (1 << 6) {
                    panic!("overflow");
                }
                inner |= reg << 22;

                let bin = RustBin { inner };
                vec.push(bin);
            }
            "JZ" => {
                let mut inner = 0;
                inner |= JZ;

                let reg;
                if a.arg1 == "$r_call" {
                    reg = 31;
                } else if a.arg1 == "$r_jmp" {
                    reg = 30;
                } else if a.arg1 == "$r_lab" {
                    reg = 29;
                } else {
                    reg = a.arg1[2..].parse::<u32>().unwrap();
                }
                if reg >= (1 << 6) {
                    panic!("overflow");
                }
                inner |= reg << 22;

                if a.arg2[2..].parse::<u32>().unwrap() >= (1 << 6) {
                    panic!("overflow");
                }
                inner |= a.arg2[2..].parse::<u32>().unwrap() << 17;

                let bin = RustBin { inner };
                vec.push(bin);
            }
            "JN" => {
                let mut inner = 0;
                inner |= JN;

                let reg;
                if a.arg1 == "$r_call" {
                    reg = 31;
                } else if a.arg1 == "$r_jmp" {
                    reg = 30;
                } else if a.arg1 == "$r_lab" {
                    reg = 29;
                } else {
                    reg = a.arg1[2..].parse::<u32>().unwrap();
                }
                if reg >= (1 << 6) {
                    panic!("overflow");
                }
                inner |= reg << 22;

                if a.arg2[2..].parse::<u32>().unwrap() >= (1 << 6) {
                    panic!("overflow");
                }
                inner |= a.arg2[2..].parse::<u32>().unwrap() << 17;

                let bin = RustBin { inner };
                vec.push(bin);
            }
            "JP" => {
                let mut inner = 0;
                inner |= JP;

                let reg;
                if a.arg1 == "$r_call" {
                    reg = 31;
                } else if a.arg1 == "$r_jmp" {
                    reg = 30;
                } else if a.arg1 == "$r_lab" {
                    reg = 29;
                } else {
                    reg = a.arg1[2..].parse::<u32>().unwrap();
                }
                if reg >= (1 << 6) {
                    panic!("overflow");
                }
                inner |= reg << 22;

                if a.arg2[2..].parse::<u32>().unwrap() >= (1 << 6) {
                    panic!("overflow");
                }
                inner |= a.arg2[2..].parse::<u32>().unwrap() << 17;

                let bin = RustBin { inner };
                vec.push(bin);
            }
            "ADD" => {
                let mut inner = 0;
                inner |= ADD;

                if a.arg1[2..].parse::<u32>().unwrap() >= (1 << 6) {
                    panic!("overflow");
                }
                inner |= a.arg1[2..].parse::<u32>().unwrap() << 22;

                if a.arg2[2..].parse::<u32>().unwrap() >= (1 << 6) {
                    panic!("overflow");
                }
                inner |= a.arg2[2..].parse::<u32>().unwrap() << 17;

                if a.arg3[2..].parse::<u32>().unwrap() >= (1 << 6) {
                    panic!("overflow");
                }
                inner |= a.arg3[2..].parse::<u32>().unwrap() << 12;

                let bin = RustBin { inner };
                vec.push(bin);
            }
            "SUB" => {
                let mut inner = 0;
                inner |= SUB;

                if a.arg1[2..].parse::<u32>().unwrap() >= (1 << 6) {
                    panic!("overflow");
                }
                inner |= a.arg1[2..].parse::<u32>().unwrap() << 22;

                if a.arg2[2..].parse::<u32>().unwrap() >= (1 << 6) {
                    panic!("overflow");
                }
                inner |= a.arg2[2..].parse::<u32>().unwrap() << 17;

                if a.arg3[2..].parse::<u32>().unwrap() >= (1 << 6) {
                    panic!("overflow");
                }
                inner |= a.arg3[2..].parse::<u32>().unwrap() << 12;

                let bin = RustBin { inner };
                vec.push(bin);
            }
            "ADDI" => {
                // TODO: ver se usa em algum lugar
                let mut inner = 0;
                inner |= ADDI;

                if a.arg1[2..].parse::<u32>().unwrap() >= (1 << 6) {
                    panic!("overflow");
                }
                inner |= a.arg1[2..].parse::<u32>().unwrap() << 22;

                if a.arg2[2..].parse::<u32>().unwrap() >= (1 << 6) {
                    panic!("overflow");
                }
                inner |= a.arg2[2..].parse::<u32>().unwrap() << 17;

                if a.arg3[2..].parse::<u32>().unwrap() >= (1 << 17) {
                    panic!("overflow");
                }
                inner |= a.arg3[2..].parse::<u32>().unwrap();

                let bin = RustBin { inner };
                vec.push(bin);
            }
            "SUBI" => {
                // TODO: ver se usa em algum lugar
                let mut inner = 0;
                inner |= SUBI;

                if a.arg1[2..].parse::<u32>().unwrap() >= (1 << 6) {
                    panic!("overflow");
                }
                inner |= a.arg1[2..].parse::<u32>().unwrap() << 22;

                if a.arg2[2..].parse::<u32>().unwrap() >= (1 << 6) {
                    panic!("overflow");
                }
                inner |= a.arg2[2..].parse::<u32>().unwrap() << 17;

                if a.arg3[2..].parse::<u32>().unwrap() >= (1 << 6) {
                    panic!("overflow");
                }
                inner |= a.arg3[2..].parse::<u32>().unwrap() << 12;

                let bin = RustBin { inner };
                vec.push(bin);
            }
            "MULT" => {
                let mut inner = 0;
                inner |= MULT;

                if a.arg1[2..].parse::<u32>().unwrap() >= (1 << 6) {
                    panic!("overflow");
                }
                inner |= a.arg1[2..].parse::<u32>().unwrap() << 22;

                if a.arg2[2..].parse::<u32>().unwrap() >= (1 << 6) {
                    panic!("overflow");
                }
                inner |= a.arg2[2..].parse::<u32>().unwrap() << 17;

                if a.arg3[2..].parse::<u32>().unwrap() >= (1 << 6) {
                    panic!("overflow");
                }
                inner |= a.arg3[2..].parse::<u32>().unwrap() << 12;

                let bin = RustBin { inner };
                vec.push(bin);
            }
            "DIV" => {
                let mut inner = 0;
                inner |= DIV;

                if a.arg1[2..].parse::<u32>().unwrap() >= (1 << 6) {
                    panic!("overflow");
                }
                inner |= a.arg1[2..].parse::<u32>().unwrap() << 22;

                if a.arg2[2..].parse::<u32>().unwrap() >= (1 << 6) {
                    panic!("overflow");
                }
                inner |= a.arg2[2..].parse::<u32>().unwrap() << 17;

                if a.arg3[2..].parse::<u32>().unwrap() >= (1 << 6) {
                    panic!("overflow");
                }
                inner |= a.arg3[2..].parse::<u32>().unwrap() << 12;

                let bin = RustBin { inner };
                vec.push(bin);
            }
            "OUT" => {
                let mut inner = 0;
                inner |= OUT;

                if a.arg2[2..].parse::<u32>().unwrap() >= (1 << 6) {
                    panic!("overflow");
                }
                inner |= a.arg2[2..].parse::<u32>().unwrap() << 17;

                let bin = RustBin { inner };
                vec.push(bin);
            }
            _ => (),
        }
    }

    // SEGURANÇA: No momento temos acesso único a variável `TraceCode`
    // justamente pelo código ser _single-thread_
    unsafe {
        if TraceCode == 1 {
            // TODO: usar `listing`
            for bin in vec.iter() {
                let bits = format!("{:032b}", bin.inner);
                println!("{}", bits);
            }
            println!("\nGeração do binário concluída.");
        }
    };

    vec
}
