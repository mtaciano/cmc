use crate::{ffi::g_trace_code, *};

/* Todos os opcodes do processador e funções auxiliares */
mod opcodes {
    pub(crate) const NOP: u32 = 0b00000 << 27;
    pub(crate) const HLT: u32 = 0b00001 << 27;
    pub(crate) const IN: u32 = 0b00010 << 27;
    pub(crate) const OUT: u32 = 0b00011 << 27;
    pub(crate) const MULT: u32 = 0b01000 << 27;
    pub(crate) const DIV: u32 = 0b01001 << 27;
    pub(crate) const ADD: u32 = 0b01011 << 27;
    pub(crate) const ADDI: u32 = 0b01100 << 27;
    pub(crate) const SUB: u32 = 0b01101 << 27;
    pub(crate) const SUBI: u32 = 0b01110 << 27;
    pub(crate) const STORE: u32 = 0b01111 << 27;
    pub(crate) const MOVE: u32 = 0b10000 << 27;
    pub(crate) const LOAD: u32 = 0b10001 << 27;
    pub(crate) const LOADI: u32 = 0b10010 << 27;
    pub(crate) const J: u32 = 0b10011 << 27;
    pub(crate) const JI: u32 = 0b10100 << 27;
    pub(crate) const JZ: u32 = 0b10101 << 27;
    pub(crate) const JN: u32 = 0b10111 << 27;
    pub(crate) const JP: u32 = 0b11001 << 27;
    pub(crate) const STORER: u32 = 0b11010 << 27; // RS RD
    pub(crate) const LOADR: u32 = 0b11011 << 27; // RD RS

    // TODO: implementar futuramente
    // const NOT: u32 = 0b01010 << 27;
    // const JZI: u32 = 0b10110 << 27;
    // const JNI: u32 = 0b11000 << 27;
    // const AND: u32 = 0b00100 << 27;
    // const ANDI: u32 = 0b00101 << 27;
    // const OR: u32 = 0b00110 << 27;
    // const ORI: u32 = 0b00111 << 27;

    /* Função `from_str` transforma uma string em um opcode */
    pub(crate) fn from_str(str: &str) -> u32 {
        match str {
            "NOP" => NOP,
            "HLT" => HLT,
            "IN" => IN,
            "OUT" => OUT,
            "MULT" => MULT,
            "DIV" => DIV,
            "ADD" => ADD,
            "ADDI" => ADDI,
            "SUB" => SUB,
            "SUBI" => SUBI,
            "STORE" => STORE,
            "LOAD" => LOAD,
            "MOVE" => MOVE,
            "LOADI" => LOADI,
            "J" => J,
            "JI" => JI,
            "JZ" => JZ,
            "JN" => JN,
            "JP" => JP,
            "STORER" => STORER,
            "LOADR" => LOADR,
            _ => panic!("opcode {} não conhecido!", str),
        }
    }
}

/* Função `parse_register` é responsável por verificar
 * a validade de um registrador, e caso ele seja válido
 * ela retorna seu valor em `u32`
 */
fn parse_register(reg: &str) -> u32 {
    // Verificar se é um registrador reservado
    if reg.eq("$r_call") {
        return 31;
    } else if reg.eq("$r_jmp") {
        return 30;
    } else if reg.eq("$r_lab") {
        return 29;
    } else if reg.eq("$r_ret") {
        return 28;
    }

    // Não é reservado
    let number = match reg[2..].parse::<u32>() {
        Ok(num) => num,
        Err(_) => {
            panic!("Registrador não reservado, mas com caracteres inválidos!");
        }
    };

    if number <= 32 {
        number
    } else {
        panic!("Tamanho inválido de registrador.");
    }
}

/* Função `parse_imediate` é responsável por verificar
 * a validade de um imediato, e caso ele seja válido
 * ela retorna seu valor em `u32`
 */
fn parse_imediate(im: &str, size: i32) -> u32 {
    match im.parse::<u32>() {
        Ok(num) => {
            if num >= (1 << size) {
                panic!("Imediato causa overflow!");
            }

            num
        }
        Err(_) => panic!("Imediato possui caracteres inválidos!"),
    }
}

