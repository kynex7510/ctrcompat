cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

# C23, CXX, ASM required for libn3ds.
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
project(ctrcompat C CXX ASM)

if (CTR_BAREMETAL)
    # Include libn3ds for baremetal compilation.
    include(BM/N3DS11)
    message(STATUS "ctrcompat: Compiling for baremetal (libn3ds)")
else()
    message(STATUS "ctrcompat: Compiling for userland (libctru)")
endif()

# Setup library.
set (CTRCOMPAT_SOURCES Source/QTMRAM.c)

if (CTR_BAREMETAL)
    set(CTRCOMPAT_SOURCES ${CTRCOMPAT_SOURCES}
        Source/libn3ds.c
    )
else()
    set(CTRCOMPAT_SOURCES ${CTRCOMPAT_SOURCES}
        Source/HOS.c
    )
endif()

add_library(ctrcompat STATIC ${CTRCOMPAT_SOURCES})

if (CTR_BAREMETAL)
    target_include_directories(ctrcompat PUBLIC ${LIBN3DS_INCLUDE_PATHS})
    target_compile_definitions(ctrcompat PUBLIC CTR_BAREMETAL)
    target_link_libraries(ctrcompat PUBLIC n3ds11)
endif()

target_include_directories(ctrcompat PUBLIC Include)
target_compile_options(ctrcompat PRIVATE -Wall -Werror)

install(TARGETS ctrcompat)
install(DIRECTORY Include/ DESTINATION include)