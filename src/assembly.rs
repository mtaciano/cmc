/* Implementação do gerador de código assembly */
use crate::ffi::{g_slot_start, g_trace_code, print_stream, std_fd};
use crate::{asm, Asm, Quad};
use std::collections::hash_map::Entry;
use std::collections::HashMap;

enum Comparison {
    LT,
    LE,
    GT,
    GE,
    EQ,
    NE,
}

#[derive(Debug, PartialEq, Eq, Hash, Clone)]
struct Variable {
    name: String,
    scope: String,
}

#[derive(Debug, Clone, Copy)]
struct Memory {
    mem_location: i32,
    size: i32,
}

struct Register {
    reserved: bool,
    available: bool,
}

fn find_available_reg(registers: &[Register]) -> Option<usize> {
    (0..32).find(|&i| !registers[i].reserved && registers[i].available)
}

fn map_reg(reg: &mut String, reg_map: &HashMap<usize, usize>) {
    let source = reg[2..].parse::<usize>().unwrap();
    let dest = reg_map.get(&source).unwrap();

    *reg = format!("$r{dest}");
}

fn remove_reg(reg: &str, reg_map: &mut HashMap<usize, usize>, available: &mut [Register]) {
    let source = reg[2..].parse::<usize>().unwrap();
    let dest = reg_map.get(&source).unwrap();

    available[*dest].available = true;
    reg_map.remove_entry(&source);
}

/* Função `link_return` é responsável por ditar onde o programa
 * tem que retornar quando uma função termina
*/
fn link_return(vec: &mut Vec<Asm>, start: usize) {
    for i in start..vec.len() {
        match vec[i].cmd.as_str() {
            "ret" => {
                let asm = Asm {
                    cmd: "MOVE".to_string(),
                    arg1: "$r_ret".to_string(),
                    arg2: format!("$r{}", vec[i].arg1[2..].parse::<i32>().unwrap()),
                    arg3: "--".to_string(),
                };
                let _ = std::mem::replace(&mut vec[i - 1], asm);

                let jmp = Asm {
                    cmd: "J".to_string(),
                    arg1: "$r_call".to_string(),
                    arg2: "--".to_string(),
                    arg3: "--".to_string(),
                };
                let _ = std::mem::replace(&mut vec[i], jmp);
            }
            "end" => {
                break;
            }
            _ => (),
        }
    }
}

