use crate::*;
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

#[allow(dead_code)]
struct IfComp {
    comp: Comparison,
    temp: String,
}

#[derive(Debug, Eq, Hash, PartialEq, Clone)]
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

#[inline(always)]
fn find_available_reg(registers: &[Register]) -> Option<usize> {
    (0..32).find(|&i| !registers[i].reserved && registers[i].available)
}

#[inline(always)]
fn map_reg(reg: &mut String, reg_map: &HashMap<usize, usize>) {
    let source = reg[2..].parse::<usize>().unwrap();
    let dest = reg_map.get(&source).unwrap();

    *reg = format!("$r{}", dest);
}

#[inline(always)]
fn remove_reg(
    reg: &str,
    reg_map: &mut HashMap<usize, usize>,
    available: &mut [Register],
) {
    let source = reg[2..].parse::<usize>().unwrap();
    let dest = reg_map.get(&source).unwrap();
    available[*dest].available = true;
    reg_map.remove_entry(&source);
}

fn link_return(vec: &mut Vec<RustAsm>, start: usize) {
    for i in start..vec.len() {
        if vec[i].cmd == "ret" {
            let source = vec[i].arg1.clone();
            let source = format!("$r{}", source[2..].parse::<i32>().unwrap());
            let asm = RustAsm {
                cmd: "MOVE".to_string(),
                arg1: "$r_ret".to_string(),
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

    let debug = false;
    let mut output_param = Vec::new();

    let mut fun_args: HashMap<&str, Vec<(Variable, Memory)>> = HashMap::new();

    let noop = RustAsm {
        cmd: "NOP".to_string(),
        arg1: "--".to_string(),
        arg2: "--".to_string(),
        arg3: "--".to_string(),
    };
    vec.push(noop); // J main

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
                let var_size = 1;

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
            "ARG" => {
                let var_size = 1;

                let k = Variable {
                    name: q.arg2.to_owned().to_lowercase(),
                    scope: scope.to_owned(),
                };
                let v = Memory {
                    mem_location: mem_free,
                    size: var_size,
                };
                mem_free += var_size;

                let clone_k = k.clone();
                fun_args
                    .entry(scope)
                    .and_modify(|value| value.push((clone_k.clone(), v)))
                    .or_insert_with(|| vec![(clone_k, v)]);

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
                let reg = RustAsm {
                    cmd: "ADDI".to_string(),
                    arg1: format!("$r{}", dest[2..].parse::<i32>().unwrap()),
                    arg2: format!("$r{}", dest[2..].parse::<i32>().unwrap()),
                    arg3: value.to_string(),
                };

                let asm = RustAsm {
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

                let asm = RustAsm {
                    cmd: command,
                    arg1: format!("$r{}", q.arg2[2..].parse::<i32>().unwrap()),
                    arg2: format!("$r{}", dest[2..].parse::<i32>().unwrap()),
                    arg3: "--".to_string(),
                };

                // vec.push(reg);
                vec.push(asm);
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
                                scope: scope.to_owned(),
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
                        scope: scope.to_owned(),
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
                vec.push(noop.clone());
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
                vec.push(noop.clone());
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
                vec.push(noop);
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
                vec.push(noop.clone());
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
                let asm;
                let noop = RustAsm {
                    cmd: "NOP".to_string(),
                    arg1: "--".to_string(),
                    arg2: "--".to_string(),
                    arg3: "--".to_string(),
                };
                if q.arg2 == "output" {
                    let mut param: String = output_param.pop().unwrap();
                    param = format!("$r{}", param[2..].parse::<i32>().unwrap());
                    asm = RustAsm {
                        cmd: "OUT".to_string(),
                        arg1: "--".to_string(),
                        arg2: param,
                        arg3: "--".to_string(),
                    };
                } else if q.arg2 == "input" {
                    if debug {
                        println!("{:?}", q);
                    }
                    let dest =
                        format!("$r{}", q.arg1[2..].parse::<i32>().unwrap());
                    asm = RustAsm {
                        cmd: "IN".to_string(),
                        arg1: dest,
                        arg2: "--".to_string(),
                        arg3: "--".to_string(),
                    };
                } else {
                    asm = RustAsm {
                        cmd: q.cmd.to_owned().to_lowercase(),
                        arg1: q.arg1.to_owned().to_lowercase(),
                        arg2: q.arg2.to_owned().to_lowercase(),
                        arg3: q.arg3.to_owned().to_lowercase(),
                    };
                    vec.push(noop.clone());

                    // for _ in 0..=q.arg3.parse::<i32>().unwrap() {
                    //     vec.push(noop.clone());
                    // }
                }
                vec.push(asm);
                if q.arg2 != "output" && q.arg2 != "input" {
                    vec.push(noop);
                }
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
            "PARAM" => {
                if quad[i + 1].arg2 == "output" {
                    output_param.push(q.arg1.clone());
                } else {
                    let asm = RustAsm {
                        cmd: q.cmd.to_owned().to_lowercase(),
                        arg1: q.arg1.to_owned().to_lowercase(),
                        arg2: q.arg2.to_owned().to_lowercase(),
                        arg3: q.arg3.to_owned().to_lowercase(),
                    };
                    vec.push(asm);
                }
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
                        let asm = RustAsm {
                            cmd: "STORE".to_string(),
                            arg1: format!(
                                "$r{}",
                                temp[2..].parse::<i32>().unwrap()
                            ),
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
                    break;
                }
            }
            let jmp = RustAsm {
                cmd: "JI".to_string(),
                arg1: start.to_string(),
                arg2: "--".to_string(),
                arg3: "--".to_string(),
            };
            let mv = RustAsm {
                cmd: "MOVE".to_string(),
                arg1: format!("$r{}", temp[2..].parse::<i32>().unwrap())
                    .to_owned(),
                arg2: "$r_ret".to_string(),
                arg3: "--".to_string(),
            };

            let load = RustAsm {
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
        if v.cmd.as_str() == "lab" {
            let noop = RustAsm {
                cmd: "NOP".to_string(),
                arg1: "--".to_string(),
                arg2: "--".to_string(),
                arg3: "--".to_string(),
            };
            let label = std::mem::replace(v, noop);
            labels.insert(label.arg1, i);
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
                    Comparison::LE => {
                        command = "JN".to_string();
                        loads.push((load, i - 1));
                    }
                    Comparison::GT => {
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
                    Comparison::GE => {
                        command = "JN".to_string();
                        loads.push((load, i - 1));
                    }
                    Comparison::EQ => {
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
                    Comparison::NE => {
                        command = "JZ".to_string();
                        loads.push((load, i - 1));
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
        for (fun, i, _, _) in function_limits.iter() {
            if fun == "main" {
                start = *i;
                break;
            }
        }
        for i in 0..vec.len() {
            match vec[i].cmd.as_str() {
                "LOAD" | "LOADI" => {
                    if vec[i].arg1 != "$r_call" && vec[i].arg1 != "$r_lab" {
                        let register =
                            vec[i].arg1[2..].parse::<usize>().unwrap();
                        if let Entry::Vacant(e) = register_map.entry(register) {
                            let available = find_available_reg(&available_reg)
                                .unwrap_or_else(|| panic!("register overflow"));
                            available_reg[available].available = false;
                            e.insert(available);
                            vec[i].arg1 = format!("$r{}", available);
                        } else {
                            let dest = register_map.get(&register).unwrap();
                            vec[i].arg1 = format!("$r{}", dest);
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
                        vec[i].arg1 = format!("$r{}", available);
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
                        remove_reg(
                            &old1,
                            &mut register_map,
                            &mut available_reg,
                        );
                    }

                    let old2 = vec[i].arg2.clone();
                    map_reg(&mut vec[i].arg2, &register_map);
                    if i > start {
                        // HACK: não sei melhorar dentro de f()
                        remove_reg(
                            &old2,
                            &mut register_map,
                            &mut available_reg,
                        );
                    }
                }
                "ADD" | "SUB" | "MULT" | "DIV" => {
                    let register = vec[i].arg1[2..].parse::<usize>().unwrap();
                    let available = find_available_reg(&available_reg)
                        .unwrap_or_else(|| panic!("register overflow"));
                    available_reg[available].available = false;
                    register_map.insert(register, available);
                    vec[i].arg1 = format!("$r{}", available);

                    let old2 = vec[i].arg2.clone();
                    map_reg(&mut vec[i].arg2, &register_map);
                    if i > start {
                        // HACK: não sei melhorar dentro de f()
                        remove_reg(
                            &old2,
                            &mut register_map,
                            &mut available_reg,
                        );
                    }

                    let old3 = vec[i].arg3.clone();
                    map_reg(&mut vec[i].arg3, &register_map);
                    if i > start {
                        // HACK: não sei melhorar dentro de f()
                        remove_reg(
                            &old3,
                            &mut register_map,
                            &mut available_reg,
                        );
                    }
                }
                "ADDI" => {
                    map_reg(&mut vec[i].arg1, &register_map);
                    map_reg(&mut vec[i].arg2, &register_map);
                }
                "MOVE" => {
                    if vec[i].arg1 != "$r_call" && vec[i].arg1 != "$r_ret" {
                        if register_map.contains_key(
                            &vec[i].arg1[2..].parse::<usize>().unwrap(),
                        ) {
                            map_reg(&mut vec[i].arg1, &register_map);
                        } else {
                            let register =
                                vec[i].arg1[2..].parse::<usize>().unwrap();
                            let available = find_available_reg(&available_reg)
                                .unwrap_or_else(|| panic!("register overflow"));
                            available_reg[available].available = false;
                            register_map.insert(register, available);
                            vec[i].arg1 = format!("$r{}", available);
                        }
                    }
                    if vec[i].arg2 != "$r_ret" {
                        let old = vec[i].arg2.clone();
                        map_reg(&mut vec[i].arg2, &register_map);
                        if i > start {
                            // HACK: não sei melhorar dentro de f()
                            remove_reg(
                                &old,
                                &mut register_map,
                                &mut available_reg,
                            );
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
                    vec[i].arg1 = format!("$r{}", available);
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

    // SEGURANÇA: No momento temos acesso único a variável `TraceCode`
    // justamente pelo código ser _single-thread_
    unsafe {
        if TraceCode == 1 {
            // TODO: usar `listing`
            println!(
                "{:<3} | {:>6}, {:>6}, {:>6}, {:>6} |\n",
                "--", "CMD", "ARG1", "ARG2", "ARG3"
            );
            if debug {
                println!("{:?}", &function_limits);
            }
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
            if debug {
                for (var, loc) in variables.iter() {
                    println!();
                    println!(
                    "var.name: {}, var.scope: {} -> loc.mem: {}, loc.size: {}",
                    var.name, var.scope, loc.mem_location, loc.size);
                }
            }
            println!("\nGeração do código assembly concluída.");
        }
    };

    vec
}
