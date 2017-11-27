cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
project(garble LANGUAGES C)

include(GNUInstallDirs)

option(HAVE_MSGPACK "Define whether msgpack is enabled" OFF)
set(GARBLE_HAVE_MSGPACK ${HAVE_MSGPACK})

set(garble_SOURCES
  src/block.c
  src/eval.c
  src/extend_printf.c
  src/garble.c
  src/gc.c
  src/scd.c
  )
set(garble_HEADERS
  src/garble.h
  src/garble/aes.h
  src/garble/block.h
  src/garble/garble_gate_halfgates.h
  src/garble/garble_gate_privacy_free.h
  src/garble/garble_gate_standard.h
  )

set(garblec_SOURCES
  garblec/circuit_builder.c
  garblec/aescircuits.c
  garblec/circuits.c
  )

set(garblec_HEADERS
  garblec/circuit_builder.h
  garblec/circuits.h
  )

add_library(garble SHARED ${garble_SOURCES})
target_include_directories(garble PRIVATE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>)
target_include_directories(garble INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
  $<INSTALL_INTERFACE:include>)
target_link_libraries(garble PRIVATE crypto)
target_compile_options(garble PRIVATE
  -Wall -Wextra -Wpedantic -Werror -std=gnu11 -march=native)

add_library(garblec SHARED ${garblec_SOURCES})
target_include_directories(garblec INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/garblec>
  $<INSTALL_INTERFACE:include/garblec>)
target_link_libraries(garblec PRIVATE crypto garble)
target_compile_options(garblec PRIVATE
  -Wall -Wextra -Wpedantic -Werror -std=gnu11 -march=native)

install(TARGETS garble LIBRARY DESTINATION lib)
install(FILES ${garble_HEADERS} DESTINATION include/garble)
install(TARGETS garblec LIBRARY DESTINATION lib)
install(FILES ${garblec_HEADERS} DESTINATION include/garblec)

# Test files

add_executable(test_aes test/aes.c test/utils.c)
add_executable(test_circuit test/circuit.c test/utils.c)
add_executable(test_gates test/gates.c test/utils.c)
target_include_directories(test_aes PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(test_aes PRIVATE garble garblec)
target_include_directories(test_circuit PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(test_circuit PRIVATE garble garblec)
target_include_directories(test_gates PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(test_gates PRIVATE garble garblec)

enable_testing()
add_test(NAME test_aes COMMAND test_aes)
add_test(NAME test_circuit COMMAND test_circuit)
add_test(NAME test_gates COMMAND test_gates)
