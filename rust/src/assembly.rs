use crate::*;
use std::collections::HashMap;

enum Comparison {
    LT,
    LE,
    GT,
    GE,
    EQ,
    NE,
}

#[allow(dead_code)]
struct IfComp {
    comp: Comparison,
    temp: String,
}

#[derive(Debug, Eq, Hash, PartialEq)]
struct Variable {
    name: String,
    scope: String,
}

#[derive(Debug)]
struct Memory {
    mem_location: i32,
    size: i32,
}

fn link_return(vec: &mut Vec<RustAsm>, temp: String, start: usize) {
    for i in start..vec.len() {
        if vec[i].cmd == "ret" {
            let source = vec[i].arg1.clone();
            let source = format!("$r{}", source[2..].parse::<i32>().unwrap());
            let dest = temp.clone();
            let dest = format!("$r{}", dest[2..].parse::<i32>().unwrap());
            let asm = RustAsm {
                cmd: "MOVE".to_string(),
                arg1: dest,
                arg2: source,
                arg3: "--".to_string(),
            };
            let _ = std::mem::replace(&mut vec[i - 1], asm);

            let jmp = RustAsm {
                cmd: "J".to_string(),
                arg1: "$r_call".to_string(),
                arg2: "--".to_string(),
                arg3: "--".to_string(),
            };
            let _ = std::mem::replace(&mut vec[i], jmp);
        }

        if vec[i].cmd == "end" {
            break;
        }
    }
}