/* Função `make_assembly` é responsável por criar a representação
 * assembly a partir de um vetor de quádruplas
*/
pub(crate) fn make_assembly(quad: Vec<Quad>) -> Vec<Asm> {
    // SEGURANÇA: No momento temos acesso único a variável `g_trace_code`
    // justamente pelo código ser _single-thread_
    unsafe {
        if g_trace_code == 1 {
            print_stream(std_fd, "\nGerando código assembly\n\n");
        }
    }

    let mut vec = Vec::new();
    let mut variables: HashMap<Variable, Memory> = HashMap::new();
    let mut iff = Vec::new();
    let mut mem_available = unsafe { g_slot_start };

    let debug = false;
    let mut output_param: Vec<String> = Vec::new();

    let mut fun_args: HashMap<&str, Vec<(Variable, Memory)>> = HashMap::new();
    vec.push(asm!["NOP", "--", "--", "--"]); // J main

    let mut quad_func_limits = Vec::new();
    let mut quad_finish;
    let mut quad_start = 0;
    let mut quad_name = Vec::new();
    let mut quad_ret_type = Vec::new();
    for (i, value) in quad
        .iter()
        .enumerate()
        .filter(|&(_, q)| q.cmd.eq("FUN") || q.cmd.eq("END"))
    {
        if value.cmd == "END" {
            quad_finish = i;
            quad_func_limits.push((
                quad_name.pop().unwrap(),
                quad_start,
                quad_finish,
                quad_ret_type.pop().unwrap(),
            ));
        } else {
            quad_start = i;
            quad_name.push(value.arg2.to_owned());
            quad_ret_type.push(value.arg1.to_owned());
        }
    }

    for (i, q) in quad.iter().enumerate() {
        let mut scope = "main";
        for (current, start, finish, _) in quad_func_limits.iter() {
            if i >= *start && i <= *finish {
                scope = current;
            }
        }

        match q.cmd.as_str() {
            "ALLOC" => {
                variables.insert(
                    Variable {
                        name: q.arg1.to_string(),
                        scope: q.arg2.to_string(),
                    },
                    Memory {
                        mem_location: mem_available,
                        size: 1,
                    },
                );

                mem_available += 1;
            }
            "ARG" => {
                let k = Variable {
                    name: q.arg2.to_owned().to_lowercase(),
                    scope: scope.to_owned(),
                };
                let v = Memory {
                    mem_location: mem_available,
                    size: 1,
                };

                match fun_args.entry(scope) {
                    Entry::Occupied(entry) => {
                        entry.into_mut().push((k.clone(), v));
                    }
                    Entry::Vacant(entry) => {
                        entry.insert(vec![(k.clone(), v)]);
                    }
                };

                variables.insert(k, v);

                mem_available += 1;
            }
            "ARRLOC" => {
                let var_size = q.arg3.parse().unwrap();

                let k = Variable {
                    name: q.arg1.to_owned(),
                    scope: q.arg2.to_owned(),
                };
                let v = Memory {
                    mem_location: mem_available,
                    size: var_size,
                };

                variables.insert(k, v);

                mem_available += var_size;
            }
            "ARRLOAD" => {
                let command = "LOADR".to_string();
                let value = variables
                    .get(&Variable {
                        name: q.arg2.to_owned(),
                        scope: scope.to_owned(),
                    })
                    .unwrap()
                    .mem_location;

                let dest = q.arg3.to_owned();
                let reg = Asm {
                    cmd: "ADDI".to_string(),
                    arg1: format!("$r{}", dest[2..].parse::<i32>().unwrap()),
                    arg2: format!("$r{}", dest[2..].parse::<i32>().unwrap()),
                    arg3: value.to_string(),
                };

                let asm = Asm {
                    cmd: command,
                    arg1: format!("$r{}", q.arg1[2..].parse::<i32>().unwrap()),
                    arg2: format!("$r{}", dest[2..].parse::<i32>().unwrap()),
                    arg3: "--".to_string(),
                };

                vec.push(reg);
                vec.push(asm);
            }
            "ARRSTR" => {
                let command = "STORER".to_string();
                // let value = variables
                //     .get(&Variable {
                //         name: q.arg1.to_owned(),
                //         scope: scope.to_owned(),
                //     })
                //     .unwrap()
                //     .mem_location;

                let dest = q.arg3.to_owned();
                // let reg = RustAsm {
                //     cmd: "ADDI".to_string(),
                //     arg1: format!("$r{}", dest[2..].parse::<i32>().unwrap()),
                //     arg2: value.to_string(),
                //     arg3: "--".to_string(),
                // };

                let asm = Asm {
                    cmd: command,
                    arg1: format!("$r{}", q.arg2[2..].parse::<i32>().unwrap()),
                    arg2: format!("$r{}", dest[2..].parse::<i32>().unwrap()),
                    arg3: "--".to_string(),
                };

                // vec.push(reg);
                vec.push(asm);
            }
            "ASSIGN" => {
                let (command, value) = match q.arg2.parse::<i32>() {
                    Ok(val) => ("LOADI".to_string(), val.to_string()),
                    Err(_) => (
                        "MOVE".to_string(),
                        format!("$r{}", q.arg2[2..].parse::<i32>().unwrap()),
                    ),
                };
                let asm = Asm {
                    cmd: command,
                    arg1: format!("$r{}", q.arg1[2..].parse::<i32>().unwrap()),
                    arg2: value.to_string(),
                    arg3: "--".to_string(),
                };

                vec.push(asm);
            }
            "LOAD" => {
                let (command, value) = match q.arg2.parse::<i32>() {
                    Ok(val) => ("LOADI".to_string(), val),
                    Err(_) => (
                        "LOAD".to_string(),
                        variables
                            .get(&Variable {
                                name: q.arg2.to_owned(),
                                scope: scope.to_owned(),
                            })
                            .unwrap()
                            .mem_location,
                    ),
                };
                let asm = Asm {
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
                        scope: scope.to_owned(),
                    })
                    .unwrap()
                    .mem_location;
                let asm = Asm {
                    cmd: "STORE".to_string(),
                    arg1: format!("$r{}", q.arg2[2..].parse::<i32>().unwrap()),
                    arg2: mem.to_string(),
                    arg3: "--".to_string(),
                };

                vec.push(asm);
            }
            "ADD" | "SUB" | "MULT" | "DIV" => {
                let asm = Asm {
                    cmd: q.cmd.to_owned(),
                    arg1: format!("$r{}", q.arg1[2..].parse::<i32>().unwrap()),
                    arg2: format!("$r{}", q.arg2[2..].parse::<i32>().unwrap()),
                    arg3: format!("$r{}", q.arg3[2..].parse::<i32>().unwrap()),
                };

                vec.push(asm);
            }
            "LESS" => {
                let asm = Asm {
                    cmd: "SUB".to_string(),
                    arg1: format!("$r{}", q.arg1[2..].parse::<i32>().unwrap()),
                    arg2: format!("$r{}", q.arg3[2..].parse::<i32>().unwrap()),
                    arg3: format!("$r{}", q.arg2[2..].parse::<i32>().unwrap()),
                };
                let noop = asm!["NOP", "--", "--", "--"];

                iff.push(Comparison::LT);
                vec.push(asm);
                vec.push(noop.clone());
                vec.push(noop);
            }
            "GREAT" => {
                let asm = Asm {
                    cmd: "SUB".to_string(),
                    arg1: format!("$r{}", q.arg1[2..].parse::<i32>().unwrap()),
                    arg2: format!("$r{}", q.arg2[2..].parse::<i32>().unwrap()),
                    arg3: format!("$r{}", q.arg3[2..].parse::<i32>().unwrap()),
                };
                let noop = asm!["NOP", "--", "--", "--"];

                iff.push(Comparison::GT);
                vec.push(asm);
                vec.push(noop.clone());
                vec.push(noop);
            }
            "LEQ" => {
                let asm = Asm {
                    cmd: "SUB".to_string(),
                    arg1: format!("$r{}", q.arg1[2..].parse::<i32>().unwrap()),
                    arg2: format!("$r{}", q.arg3[2..].parse::<i32>().unwrap()),
                    arg3: format!("$r{}", q.arg2[2..].parse::<i32>().unwrap()),
                };
                let noop = asm!["NOP", "--", "--", "--"];

                iff.push(Comparison::LE);
                vec.push(asm);
                vec.push(noop);
            }
            "GRTEQ" => {
                let asm = Asm {
                    cmd: "SUB".to_string(),
                    arg1: format!("$r{}", q.arg1[2..].parse::<i32>().unwrap()),
                    arg2: format!("$r{}", q.arg2[2..].parse::<i32>().unwrap()),
                    arg3: format!("$r{}", q.arg3[2..].parse::<i32>().unwrap()),
                };
                let noop = asm!["NOP", "--", "--", "--"];

                iff.push(Comparison::GE);
                vec.push(asm);
                vec.push(noop)
            }
            "NOTEQ" => {
                let asm = Asm {
                    cmd: "SUB".to_string(),
                    arg1: format!("$r{}", q.arg1[2..].parse::<i32>().unwrap()),
                    arg2: format!("$r{}", q.arg2[2..].parse::<i32>().unwrap()),
                    arg3: format!("$r{}", q.arg3[2..].parse::<i32>().unwrap()),
                };
                let noop = asm!["NOP", "--", "--", "--"];

                iff.push(Comparison::NE);
                vec.push(asm);
                vec.push(noop);
            }
            "EQUAL" => {
                let asm = Asm {
                    cmd: "SUB".to_string(),
                    arg1: format!("$r{}", q.arg1[2..].parse::<i32>().unwrap()),
                    arg2: format!("$r{}", q.arg2[2..].parse::<i32>().unwrap()),
                    arg3: format!("$r{}", q.arg3[2..].parse::<i32>().unwrap()),
                };
                let noop = asm!["NOP", "--", "--", "--"];

                iff.push(Comparison::EQ);
                vec.push(asm);
                vec.push(noop.clone());
                vec.push(noop);
            }
            "HALT" => {
                let asm = Asm {
                    cmd: "HLT".to_string(),
                    arg1: "--".to_string(),
                    arg2: "--".to_string(),
                    arg3: "--".to_string(),
                };

                vec.push(asm);
            }
            "CALL" => {
                let asm;
                let noop = asm!["NOP", "--", "--", "--"];

                if q.arg2 == "output" {
                    let param = format!(
                        "$r{}",
                        output_param.pop().unwrap()[2..].parse::<i32>().unwrap()
                    );
                    asm = Asm {
                        cmd: "OUT".to_string(),
                        arg1: "--".to_string(),
                        arg2: param,
                        arg3: "--".to_string(),
                    };
                } else if q.arg2 == "input" {
                    if debug {
                        println!("{q:?}");
                    }

                    let dest = format!("$r{}", q.arg1[2..].parse::<i32>().unwrap());
                    asm = Asm {
                        cmd: "IN".to_string(),
                        arg1: dest,
                        arg2: "--".to_string(),
                        arg3: "--".to_string(),
                    };
                } else {
                    asm = Asm {
                        cmd: q.cmd.to_owned().to_lowercase(),
                        arg1: q.arg1.to_owned().to_lowercase(),
                        arg2: q.arg2.to_owned().to_lowercase(),
                        arg3: q.arg3.to_owned().to_lowercase(),
                    };

                    vec.push(noop.clone());
                }

                vec.push(asm);

                if q.arg2 != "output" && q.arg2 != "input" {
                    vec.push(noop);
                }
            }
            "RET" => {
                vec.push(asm!["NOP", "--", "--", "--"]);

                vec.push(Asm {
                    cmd: q.cmd.to_owned().to_lowercase(),
                    arg1: q.arg1.to_owned().to_lowercase(),
                    arg2: q.arg2.to_owned().to_lowercase(),
                    arg3: q.arg3.to_owned().to_lowercase(),
                });
            }
            "PARAM" => {
                if quad[i + 1].arg2 == "output" {
                    output_param.push(q.arg1.clone());
                } else {
                    vec.push(Asm {
                        cmd: q.cmd.to_owned().to_lowercase(),
                        arg1: q.arg1.to_owned().to_lowercase(),
                        arg2: q.arg2.to_owned().to_lowercase(),
                        arg3: q.arg3.to_owned().to_lowercase(),
                    });
                }
            }
            _ => {
                let asm = Asm {
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

    for i in 0..vec.len() {
        if vec[i].cmd == "call" {
            let scope = vec[i].arg2.as_str();
            let param_num = vec[i].arg3.parse::<usize>().unwrap();
            let mut param_count = 0;
            let mut j = i;
            let params = fun_args.get(scope).unwrap();
            for p in params.iter().rev() {
                if param_count >= param_num {
                    break;
                }
                while j > 0 {
                    if param_count >= param_num {
                        break;
                    }
                    if vec[j].cmd == "param" {
                        param_count += 1;
                        let temp = vec[j].arg1.clone();
                        let loc = p.1.mem_location;
                        let asm = Asm {
                            cmd: "STORE".to_string(),
                            arg1: format!("$r{}", temp[2..].parse::<i32>().unwrap()),
                            arg2: loc.to_string(),
                            arg3: "--".to_string(),
                        };

                        let _ = std::mem::replace(&mut vec[j], asm);
                        break;
                    }

                    j -= 1;
                }
            }
        }
    }

    let mut fn_limits = Vec::<(String, usize, usize, String)>::new();
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
            fn_limits.push((
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
            for f in fn_limits.iter() {
                if f.0 == scope {
                    start = f.1;
                    break;
                }
            }
            let jmp = Asm {
                cmd: "JI".to_string(),
                arg1: start.to_string(),
                arg2: "--".to_string(),
                arg3: "--".to_string(),
            };
            let mv = Asm {
                cmd: "MOVE".to_string(),
                arg1: format!("$r{}", temp[2..].parse::<i32>().unwrap()).to_owned(),
                arg2: "$r_ret".to_string(),
                arg3: "--".to_string(),
            };

            let load = Asm {
                cmd: "LOADI".to_string(),
                arg1: "$r_call".to_string(),
                arg2: (i + 1).to_string(),
                arg3: "--".to_string(),
            };

            let _ = std::mem::replace(&mut vec[i - 1], load);
            let _ = std::mem::replace(&mut vec[i], jmp);
            let _ = std::mem::replace(&mut vec[i + 1], mv);

            link_return(&mut vec, start);
        }
    }

    for function in fn_limits.iter() {
        let noop = asm!["NOP", "--", "--", "--"];

        if function.0 == "main" {
            let jmp = Asm {
                cmd: "JI".to_string(),
                arg1: function.1.to_string(),
                arg2: "--".to_string(),
                arg3: "--".to_string(),
            };

            let _ = std::mem::replace(&mut vec[0], jmp); // J main
            let _ = std::mem::replace(&mut vec[function.1], noop.clone());
            let _ = std::mem::replace(&mut vec[function.2], noop);
        } else {
            let ret = Asm {
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
        if v.cmd.as_str() == "lab" {
            let label = std::mem::replace(v, asm!["NOP", "--", "--", "--"]);

            labels.insert(label.arg1, i);
        }
    }

    let mut loads = Vec::new();
    // Tem que ser feito em outro loop pois GOTO pode vir antes de LAB
    for (i, v) in vec.iter_mut().enumerate() {
        match v.cmd.as_str() {
            "goto" => {
                let location = labels.get(&v.arg1).unwrap();
                let jmp = Asm {
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
                let load = Asm {
                    cmd: "LOADI".to_string(),
                    arg1: "$r_lab".to_string(),
                    arg2: location.to_string(),
                    arg3: "--".to_string(),
                };

                let command = match iff.pop().unwrap() {
                    Comparison::LT => {
                        let jmp = Asm {
                            cmd: "JZ".to_string(),
                            arg1: "$r_lab".to_string(),
                            arg2: format!("$r{}", v.arg1[2..].parse::<i32>().unwrap()),
                            arg3: "--".to_string(),
                        };

                        loads.push((jmp, i - 1));
                        loads.push((load, i - 2));

                        "JN".to_string()
                    }
                    Comparison::LE => {
                        loads.push((load, i - 1));

                        "JN".to_string()
                    }
                    Comparison::GT => {
                        let jmp = Asm {
                            cmd: "JZ".to_string(),
                            arg1: "$r_lab".to_string(),
                            arg2: format!("$r{}", v.arg1[2..].parse::<i32>().unwrap()),
                            arg3: "--".to_string(),
                        };

                        loads.push((jmp, i - 1));
                        loads.push((load, i - 2));

                        "JN".to_string()
                    }
                    Comparison::GE => {
                        loads.push((load, i - 1));

                        "JN".to_string()
                    }
                    Comparison::EQ => {
                        let jmp = Asm {
                            cmd: "JP".to_string(),
                            arg1: "$r_lab".to_string(),
                            arg2: format!("$r{}", v.arg1[2..].parse::<i32>().unwrap()),
                            arg3: "--".to_string(),
                        };

                        loads.push((jmp, i - 1));
                        loads.push((load, i - 2));

                        "JN".to_string()
                    }
                    Comparison::NE => {
                        loads.push((load, i - 1));

                        "JZ".to_string()
                    }
                };

                let _ = std::mem::replace(
                    v,
                    Asm {
                        cmd: command,
                        arg1: "$r_lab".to_string(),
                        arg2: format!("$r{}", v.arg1[2..].parse::<i32>().unwrap()),
                        arg3: "--".to_string(),
                    },
                );
            }
            _ => (),
        }
    }

    for (load, i) in loads {
        let _ = std::mem::replace(&mut vec[i], load);
    }

    let mut register_map: HashMap<usize, usize> = HashMap::new();
    let mut available_reg = Vec::with_capacity(32);

    for i in 0..32 {
        let mut reserved = false;
        let mut available = true;

        if i >= 28 {
            reserved = true;
            available = false;
        }

        available_reg.push(Register {
            reserved,
            available,
        });
    }

    let refactor = true;
    if refactor {
        let mut start = 0;
        for (fun, i, _, _) in fn_limits.iter() {
            if fun == "main" {
                start = *i;
                break;
            }
        }
        for i in 0..vec.len() {
            match vec[i].cmd.as_str() {
                "LOAD" | "LOADI" => {
                    if vec[i].arg1 != "$r_call" && vec[i].arg1 != "$r_lab" {
                        let register = vec[i].arg1[2..].parse::<usize>().unwrap();
                        if let Entry::Vacant(e) = register_map.entry(register) {
                            let available = find_available_reg(&available_reg)
                                .unwrap_or_else(|| panic!("register overflow"));
                            available_reg[available].available = false;
                            e.insert(available);
                            vec[i].arg1 = format!("$r{available}");
                        } else {
                            let dest = register_map.get(&register).unwrap();
                            vec[i].arg1 = format!("$r{dest}");
                        }
                    }
                }
                "LOADR" => {
                    let register = vec[i].arg1[2..].parse::<usize>().unwrap();
                    if let Entry::Vacant(e) = register_map.entry(register) {
                        let available = find_available_reg(&available_reg)
                            .unwrap_or_else(|| panic!("register overflow"));
                        available_reg[available].available = false;
                        e.insert(available);
                        vec[i].arg1 = format!("$r{available}");
                        map_reg(&mut vec[i].arg2, &register_map);
                    }
                }
                "STORE" => {
                    let old = vec[i].arg1.clone();
                    map_reg(&mut vec[i].arg1, &register_map);
                    if i > start {
                        // HACK: não sei melhorar dentro de f()
                        remove_reg(&old, &mut register_map, &mut available_reg);
                    }
                }
                "STORER" => {
                    let old1 = vec[i].arg1.clone();
                    map_reg(&mut vec[i].arg1, &register_map);
                    if i > start {
                        // HACK: não sei melhorar dentro de f()
                        remove_reg(&old1, &mut register_map, &mut available_reg);
                    }

                    let old2 = vec[i].arg2.clone();
                    map_reg(&mut vec[i].arg2, &register_map);
                    if i > start {
                        // HACK: não sei melhorar dentro de f()
                        remove_reg(&old2, &mut register_map, &mut available_reg);
                    }
                }
                "ADD" | "SUB" | "MULT" | "DIV" => {
                    let register = vec[i].arg1[2..].parse::<usize>().unwrap();
                    let available = find_available_reg(&available_reg)
                        .unwrap_or_else(|| panic!("register overflow"));
                    available_reg[available].available = false;
                    register_map.insert(register, available);
                    vec[i].arg1 = format!("$r{available}");

                    let old2 = vec[i].arg2.clone();
                    map_reg(&mut vec[i].arg2, &register_map);
                    if i > start {
                        // HACK: não sei melhorar dentro de f()
                        remove_reg(&old2, &mut register_map, &mut available_reg);
                    }

                    let old3 = vec[i].arg3.clone();
                    map_reg(&mut vec[i].arg3, &register_map);
                    if i > start {
                        // HACK: não sei melhorar dentro de f()
                        remove_reg(&old3, &mut register_map, &mut available_reg);
                    }
                }
                "ADDI" => {
                    map_reg(&mut vec[i].arg1, &register_map);
                    map_reg(&mut vec[i].arg2, &register_map);
                }
                "MOVE" => {
                    if vec[i].arg1 != "$r_call" && vec[i].arg1 != "$r_ret" {
                        if register_map.contains_key(&vec[i].arg1[2..].parse::<usize>().unwrap()) {
                            map_reg(&mut vec[i].arg1, &register_map);
                        } else {
                            let register = vec[i].arg1[2..].parse::<usize>().unwrap();
                            let available = find_available_reg(&available_reg)
                                .unwrap_or_else(|| panic!("register overflow"));
                            available_reg[available].available = false;
                            register_map.insert(register, available);
                            vec[i].arg1 = format!("$r{available}");
                        }
                    }
                    if vec[i].arg2 != "$r_ret" {
                        let old = vec[i].arg2.clone();
                        map_reg(&mut vec[i].arg2, &register_map);
                        if i > start {
                            // HACK: não sei melhorar dentro de f()
                            remove_reg(&old, &mut register_map, &mut available_reg);
                        }
                    }
                }
                "OUT" => {
                    let old = vec[i].arg2.clone();
                    map_reg(&mut vec[i].arg2, &register_map);
                    if i > start {
                        // HACK: não sei melhorar dentro de f()
                        remove_reg(&old, &mut register_map, &mut available_reg);
                    }
                }
                "IN" => {
                    let register = vec[i].arg1[2..].parse::<usize>().unwrap();
                    let available = find_available_reg(&available_reg)
                        .unwrap_or_else(|| panic!("register overflow"));
                    available_reg[available].available = false;
                    register_map.insert(register, available);
                    vec[i].arg1 = format!("$r{available}");
                }
                "JN" | "JZ" | "JP" => {
                    let old = vec[i].arg2.clone();
                    map_reg(&mut vec[i].arg2, &register_map);
                    let jump = &vec[i + 1].cmd;
                    if jump != "JN" && jump != "JZ" && jump != "JP" {
                        remove_reg(&old, &mut register_map, &mut available_reg);
                    }
                }
                _ => (),
            }
        }
    }

    // SEGURANÇA: No momento temos acesso único a variável `g_trace_code`
    // justamente pelo código ser _single-thread_
    unsafe {
        if g_trace_code == 1 {
            // TODO: mudar para uma função (print_assembly)
            // TODO: usar `listing`
            print_stream(
                std_fd,
                format!(
                    "{:<3} | {:>6}, {:>6}, {:>6}, {:>6} |\n",
                    "--", "CMD", "ARG1", "ARG2", "ARG3"
                )
                .as_str(),
            );

            if debug {
                println!("{:?}", &fn_limits);
            }

            for (i, asm) in vec.iter().enumerate() {
                print_stream(
                    std_fd,
                    format!(
                        "{:<3} < {:>6}, {:>6}, {:>6}, {:>6} >\n",
                        i, asm.cmd, asm.arg1, asm.arg2, asm.arg3
                    )
                    .as_str(),
                );
            }

            if debug {
                for (var, loc) in variables.iter() {
                    println!();
                    println!(
                        "var.name: {}, var.scope: {} -> loc.mem: {}, loc.size: {}",
                        var.name, var.scope, loc.mem_location, loc.size
                    );
                }
            }

            print_stream(std_fd, "\nGeração do código assembly concluída.\n");
        }
    };

    vec
}
