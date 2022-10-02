use ::core::ffi::c_char;
use ::core::ffi::c_int;

/* A variável `g_trace_code` faz com que informações adicionais sejam printadas
 * durante a fase de geração do código intermediário
 */
extern "C" {
    pub(crate) static mut g_trace_code: c_int;
}

/* Struct de Quádruplas */
#[repr(C)]
pub struct QuadRec {
    pub cmd: *mut c_char,
    pub arg1: *mut c_char,
    pub arg2: *mut c_char,
    pub arg3: *mut c_char,
    pub next: *mut QuadRec,
}
pub type Quad = *mut QuadRec;

// TODO: talvez os testes não sejam necessários
#[allow(non_upper_case_globals)]
#[allow(non_camel_case_types)]
#[allow(non_snake_case)]
#[cfg(test)]
mod tests {
    use super::*;
    use ::std::mem;
    use ::std::ptr;

    // Verifica se `QuadRec` tem o tamanho e alinhamento correto
    #[test]
    fn test_layout_QuadRec() {
        // Verifica que o tamanho de `QuadRec` é 40usize
        assert_eq!(
            mem::size_of::<QuadRec>(),
            40usize,
            concat!("Size of: ", stringify!(QuadRec))
        );

        // Verifica que o alinhamento de `QuadRec` é 8usize
        assert_eq!(
            mem::align_of::<QuadRec>(),
            8usize,
            concat!("Alignment of ", stringify!(QuadRec))
        );
    }

    // Verifica que o alinhamento do campo `cmd` está certo
    #[test]
    fn test_cmd_offset() {
        assert_eq!(
            unsafe {
                let uninit = mem::MaybeUninit::<QuadRec>::uninit();
                let ptr = uninit.as_ptr();
                ptr::addr_of!((*ptr).cmd) as usize - ptr as usize
            },
            0usize,
            concat!(
                "Offset of field: ",
                stringify!(QuadRec),
                "::",
                stringify!(cmd)
            )
        );
    }

    // Verifica que o offset do campo `arg1` está certo
    #[test]
    fn test_arg1_offset() {
        assert_eq!(
            unsafe {
                let uninit = mem::MaybeUninit::<QuadRec>::uninit();
                let ptr = uninit.as_ptr();
                ptr::addr_of!((*ptr).arg1) as usize - ptr as usize
            },
            8usize,
            concat!(
                "Offset of field: ",
                stringify!(QuadRec),
                "::",
                stringify!(arg1)
            )
        );
    }

    #[test]
    // Verifica que o offset do campo `arg2` está certo
    fn test_arg2_offset() {
        assert_eq!(
            unsafe {
                let uninit = mem::MaybeUninit::<QuadRec>::uninit();
                let ptr = uninit.as_ptr();
                ptr::addr_of!((*ptr).arg2) as usize - ptr as usize
            },
            16usize,
            concat!(
                "Offset of field: ",
                stringify!(QuadRec),
                "::",
                stringify!(arg2)
            )
        );
    }

    #[test]
    // Verifica que o offset do campo `arg3` está certo
    fn test_arg3_offset() {
        assert_eq!(
            unsafe {
                let uninit = mem::MaybeUninit::<QuadRec>::uninit();
                let ptr = uninit.as_ptr();
                ptr::addr_of!((*ptr).arg3) as usize - ptr as usize
            },
            24usize,
            concat!(
                "Offset of field: ",
                stringify!(QuadRec),
                "::",
                stringify!(arg3)
            )
        );
    }

    #[test]
    // Verifica que o offset do campo `next` está certo
    fn test_next_offset() {
        assert_eq!(
            unsafe {
                let uninit = mem::MaybeUninit::<QuadRec>::uninit();
                let ptr = uninit.as_ptr();
                ptr::addr_of!((*ptr).next) as usize - ptr as usize
            },
            32usize,
            concat!(
                "Offset of field: ",
                stringify!(QuadRec),
                "::",
                stringify!(next)
            )
        );
    }
}