/* Função `make_binary` é responsável por criar a representação
 * binária a partir de um vetor de instruções assembly
 */
pub(crate) fn make_binary(bin: Vec<Asm>) -> Vec<Bin> {
    // SEGURANÇA: No momento temos acesso único a variável `g_trace_code`
    // justamente pelo código ser _single-thread_
    unsafe {
        if g_trace_code == 1 {
            // TODO: usar `listing`
            println!("\nGerando binário\n");
        }
    }

    let mut vec = Vec::new();

    for inst in bin.iter() {
        match inst.cmd.as_str() {
            "NOP" | "HLT" => {
                vec.push(Bin {
                    word: opcodes::from_str(&inst.cmd),
                });
            }
            "LOAD" | "LOADI" | "STORE" => {
                // OPCODE RD IM22 (Endereço)
                let mut inner = opcodes::from_str(&inst.cmd);

                inner |= parse_imediate(&inst.arg2, 22);

                let reg = parse_register(&inst.arg1);
                inner |= reg << 22;

                vec.push(Bin { word: inner });
            }
            "STORER" | "LOADR" | "MOVE" => {
                let mut inner = opcodes::from_str(&inst.cmd);

                let rs = parse_register(&inst.arg1);
                inner |= rs << 22;

                let rd = parse_register(&inst.arg2);
                inner |= rd << 17;

                vec.push(Bin { word: inner });
            }
            "J" => {
                let mut inner = opcodes::J;

                let reg = parse_register(&inst.arg1);
                inner |= reg << 22;

                vec.push(Bin { word: inner });
            }
            "JZ" | "JN" | "JP" => {
                let mut inner = opcodes::from_str(&inst.cmd);

                let rs = parse_register(&inst.arg1);
                inner |= rs << 22;

                let rd = parse_register(&inst.arg2);
                inner |= rd << 17;

                vec.push(Bin { word: inner });
            }
            "JI" => {
                // TODO: ver oq fazer com isso
                let mut inner = opcodes::JI;

                if inst.arg1.parse::<u32>().unwrap() >= (1 << 22) {
                    panic!("overflow");
                }

                inner |= inst.arg1.parse::<u32>().unwrap();

                let bin = Bin { word: inner };

                vec.push(bin);
            }
            "ADD" | "SUB" | "MULT" | "DIV" => {
                let mut inner = opcodes::from_str(&inst.cmd);

                let rs = parse_register(&inst.arg1);
                inner |= rs << 22;

                let rd = parse_register(&inst.arg2);
                inner |= rd << 17;

                let rt = parse_register(&inst.arg3);
                inner |= rt << 12;

                vec.push(Bin { word: inner });
            }
            "ADDI" | "SUBI" => {
                let mut inner = opcodes::from_str(&inst.cmd);

                let rs = parse_register(&inst.arg1);
                inner |= rs << 22;

                let rd = parse_register(&inst.arg2);
                inner |= rd << 17;

                inner |= parse_imediate(&inst.arg3, 17);

                vec.push(Bin { word: inner });
            }
            "OUT" => {
                let mut inner = opcodes::OUT;

                let rd = parse_register(&inst.arg2);
                inner |= rd << 17;

                vec.push(Bin { word: inner });
            }
            "IN" => {
                let mut inner = opcodes::IN;

                let rs = parse_register(&inst.arg1);
                inner |= rs << 22;

                vec.push(Bin { word: inner });
            }
            _ => panic!("Valor de assembly não conhecido!"),
        }
    }

    // SEGURANÇA: No momento temos acesso único a variável `g_trace_code`
    // justamente pelo código ser _single-thread_
    unsafe {
        if g_trace_code == 1 {
            // TODO: usar `listing`
            for bin in vec.iter() {
                let bits = format!("{:032b}", bin.word);
                println!("{}", bits);
            }
            println!("\nGeração do binário concluída.");
        }
    };

    vec
}
