cmake_minimum_required(VERSION 3.11)
project(cmc LANGUAGES C)

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fsanitize=address,undefined")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -march=native -flto")

# Cria o tipo de build automáticamente dependendo do ambiente
set(default_build_type "Release")
if(EXISTS "${CMAKE_SOURCE_DIR}/.git")
    set(default_build_type "Debug")
endif()

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS
        "Setting build type to '${default_build_type}' as none was specified."
    )
    set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
        STRING "Choose the type of build." FORCE
    )
    # Valores possíveis de serem escolhidos pela cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
        "Debug" "Release" "MinSizeRel" "RelWithDebInfo"
    )
endif()

# Parser e Scanner
find_package(BISON)
find_package(FLEX)
BISON_TARGET(parser src/parse.y ${CMAKE_CURRENT_BINARY_DIR}/parse.tab.c
    COMPILE_FLAGS "-W --color=always"
)
FLEX_TARGET(scanner src/scan.l ${CMAKE_CURRENT_BINARY_DIR}/scan.yy.c)
ADD_FLEX_BISON_DEPENDENCY(scanner parser)

# Parte em Rust
include(FetchContent)
FetchContent_Declare(
    Corrosion
    GIT_REPOSITORY https://github.com/corrosion-rs/corrosion.git
)
FetchContent_MakeAvailable(Corrosion)
corrosion_import_crate(MANIFEST_PATH Cargo.toml)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_executable(cmc
    src/main.c
    src/common/globals.h
    src/common/util.c
    src/common/util.h
    src/analyze.c
    src/analyze.h
    src/intermediate.c
    src/intermediate.h
    src/parse.h
    src/rust.h
    src/scan.h
    src/symtab.c
    src/symtab.h
    ${BISON_parser_OUTPUTS}
    ${FLEX_scanner_OUTPUTS}
)
target_link_libraries(cmc
    ${FLEX_LIBRARIES}
    rust
)
target_compile_options(cmc PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -std=c99
)