pub(crate) fn make_assembly(quad: Vec<RustQuad>) -> Vec<RustAsm> {
    // SEGURANÇA: No momento temos acesso único a variável `TraceCode`
    // justamente pelo código ser _single-thread_
    unsafe {
        if TraceCode == 1 {
            // TODO: usar `listing`
            println!("\nGerando código assembly\n");
        }
    }
    let mut vec = Vec::new();
    let mut variables: HashMap<Variable, Memory> = HashMap::new();
    let mut iff = Vec::new();
    let mut mem_free = 0;

    let noop = RustAsm {
        cmd: "NOP".to_string(),
        arg1: "--".to_string(),
        arg2: "--".to_string(),
        arg3: "--".to_string(),
    };
    vec.push(noop); // J main

    for q in quad.iter() {
        match q.cmd.as_str() {
            "ALLOC" => {
                let var_size = 1;

                let k = Variable {
                    name: q.arg1.to_owned(),
                    // scope: q.arg2.to_owned(),
                    scope: "--".to_string(),
                };
                let v = Memory {
                    mem_location: mem_free,
                    size: var_size,
                };
                mem_free += var_size;

                variables.insert(k, v);
            }
            "ARG" => {
                // TODO: de algum modo linkar o arg com o lugar da memoria real
                // ou seja, a variavel mandada em param
                let var_size = 1;

                let k = Variable {
                    name: q.arg2.to_owned().to_lowercase(),
                    // scope: q.arg2.to_owned(),
                    scope: "--".to_string().to_lowercase(),
                };
                let v = Memory {
                    mem_location: mem_free,
                    size: var_size,
                };
                mem_free += var_size;

                variables.insert(k, v);
            }
            "ARRLOC" => {
                let var_size = q.arg3.parse().unwrap();

                let k = Variable {
                    name: q.arg1.to_owned(),
                    scope: q.arg2.to_owned(),
                };
                let v = Memory {
                    mem_location: mem_free,
                    size: var_size,
                };
                mem_free += var_size;

                variables.insert(k, v);
            }
            "ASSIGN" => {
                let command;
                let value;
                match q.arg2.parse::<i32>() {
                    Ok(val) => {
                        command = "LOADI".to_string();
                        value = val.to_string();
                    }
                    Err(_) => {
                        command = "MOVE".to_string();
                        value = format!(
                            "$r{}",
                            q.arg2[2..].parse::<i32>().unwrap()
                        );
                    }
                };
                let asm = RustAsm {
                    cmd: command,
                    arg1: format!("$r{}", q.arg1[2..].parse::<i32>().unwrap()),
                    arg2: value.to_string(),
                    arg3: "--".to_string(),
                };

                vec.push(asm);
            }
            "LOAD" => {
                let command;
                let value;
                match q.arg2.parse::<i32>() {
                    Ok(val) => {
                        command = "LOADI".to_string();
                        value = val;
                    }
                    Err(_) => {
                        command = "LOAD".to_string();
                        value = variables
                            .get(&Variable {
                                name: q.arg2.to_owned(),
                                scope: "--".to_string(),
                            })
                            .unwrap()
                            .mem_location;
                    }
                };
                let asm = RustAsm {
                    cmd: command,
                    arg1: format!("$r{}", q.arg1[2..].parse::<i32>().unwrap()),
                    arg2: value.to_string(),
                    arg3: "--".to_string(),
                };

                vec.push(asm);
            }
            "STORE" => {
                let mem = variables
                    .get(&Variable {
                        name: q.arg1.to_owned(),
                        scope: "--".to_owned(),
                    })
                    .unwrap()
                    .mem_location;
                let asm = RustAsm {
                    cmd: "STORE".to_string(),
                    arg1: format!("$r{}", q.arg2[2..].parse::<i32>().unwrap()),
                    arg2: mem.to_string(),
                    arg3: "--".to_string(),
                };

                vec.push(asm);
            }
            "ADD" | "SUB" | "MULT" | "DIV" => {
                let asm = RustAsm {
                    cmd: q.cmd.to_owned(),
                    arg1: format!("$r{}", q.arg1[2..].parse::<i32>().unwrap()),
                    arg2: format!("$r{}", q.arg2[2..].parse::<i32>().unwrap()),
                    arg3: format!("$r{}", q.arg3[2..].parse::<i32>().unwrap()),
                };

                vec.push(asm);
            }
            "LESS" => {
                let asm = RustAsm {
                    cmd: "SUB".to_string(),
                    arg1: format!("$r{}", q.arg1[2..].parse::<i32>().unwrap()),
                    arg2: format!("$r{}", q.arg3[2..].parse::<i32>().unwrap()),
                    arg3: format!("$r{}", q.arg2[2..].parse::<i32>().unwrap()),
                };
                let noop = RustAsm {
                    cmd: "NOP".to_string(),
                    arg1: "--".to_string(),
                    arg2: "--".to_string(),
                    arg3: "--".to_string(),
                };

                let comparison = IfComp {
                    comp: Comparison::LT,
                    temp: format!("$r{}", q.arg1[2..].parse::<i32>().unwrap()),
                };

                iff.push(comparison);
                vec.push(asm);
                vec.push(noop);
            }
            "GREAT" => {
                let asm = RustAsm {
                    cmd: "SUB".to_string(),
                    arg1: format!("$r{}", q.arg1[2..].parse::<i32>().unwrap()),
                    arg2: format!("$r{}", q.arg2[2..].parse::<i32>().unwrap()),
                    arg3: format!("$r{}", q.arg3[2..].parse::<i32>().unwrap()),
                };
                let noop = RustAsm {
                    cmd: "NOP".to_string(),
                    arg1: "--".to_string(),
                    arg2: "--".to_string(),
                    arg3: "--".to_string(),
                };

                let comparison = IfComp {
                    comp: Comparison::GT,
                    temp: format!("$r{}", q.arg1[2..].parse::<i32>().unwrap()),
                };

                iff.push(comparison);
                vec.push(asm);
                vec.push(noop);
            }
            "LEQ" => {
                let asm = RustAsm {
                    cmd: "SUB".to_string(),
                    arg1: format!("$r{}", q.arg1[2..].parse::<i32>().unwrap()),
                    arg2: format!("$r{}", q.arg3[2..].parse::<i32>().unwrap()),
                    arg3: format!("$r{}", q.arg2[2..].parse::<i32>().unwrap()),
                };
                let noop = RustAsm {
                    cmd: "NOP".to_string(),
                    arg1: "--".to_string(),
                    arg2: "--".to_string(),
                    arg3: "--".to_string(),
                };

                let comparison = IfComp {
                    comp: Comparison::LE,
                    temp: format!("$r{}", q.arg1[2..].parse::<i32>().unwrap()),
                };

                iff.push(comparison);
                vec.push(asm);
                vec.push(noop.clone());
                vec.push(noop)
            }
            "GRTEQ" => {
                let asm = RustAsm {
                    cmd: "SUB".to_string(),
                    arg1: format!("$r{}", q.arg1[2..].parse::<i32>().unwrap()),
                    arg2: format!("$r{}", q.arg2[2..].parse::<i32>().unwrap()),
                    arg3: format!("$r{}", q.arg3[2..].parse::<i32>().unwrap()),
                };
                let noop = RustAsm {
                    cmd: "NOP".to_string(),
                    arg1: "--".to_string(),
                    arg2: "--".to_string(),
                    arg3: "--".to_string(),
                };

                let comparison = IfComp {
                    comp: Comparison::GE,
                    temp: format!("$r{}", q.arg1[2..].parse::<i32>().unwrap()),
                };

                iff.push(comparison);
                vec.push(asm);
                vec.push(noop.clone());
                vec.push(noop)
            }
            "NOTEQ" => {
                let asm = RustAsm {
                    cmd: "SUB".to_string(),
                    arg1: format!("$r{}", q.arg1[2..].parse::<i32>().unwrap()),
                    arg2: format!("$r{}", q.arg2[2..].parse::<i32>().unwrap()),
                    arg3: format!("$r{}", q.arg3[2..].parse::<i32>().unwrap()),
                };
                let noop = RustAsm {
                    cmd: "NOP".to_string(),
                    arg1: "--".to_string(),
                    arg2: "--".to_string(),
                    arg3: "--".to_string(),
                };

                let comparison = IfComp {
                    comp: Comparison::NE,
                    temp: format!("$r{}", q.arg1[2..].parse::<i32>().unwrap()),
                };

                iff.push(comparison);
                vec.push(asm);
                vec.push(noop.clone());
                vec.push(noop);
            }
            "EQUAL" => {
                let asm = RustAsm {
                    cmd: "SUB".to_string(),
                    arg1: format!("$r{}", q.arg1[2..].parse::<i32>().unwrap()),
                    arg2: format!("$r{}", q.arg2[2..].parse::<i32>().unwrap()),
                    arg3: format!("$r{}", q.arg3[2..].parse::<i32>().unwrap()),
                };
                let noop = RustAsm {
                    cmd: "NOP".to_string(),
                    arg1: "--".to_string(),
                    arg2: "--".to_string(),
                    arg3: "--".to_string(),
                };

                let comparison = IfComp {
                    comp: Comparison::EQ,
                    temp: format!("$r{}", q.arg1[2..].parse::<i32>().unwrap()),
                };

                iff.push(comparison);
                vec.push(asm);
                vec.push(noop);
            }
            "HALT" => {
                let asm = RustAsm {
                    cmd: "HLT".to_string(),
                    arg1: "--".to_string(),
                    arg2: "--".to_string(),
                    arg3: "--".to_string(),
                };

                vec.push(asm);
            }
            "CALL" => {
                let asm = RustAsm {
                    cmd: q.cmd.to_owned().to_lowercase(),
                    arg1: q.arg1.to_owned().to_lowercase(),
                    arg2: q.arg2.to_owned().to_lowercase(),
                    arg3: q.arg3.to_owned().to_lowercase(),
                };
                let noop = RustAsm {
                    cmd: "NOP".to_string(),
                    arg1: "--".to_string(),
                    arg2: "--".to_string(),
                    arg3: "--".to_string(),
                };
                for _ in 0..=q.arg3.parse::<i32>().unwrap() {
                    vec.push(noop.clone());
                }
                vec.push(asm);
            }
            "RET" => {
                let asm = RustAsm {
                    cmd: q.cmd.to_owned().to_lowercase(),
                    arg1: q.arg1.to_owned().to_lowercase(),
                    arg2: q.arg2.to_owned().to_lowercase(),
                    arg3: q.arg3.to_owned().to_lowercase(),
                };
                let noop = RustAsm {
                    cmd: "NOP".to_string(),
                    arg1: "--".to_string(),
                    arg2: "--".to_string(),
                    arg3: "--".to_string(),
                };

                vec.push(noop.clone());
                vec.push(asm);
            }
            _ => {
                let asm = RustAsm {
                    cmd: q.cmd.to_owned().to_lowercase(),
                    arg1: q.arg1.to_owned().to_lowercase(),
                    arg2: q.arg2.to_owned().to_lowercase(),
                    arg3: q.arg3.to_owned().to_lowercase(),
                };

                vec.push(asm);
            }
        }
    }

    iff.reverse();

    let mut function_limits = Vec::<(String, usize, usize, String)>::new();
    let mut start = 0;
    let mut finish;
    let mut name = Vec::new();
    let mut return_type = Vec::new();
    for (i, value) in vec
        .iter()
        .enumerate()
        .filter(|&(_, q)| q.cmd.eq("fun") || q.cmd.eq("end"))
    {
        if value.cmd == "end" {
            finish = i;
            function_limits.push((
                name.pop().unwrap(),
                start,
                finish,
                return_type.pop().unwrap(),
            ));
        } else {
            start = i;
            name.push(value.arg2.to_owned());
            return_type.push(value.arg1.to_owned());
        }
    }

    for i in 0..vec.len() {
        if vec[i].cmd == "call" {
            let scope = vec[i].arg2.clone();
            let temp = vec[i].arg1.clone();
            let mut start = 0;
            for f in function_limits.iter() {
                if f.0 == scope {
                    start = f.1;
                }
            }
            let jmp = RustAsm {
                cmd: "JI".to_string(),
                arg1: start.to_string(),
                arg2: "--".to_string(),
                arg3: "--".to_string(),
            };

            for j in (0..vec[i].arg3.parse::<usize>().unwrap()).rev() {
                // pegar argumentos
            }
            let load = RustAsm {
                cmd: "LOADI".to_string(),
                arg1: "$r_call".to_string(),
                arg2: (i + 1).to_string(),
                arg3: "--".to_string(),
            };
            let _ = std::mem::replace(&mut vec[i - 1], load);
            let _ = std::mem::replace(&mut vec[i], jmp);
            link_return(&mut vec, temp, start);
        }
    }

    for function in function_limits.iter() {
        let noop = RustAsm {
            cmd: "NOP".to_string(),
            arg1: "--".to_string(),
            arg2: "--".to_string(),
            arg3: "--".to_string(),
        };
        if function.0 == "main" {
            let jmp = RustAsm {
                cmd: "JI".to_string(),
                arg1: function.1.to_string(),
                arg2: "--".to_string(),
                arg3: "--".to_string(),
            };
            let _ = std::mem::replace(&mut vec[0], jmp); // J main
            let _ = std::mem::replace(&mut vec[function.1], noop.clone());
            let _ = std::mem::replace(&mut vec[function.2], noop);
        } else {
            let ret = RustAsm {
                cmd: "J".to_string(),
                arg1: "$r_jmp".to_string(),
                arg2: "--".to_string(),
                arg3: "--".to_string(),
            };
            if function.3 == "void" {
                let _ = std::mem::replace(&mut vec[function.1], noop);
                let _ = std::mem::replace(&mut vec[function.2], ret);
            } else {
                let _ = std::mem::replace(&mut vec[function.1], noop.clone());
                let _ = std::mem::replace(&mut vec[function.2], noop);
            }
        }
    }

    let mut labels: HashMap<String, usize> = HashMap::new();
    for (i, v) in vec.iter_mut().enumerate() {
        match v.cmd.as_str() {
            "lab" => {
                let noop = RustAsm {
                    cmd: "NOP".to_string(),
                    arg1: "--".to_string(),
                    arg2: "--".to_string(),
                    arg3: "--".to_string(),
                };
                let label = std::mem::replace(v, noop);
                labels.insert(label.arg1, i);
            }
            _ => (),
        }
    }

    let mut loads = Vec::new();
    // Tem que ser feito em outro loop pois GOTO pode vir antes de LAB
    for (i, v) in vec.iter_mut().enumerate() {
        match v.cmd.as_str() {
            "goto" => {
                let location = labels.get(&v.arg1).unwrap();
                let jmp = RustAsm {
                    cmd: "JI".to_string(),
                    arg1: location.to_string(),
                    arg2: "--".to_string(),
                    arg3: "--".to_string(),
                };
                let label = std::mem::replace(v, jmp);
                labels.insert(label.arg1, i);
            }
            "iff" => {
                let location = labels.get(&v.arg2).unwrap();
                let load = RustAsm {
                    cmd: "LOADI".to_string(),
                    arg1: "$r_lab".to_string(),
                    arg2: location.to_string(),
                    arg3: "--".to_string(),
                };
                let command;
                let comparison = iff.pop().unwrap();
                match comparison.comp {
                    Comparison::LT => {
                        command = "JN".to_string();
                        loads.push((load, i - 1));
                    }
                    Comparison::LE => {
                        command = "JN".to_string();
                        let jmp = RustAsm {
                            cmd: "JZ".to_string(),
                            arg1: "$r_lab".to_string(),
                            arg2: format!(
                                "$r{}",
                                v.arg1[2..].parse::<i32>().unwrap()
                            ),
                            arg3: "--".to_string(),
                        };
                        loads.push((jmp, i - 1));
                        loads.push((load, i - 2));
                    }
                    Comparison::GT => {
                        command = "JN".to_string();
                        loads.push((load, i - 1));
                    }
                    Comparison::GE => {
                        command = "JN".to_string();
                        let jmp = RustAsm {
                            cmd: "JZ".to_string(),
                            arg1: "$r_lab".to_string(),
                            arg2: format!(
                                "$r{}",
                                v.arg1[2..].parse::<i32>().unwrap()
                            ),
                            arg3: "--".to_string(),
                        };
                        loads.push((jmp, i - 1));
                        loads.push((load, i - 2));
                    }
                    Comparison::EQ => {
                        command = "JZ".to_string();
                        loads.push((load, i - 1));
                    }
                    Comparison::NE => {
                        command = "JN".to_string();
                        let jmp = RustAsm {
                            cmd: "JP".to_string(),
                            arg1: "$r_lab".to_string(),
                            arg2: format!(
                                "$r{}",
                                v.arg1[2..].parse::<i32>().unwrap()
                            ),
                            arg3: "--".to_string(),
                        };
                        loads.push((jmp, i - 1));
                        loads.push((load, i - 2));
                    }
                }

                let asm = RustAsm {
                    cmd: command,
                    arg1: "$r_lab".to_string(),
                    arg2: format!("$r{}", v.arg1[2..].parse::<i32>().unwrap()),
                    arg3: "--".to_string(),
                };
                let _ = std::mem::replace(v, asm);
            }
            _ => (),
        }
    }

    for (load, i) in loads {
        let _ = std::mem::replace(&mut vec[i], load);
    }

    // SEGURANÇA: No momento temos acesso único a variável `TraceCode`
    // justamente pelo código ser _single-thread_
    unsafe {
        if TraceCode == 1 {
            // TODO: usar `listing`
            println!(
                "{:<3} | {:>6}, {:>6}, {:>6}, {:>6} |\n",
                "--", "CMD", "ARG1", "ARG2", "ARG3"
            );
            println!("{:?}", &function_limits);
            for (i, asm) in vec.iter().enumerate() {
                if asm.cmd.chars().any(|c| c.is_lowercase()) {
                    println!(
                        "{:<3} ( {:>6}, {:>6}, {:>6}, {:>6} )",
                        i, asm.cmd, asm.arg1, asm.arg2, asm.arg3
                    );
                } else {
                    println!(
                        "{:<3} < {:>6}, {:>6}, {:>6}, {:>6} >",
                        i, asm.cmd, asm.arg1, asm.arg2, asm.arg3
                    );
                }
            }
            for (var, loc) in variables.iter() {
                println!("");
                println!(
                    "var.name: {}, var.scope: {} -> loc.mem: {}, loc.size: {}",
                    var.name, var.scope, loc.mem_location, loc.size
                );
            }
            println!("\nGeração do código assembly concluída.");
        }
    };

    vec
}
