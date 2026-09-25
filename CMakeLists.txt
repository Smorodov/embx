cmake_minimum_required(VERSION 3.16)
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" EMBX_VERSION_FILE)
string(STRIP "${EMBX_VERSION_FILE}" EMBX_VERSION)
if(NOT EMBX_VERSION MATCHES "^[0-9]+\\.[0-9]+\\.[0-9]+$")
  message(FATAL_ERROR "Invalid EmbX VERSION file: ${EMBX_VERSION}")
endif()
project(EmbX VERSION ${EMBX_VERSION} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Catch2 is the single test framework used by the entire EmbX test suite.
# Prefer a suitable system Catch2 3 installation; use the pinned source
# fallback only when no compatible package is available.
# Derive the MSYS2 UCRT64 prefix from the active C++ compiler by default.
# A valid user-supplied -DMSYS2_PREFIX=... remains an explicit override.
# Heal stale cache values automatically when the previous installation path
# no longer contains the ANTLR4 installation expected by this project.
get_filename_component(EMBX_COMPILER_BIN "${CMAKE_CXX_COMPILER}" DIRECTORY)
get_filename_component(EMBX_DEFAULT_MSYS2_PREFIX "${EMBX_COMPILER_BIN}/.." ABSOLUTE)
set(EMBX_DEFAULT_ANTLR4_JAR "${EMBX_DEFAULT_MSYS2_PREFIX}/share/java/antlr-4.13.2-complete.jar")
if(NOT DEFINED MSYS2_PREFIX OR NOT EXISTS "${MSYS2_PREFIX}/share/java/antlr-4.13.2-complete.jar")
  set(MSYS2_PREFIX "${EMBX_DEFAULT_MSYS2_PREFIX}" CACHE PATH "MSYS2 UCRT64 installation prefix" FORCE)
endif()
list(PREPEND CMAKE_PREFIX_PATH "${MSYS2_PREFIX}")
find_package(Catch2 3 CONFIG QUIET)
if(Catch2_FOUND)
  message(STATUS "Catch2: using system installation (${Catch2_VERSION})")
else()
  message(STATUS "Catch2: suitable system installation not found; using pinned fallback")
  include(FetchContent)
  set(CATCH_BUILD_TESTING OFF CACHE BOOL "" FORCE)
  set(CATCH_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
  set(CATCH_BUILD_EXTRA_TESTS OFF CACHE BOOL "" FORCE)
  set(CATCH_INSTALL_DOCS OFF CACHE BOOL "" FORCE)
  FetchContent_Declare(
    Catch2
    URL      https://github.com/catchorg/Catch2/archive/refs/tags/v3.15.3.tar.gz
    URL_HASH SHA256=b0299ae552918220a7a6e21e7de5b714777f4e8c883fb70c4bb23fe01df8c6e3
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
  )
  FetchContent_MakeAvailable(Catch2)
endif()

# Java is discovered from the active toolchain/environment; no machine-local JDK path is embedded.
find_package(Java 11 REQUIRED COMPONENTS Runtime)

if(NOT DEFINED ANTLR4_JAR_LOCATION OR NOT EXISTS "${ANTLR4_JAR_LOCATION}")
  set(ANTLR4_JAR_LOCATION "${MSYS2_PREFIX}/share/java/antlr-4.13.2-complete.jar" CACHE FILEPATH "ANTLR4 complete JAR" FORCE)
endif()
if(NOT EXISTS "${ANTLR4_JAR_LOCATION}")
  message(FATAL_ERROR "ANTLR4 JAR not found: ${ANTLR4_JAR_LOCATION}")
endif()
if(NOT DEFINED ANTLR4_RUNTIME_DIR OR NOT EXISTS "${ANTLR4_RUNTIME_DIR}/lib/libantlr4-runtime.dll.a")
  set(ANTLR4_RUNTIME_DIR "${MSYS2_PREFIX}" CACHE PATH "ANTLR4 runtime directory" FORCE)
endif()
find_library(ANTLR4_LIB NAMES antlr4-runtime PATHS "${ANTLR4_RUNTIME_DIR}/lib" NO_DEFAULT_PATH)
if(NOT ANTLR4_LIB)
  message(FATAL_ERROR "ANTLR4 runtime library not found in ${ANTLR4_RUNTIME_DIR}/lib")
endif()

set(GRAMMAR_FILE "${CMAKE_CURRENT_SOURCE_DIR}/grammar/EmbX.g4")
set(ANTLR4_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(ANTLR4_GENERATED_FILES
  "${ANTLR4_OUTPUT_DIR}/EmbXLexer.cpp" "${ANTLR4_OUTPUT_DIR}/EmbXLexer.h"
  "${ANTLR4_OUTPUT_DIR}/EmbXParser.cpp" "${ANTLR4_OUTPUT_DIR}/EmbXParser.h"
  "${ANTLR4_OUTPUT_DIR}/EmbXVisitor.h" "${ANTLR4_OUTPUT_DIR}/EmbXBaseVisitor.h")

add_custom_command(OUTPUT ${ANTLR4_GENERATED_FILES}
  COMMAND "${Java_JAVA_EXECUTABLE}" -jar "${ANTLR4_JAR_LOCATION}"
          -Dlanguage=Cpp -visitor -no-listener -Xexact-output-dir
          -o "${ANTLR4_OUTPUT_DIR}" "${GRAMMAR_FILE}"
  DEPENDS "${GRAMMAR_FILE}"
  COMMENT "Generating ANTLR4 EmbX parser" VERBATIM)
add_custom_target(GenerateAntlr DEPENDS ${ANTLR4_GENERATED_FILES})

# Generated ANTLR sources may contain intentionally unused callback parameters.
# Keep strict warnings for EmbX sources while suppressing this generated-only noise.
set_source_files_properties(
  "${ANTLR4_OUTPUT_DIR}/EmbXLexer.cpp"
  "${ANTLR4_OUTPUT_DIR}/EmbXParser.cpp"
  PROPERTIES COMPILE_OPTIONS "-Wno-unused-parameter")


foreach(required_source IN ITEMS
  "${CMAKE_CURRENT_SOURCE_DIR}/src/compiler/Compiler.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/plan/LayoutGraph.cpp")
  if(NOT EXISTS "${required_source}")
    message(FATAL_ERROR "Required EmbX source is missing: ${required_source}")
  endif()
endforeach()

set(EMBX_CORE_SOURCES
  src/compiler/Compiler.cpp
  src/parser/ParserDriver.cpp
  src/semantic/SemanticAnalyzer.cpp
  src/runtime/Runtime.cpp
  src/runtime/ExpressionEvaluator.cpp
  src/ir/IrBuilder.cpp
  src/plan/PlanBuilder.cpp
  src/plan/ArrayDescriptor.cpp
  src/plan/LayoutGraph.cpp
  src/decoder/Decoder.cpp
  src/encoder/Encoder.cpp
  src/value/Value.cpp
  src/diagnostics/Diagnostic.cpp
  src/reflection/Reflection.cpp
  src/codegen/CppGenerator.cpp
)

add_executable(embx
  src/main.cpp
  ${EMBX_CORE_SOURCES}
  ${ANTLR4_GENERATED_FILES})
add_dependencies(embx GenerateAntlr)
target_include_directories(embx PRIVATE
  "${ANTLR4_OUTPUT_DIR}" "${ANTLR4_RUNTIME_DIR}/include/antlr4-runtime" "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(embx PRIVATE "${ANTLR4_LIB}")
target_compile_definitions(embx PRIVATE EMBX_VERSION="${PROJECT_VERSION}")

add_executable(embx_runtime_test tests/runtime_test.cpp src/runtime/Runtime.cpp)
add_executable(embx_ast_test tests/ast_test.cpp)
target_include_directories(embx_ast_test PRIVATE src)
target_compile_options(embx_ast_test PRIVATE -Wall -Wextra -Wpedantic)
target_include_directories(embx_runtime_test PRIVATE src)

target_compile_options(embx PRIVATE -Wall -Wextra -Wpedantic)
# ANTLR runtime intentionally hides a virtual equality overload in RuleContext.
# This is generated/external code and not an EmbX diagnostic.
target_compile_options(embx PRIVATE -Wno-overloaded-virtual)
target_compile_options(embx_runtime_test PRIVATE -Wall -Wextra -Wpedantic)

add_executable(embx_semantic_ir_test tests/semantic_ir_test.cpp src/semantic/SemanticAnalyzer.cpp src/ir/IrBuilder.cpp src/parser/ParserDriver.cpp ${ANTLR4_GENERATED_FILES})
add_dependencies(embx_semantic_ir_test GenerateAntlr)
target_include_directories(embx_semantic_ir_test PRIVATE "${ANTLR4_OUTPUT_DIR}" "${ANTLR4_RUNTIME_DIR}/include/antlr4-runtime" src)
target_link_libraries(embx_semantic_ir_test PRIVATE "${ANTLR4_LIB}")
target_compile_options(embx_semantic_ir_test PRIVATE -Wall -Wextra -Wpedantic -Wno-overloaded-virtual)

enable_testing()

add_executable(embx_layout_size_64bit_test tests/layout_size_64bit_test.cpp src/runtime/ExpressionEvaluator.cpp)
target_include_directories(embx_layout_size_64bit_test PRIVATE src)
target_compile_options(embx_layout_size_64bit_test PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME layout_size_64bit_test COMMAND embx_layout_size_64bit_test)

add_executable(embx_array_buffer_contract_test tests/array_buffer_contract_test.cpp)
target_include_directories(embx_array_buffer_contract_test PRIVATE src)
target_compile_options(embx_array_buffer_contract_test PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME array_buffer_contract_test COMMAND embx_array_buffer_contract_test)

add_executable(embx_array_descriptor_plan_test
  tests/array_descriptor_plan_test.cpp
  src/plan/ArrayDescriptor.cpp src/plan/PlanBuilder.cpp src/plan/LayoutGraph.cpp
  src/runtime/ExpressionEvaluator.cpp)
target_include_directories(embx_array_descriptor_plan_test PRIVATE src)
target_compile_options(embx_array_descriptor_plan_test PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME array_descriptor_plan_test COMMAND embx_array_descriptor_plan_test)

add_executable(embx_array_descriptor_dynamic_nested_test
  tests/array_descriptor_dynamic_nested_test.cpp
  src/plan/ArrayDescriptor.cpp src/plan/PlanBuilder.cpp src/plan/LayoutGraph.cpp
  src/runtime/Runtime.cpp src/runtime/ExpressionEvaluator.cpp src/value/Value.cpp
  src/diagnostics/Diagnostic.cpp)
target_include_directories(embx_array_descriptor_dynamic_nested_test PRIVATE src)
target_compile_options(embx_array_descriptor_dynamic_nested_test PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME array_descriptor_dynamic_nested_test COMMAND embx_array_descriptor_dynamic_nested_test)

add_executable(embx_array_conformance_test tests/array_conformance_test.cpp)
target_include_directories(embx_array_conformance_test PRIVATE src)
target_compile_options(embx_array_conformance_test PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME array_conformance_test COMMAND embx_array_conformance_test)

add_test(NAME runtime_test COMMAND embx_runtime_test)
add_executable(embx_expression_test tests/expression_test.cpp src/runtime/ExpressionEvaluator.cpp)
target_include_directories(embx_expression_test PRIVATE src)
target_compile_options(embx_expression_test PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME expression_test COMMAND embx_expression_test)

add_test(NAME ast_structural_test COMMAND embx_ast_test)
add_test(NAME semantic_ir_test COMMAND embx_semantic_ir_test)
add_executable(embx_semantic_negative_test tests/semantic_negative_test.cpp src/semantic/SemanticAnalyzer.cpp src/parser/ParserDriver.cpp ${ANTLR4_GENERATED_FILES})
add_dependencies(embx_semantic_negative_test GenerateAntlr)
target_include_directories(embx_semantic_negative_test PRIVATE "${ANTLR4_OUTPUT_DIR}" "${ANTLR4_RUNTIME_DIR}/include/antlr4-runtime" src)
target_link_libraries(embx_semantic_negative_test PRIVATE "${ANTLR4_LIB}")
target_compile_options(embx_semantic_negative_test PRIVATE -Wall -Wextra -Wpedantic -Wno-overloaded-virtual)
add_test(NAME semantic_negative_test COMMAND embx_semantic_negative_test)
add_executable(embx_ir_structural_test tests/ir_structural_test.cpp src/ir/IrBuilder.cpp src/semantic/SemanticAnalyzer.cpp src/parser/ParserDriver.cpp ${ANTLR4_GENERATED_FILES})
add_dependencies(embx_ir_structural_test GenerateAntlr)
target_include_directories(embx_ir_structural_test PRIVATE "${ANTLR4_OUTPUT_DIR}" "${ANTLR4_RUNTIME_DIR}/include/antlr4-runtime" src)
target_link_libraries(embx_ir_structural_test PRIVATE "${ANTLR4_LIB}")
target_compile_options(embx_ir_structural_test PRIVATE -Wall -Wextra -Wpedantic -Wno-overloaded-virtual)
add_test(NAME ir_structural_test COMMAND embx_ir_structural_test)
add_executable(embx_plan_test tests/plan_test.cpp src/plan/PlanBuilder.cpp src/plan/LayoutGraph.cpp src/runtime/ExpressionEvaluator.cpp src/ir/IrBuilder.cpp src/semantic/SemanticAnalyzer.cpp src/parser/ParserDriver.cpp ${ANTLR4_GENERATED_FILES})
add_dependencies(embx_plan_test GenerateAntlr)
target_include_directories(embx_plan_test PRIVATE "${ANTLR4_OUTPUT_DIR}" "${ANTLR4_RUNTIME_DIR}/include/antlr4-runtime" src)
target_link_libraries(embx_plan_test PRIVATE "${ANTLR4_LIB}")
target_compile_options(embx_plan_test PRIVATE -Wall -Wextra -Wpedantic -Wno-overloaded-virtual)
add_test(NAME plan_test COMMAND embx_plan_test)
add_executable(embx_plan_executable_contract_test tests/plan_executable_contract_test.cpp src/plan/PlanBuilder.cpp src/plan/LayoutGraph.cpp src/runtime/ExpressionEvaluator.cpp src/ir/IrBuilder.cpp src/semantic/SemanticAnalyzer.cpp src/parser/ParserDriver.cpp ${ANTLR4_GENERATED_FILES})
add_dependencies(embx_plan_executable_contract_test GenerateAntlr)
target_include_directories(embx_plan_executable_contract_test PRIVATE "${ANTLR4_OUTPUT_DIR}" "${ANTLR4_RUNTIME_DIR}/include/antlr4-runtime" src)
target_link_libraries(embx_plan_executable_contract_test PRIVATE "${ANTLR4_LIB}")
target_compile_options(embx_plan_executable_contract_test PRIVATE -Wall -Wextra -Wpedantic -Wno-overloaded-virtual)
add_test(NAME plan_executable_contract_test COMMAND embx_plan_executable_contract_test)
add_executable(embx_runtime_lookup_audit_test tests/runtime_lookup_audit_test.cpp
  ${EMBX_CORE_SOURCES}
  ${ANTLR4_OUTPUT_DIR}/EmbXLexer.cpp
  ${ANTLR4_OUTPUT_DIR}/EmbXParser.cpp
)
add_dependencies(embx_runtime_lookup_audit_test GenerateAntlr)
target_include_directories(embx_runtime_lookup_audit_test PRIVATE "${ANTLR4_OUTPUT_DIR}" "${ANTLR4_RUNTIME_DIR}/include/antlr4-runtime" src)
target_link_libraries(embx_runtime_lookup_audit_test PRIVATE "${ANTLR4_LIB}")
target_compile_options(embx_runtime_lookup_audit_test PRIVATE -Wall -Wextra -Wpedantic -Wno-overloaded-virtual)
add_test(NAME runtime_lookup_audit_test COMMAND embx_runtime_lookup_audit_test)

add_executable(embx_plan_execution_independence_test tests/plan_execution_independence_test.cpp
  src/plan/PlanBuilder.cpp src/plan/LayoutGraph.cpp src/ir/IrBuilder.cpp
  src/semantic/SemanticAnalyzer.cpp src/parser/ParserDriver.cpp
  src/runtime/ExpressionEvaluator.cpp src/runtime/Runtime.cpp
  src/encoder/Encoder.cpp src/decoder/Decoder.cpp src/value/Value.cpp
  src/diagnostics/Diagnostic.cpp ${ANTLR4_GENERATED_FILES})
add_dependencies(embx_plan_execution_independence_test GenerateAntlr)
target_include_directories(embx_plan_execution_independence_test PRIVATE "${ANTLR4_OUTPUT_DIR}" "${ANTLR4_RUNTIME_DIR}/include/antlr4-runtime" src)
target_link_libraries(embx_plan_execution_independence_test PRIVATE "${ANTLR4_LIB}")
target_compile_options(embx_plan_execution_independence_test PRIVATE -Wall -Wextra -Wpedantic -Wno-overloaded-virtual)
add_test(NAME plan_execution_independence_test COMMAND embx_plan_execution_independence_test)
add_executable(embx_dynamic_expression_determinism_audit_test tests/dynamic_expression_determinism_audit_test.cpp
  src/plan/PlanBuilder.cpp src/plan/LayoutGraph.cpp src/ir/IrBuilder.cpp
  src/semantic/SemanticAnalyzer.cpp src/parser/ParserDriver.cpp
  src/runtime/ExpressionEvaluator.cpp src/runtime/Runtime.cpp
  src/encoder/Encoder.cpp src/decoder/Decoder.cpp src/value/Value.cpp
  src/diagnostics/Diagnostic.cpp ${ANTLR4_GENERATED_FILES})
add_dependencies(embx_dynamic_expression_determinism_audit_test GenerateAntlr)
target_include_directories(embx_dynamic_expression_determinism_audit_test PRIVATE "${ANTLR4_OUTPUT_DIR}" "${ANTLR4_RUNTIME_DIR}/include/antlr4-runtime" src)
target_link_libraries(embx_dynamic_expression_determinism_audit_test PRIVATE "${ANTLR4_LIB}")
target_compile_options(embx_dynamic_expression_determinism_audit_test PRIVATE -Wall -Wextra -Wpedantic -Wno-overloaded-virtual)
add_test(NAME dynamic_expression_determinism_audit_test COMMAND embx_dynamic_expression_determinism_audit_test)
add_executable(embx_plan_negative_test tests/plan_negative_test.cpp src/plan/PlanBuilder.cpp src/plan/LayoutGraph.cpp src/runtime/ExpressionEvaluator.cpp src/ir/IrBuilder.cpp src/semantic/SemanticAnalyzer.cpp src/parser/ParserDriver.cpp ${ANTLR4_GENERATED_FILES})
add_dependencies(embx_plan_negative_test GenerateAntlr)
target_include_directories(embx_plan_negative_test PRIVATE "${ANTLR4_OUTPUT_DIR}" "${ANTLR4_RUNTIME_DIR}/include/antlr4-runtime" src)
target_link_libraries(embx_plan_negative_test PRIVATE "${ANTLR4_LIB}")
target_compile_options(embx_plan_negative_test PRIVATE -Wall -Wextra -Wpedantic -Wno-overloaded-virtual)
add_test(NAME plan_negative_test COMMAND embx_plan_negative_test)

add_executable(embx_decoder_test tests/decoder_test.cpp src/decoder/Decoder.cpp src/runtime/Runtime.cpp src/runtime/ExpressionEvaluator.cpp src/value/Value.cpp src/diagnostics/Diagnostic.cpp)
target_include_directories(embx_decoder_test PRIVATE src)
target_compile_options(embx_decoder_test PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME decoder_test COMMAND embx_decoder_test)

add_executable(embx_encoder_test tests/encoder_test.cpp src/encoder/Encoder.cpp src/decoder/Decoder.cpp src/runtime/Runtime.cpp src/runtime/ExpressionEvaluator.cpp src/value/Value.cpp src/diagnostics/Diagnostic.cpp)
target_include_directories(embx_encoder_test PRIVATE src)
target_compile_options(embx_encoder_test PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME encoder_test COMMAND embx_encoder_test)
add_executable(embx_composite_type_test tests/composite_type_test.cpp src/encoder/Encoder.cpp src/decoder/Decoder.cpp src/runtime/Runtime.cpp src/runtime/ExpressionEvaluator.cpp src/value/Value.cpp src/diagnostics/Diagnostic.cpp src/plan/ArrayDescriptor.cpp src/plan/PlanBuilder.cpp src/plan/LayoutGraph.cpp)
target_include_directories(embx_composite_type_test PRIVATE src)
target_compile_options(embx_composite_type_test PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME composite_type_test COMMAND embx_composite_type_test)

add_executable(embx_nested_composite_test tests/nested_composite_test.cpp src/encoder/Encoder.cpp src/decoder/Decoder.cpp src/runtime/Runtime.cpp src/runtime/ExpressionEvaluator.cpp src/value/Value.cpp src/diagnostics/Diagnostic.cpp)
target_include_directories(embx_nested_composite_test PRIVATE src)
target_compile_options(embx_nested_composite_test PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME nested_composite_test COMMAND embx_nested_composite_test)
add_executable(embx_dynamic_layout_test tests/dynamic_layout_test.cpp src/encoder/Encoder.cpp src/decoder/Decoder.cpp src/runtime/Runtime.cpp src/runtime/ExpressionEvaluator.cpp src/value/Value.cpp src/diagnostics/Diagnostic.cpp src/parser/ParserDriver.cpp src/semantic/SemanticAnalyzer.cpp src/ir/IrBuilder.cpp src/plan/PlanBuilder.cpp src/plan/LayoutGraph.cpp ${ANTLR4_GENERATED_FILES})
add_dependencies(embx_dynamic_layout_test GenerateAntlr)
target_include_directories(embx_dynamic_layout_test PRIVATE "${ANTLR4_OUTPUT_DIR}" "${ANTLR4_RUNTIME_DIR}/include/antlr4-runtime" src)
target_link_libraries(embx_dynamic_layout_test PRIVATE "${ANTLR4_LIB}")
target_compile_options(embx_dynamic_layout_test PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME dynamic_layout_test COMMAND embx_dynamic_layout_test)

add_executable(embx_value_reflection_test tests/value_reflection_test.cpp src/value/Value.cpp src/diagnostics/Diagnostic.cpp src/reflection/Reflection.cpp)
target_include_directories(embx_value_reflection_test PRIVATE src)
target_compile_options(embx_value_reflection_test PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME value_reflection_test COMMAND embx_value_reflection_test)

add_executable(embx_diagnostics_pipeline_test tests/diagnostics_pipeline_test.cpp
  src/decoder/Decoder.cpp src/encoder/Encoder.cpp src/runtime/Runtime.cpp
  src/runtime/ExpressionEvaluator.cpp
  src/value/Value.cpp src/diagnostics/Diagnostic.cpp)
target_include_directories(embx_diagnostics_pipeline_test PRIVATE src)
target_compile_options(embx_diagnostics_pipeline_test PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME diagnostics_pipeline_test COMMAND embx_diagnostics_pipeline_test)
add_executable(embx_layout_graph_test tests/layout_graph_test.cpp src/plan/LayoutGraph.cpp src/plan/PlanBuilder.cpp src/ir/IrBuilder.cpp src/semantic/SemanticAnalyzer.cpp src/parser/ParserDriver.cpp src/runtime/ExpressionEvaluator.cpp src/value/Value.cpp src/diagnostics/Diagnostic.cpp ${ANTLR4_GENERATED_FILES})
add_dependencies(embx_layout_graph_test GenerateAntlr)
target_include_directories(embx_layout_graph_test PRIVATE "${ANTLR4_OUTPUT_DIR}" "${ANTLR4_RUNTIME_DIR}/include/antlr4-runtime" src)
target_link_libraries(embx_layout_graph_test PRIVATE "${ANTLR4_LIB}")
target_compile_options(embx_layout_graph_test PRIVATE -Wall -Wextra -Wpedantic -Wno-overloaded-virtual)
add_test(NAME layout_graph_test COMMAND embx_layout_graph_test)
add_executable(embx_dependency_validation_test tests/dependency_validation_test.cpp src/plan/PlanBuilder.cpp src/plan/LayoutGraph.cpp src/ir/IrBuilder.cpp src/semantic/SemanticAnalyzer.cpp src/parser/ParserDriver.cpp src/runtime/ExpressionEvaluator.cpp src/value/Value.cpp src/diagnostics/Diagnostic.cpp ${ANTLR4_GENERATED_FILES})
add_dependencies(embx_dependency_validation_test GenerateAntlr)
target_include_directories(embx_dependency_validation_test PRIVATE "${ANTLR4_OUTPUT_DIR}" "${ANTLR4_RUNTIME_DIR}/include/antlr4-runtime" src)
target_link_libraries(embx_dependency_validation_test PRIVATE "${ANTLR4_LIB}")
target_compile_options(embx_dependency_validation_test PRIVATE -Wall -Wextra -Wpedantic -Wno-overloaded-virtual)
add_test(NAME dependency_validation_test COMMAND embx_dependency_validation_test)


add_executable(embx_foundation_hardening_test tests/foundation_hardening_test.cpp
  src/decoder/Decoder.cpp src/encoder/Encoder.cpp src/runtime/Runtime.cpp
  src/runtime/ExpressionEvaluator.cpp src/value/Value.cpp
  src/diagnostics/Diagnostic.cpp src/reflection/Reflection.cpp)
target_include_directories(embx_foundation_hardening_test PRIVATE src)
target_compile_options(embx_foundation_hardening_test PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME foundation_hardening_test COMMAND embx_foundation_hardening_test)

message(STATUS "================ EmbX ${PROJECT_VERSION} ================")
message(STATUS "MSYS2:     ${MSYS2_PREFIX}")
message(STATUS "Java:      ${Java_JAVA_EXECUTABLE}")
message(STATUS "Java:      ${Java_VERSION_STRING}")
message(STATUS "ANTLR JAR: ${ANTLR4_JAR_LOCATION}")
message(STATUS "ANTLR lib: ${ANTLR4_LIB}")
message(STATUS "Grammar:   ${GRAMMAR_FILE}")
message(STATUS "Generated: ${ANTLR4_OUTPUT_DIR}")
message(STATUS "=========================================================")

add_executable(embx_namespace_import_test tests/namespace_import_test.cpp
  src/semantic/SemanticAnalyzer.cpp src/ir/IrBuilder.cpp src/parser/ParserDriver.cpp ${ANTLR4_GENERATED_FILES})
add_dependencies(embx_namespace_import_test GenerateAntlr)
target_include_directories(embx_namespace_import_test PRIVATE "${ANTLR4_OUTPUT_DIR}" "${ANTLR4_RUNTIME_DIR}/include/antlr4-runtime" src)
target_link_libraries(embx_namespace_import_test PRIVATE "${ANTLR4_LIB}")
target_compile_options(embx_namespace_import_test PRIVATE -Wall -Wextra -Wpedantic -Wno-overloaded-virtual)
add_test(NAME namespace_import_test COMMAND embx_namespace_import_test)

add_executable(embx_type_system_test tests/type_system_test.cpp
  src/plan/PlanBuilder.cpp src/runtime/ExpressionEvaluator.cpp src/ir/IrBuilder.cpp
  src/plan/LayoutGraph.cpp
  src/semantic/SemanticAnalyzer.cpp src/parser/ParserDriver.cpp ${ANTLR4_GENERATED_FILES})
add_dependencies(embx_type_system_test GenerateAntlr)
target_include_directories(embx_type_system_test PRIVATE "${ANTLR4_OUTPUT_DIR}" "${ANTLR4_RUNTIME_DIR}/include/antlr4-runtime" src)
target_link_libraries(embx_type_system_test PRIVATE "${ANTLR4_LIB}")
target_compile_options(embx_type_system_test PRIVATE -Wall -Wextra -Wpedantic -Wno-overloaded-virtual)
add_test(NAME type_system_test COMMAND embx_type_system_test)

add_executable(embx_enum_test tests/enum_test.cpp
    src/plan/PlanBuilder.cpp src/runtime/ExpressionEvaluator.cpp src/ir/IrBuilder.cpp
    src/plan/LayoutGraph.cpp
    src/semantic/SemanticAnalyzer.cpp src/parser/ParserDriver.cpp ${ANTLR4_GENERATED_FILES})
add_dependencies(embx_enum_test GenerateAntlr)
target_include_directories(embx_enum_test PRIVATE "${ANTLR4_OUTPUT_DIR}" "${ANTLR4_RUNTIME_DIR}/include/antlr4-runtime" src)
target_link_libraries(embx_enum_test PRIVATE "${ANTLR4_LIB}")
target_compile_options(embx_enum_test PRIVATE -Wall -Wextra -Wpedantic -Wno-overloaded-virtual)
add_test(NAME enum_test COMMAND embx_enum_test)

add_executable(embx_attribute_test tests/attribute_test.cpp
  src/plan/PlanBuilder.cpp src/runtime/ExpressionEvaluator.cpp src/ir/IrBuilder.cpp
  src/plan/LayoutGraph.cpp
  src/semantic/SemanticAnalyzer.cpp src/parser/ParserDriver.cpp
  src/reflection/Reflection.cpp ${ANTLR4_GENERATED_FILES})
add_dependencies(embx_attribute_test GenerateAntlr)
target_include_directories(embx_attribute_test PRIVATE "${ANTLR4_OUTPUT_DIR}" "${ANTLR4_RUNTIME_DIR}/include/antlr4-runtime" src)
target_link_libraries(embx_attribute_test PRIVATE "${ANTLR4_LIB}")
target_compile_options(embx_attribute_test PRIVATE -Wall -Wextra -Wpedantic -Wno-overloaded-virtual)
add_test(NAME attribute_test COMMAND embx_attribute_test)

add_executable(embx_documentation_test tests/documentation_test.cpp
  src/reflection/Reflection.cpp src/plan/PlanBuilder.cpp src/runtime/ExpressionEvaluator.cpp
  src/plan/LayoutGraph.cpp
  src/ir/IrBuilder.cpp src/semantic/SemanticAnalyzer.cpp
  src/parser/ParserDriver.cpp ${ANTLR4_GENERATED_FILES})
add_dependencies(embx_documentation_test GenerateAntlr)
target_include_directories(embx_documentation_test PRIVATE "${ANTLR4_OUTPUT_DIR}" "${ANTLR4_RUNTIME_DIR}/include/antlr4-runtime" src)
target_link_libraries(embx_documentation_test PRIVATE "${ANTLR4_LIB}")
target_compile_options(embx_documentation_test PRIVATE -Wall -Wextra -Wpedantic -Wno-overloaded-virtual)
add_test(NAME documentation_test COMMAND embx_documentation_test)

add_executable(embx_type_system_wp2_test tests/type_system_wp2_test.cpp
  src/semantic/SemanticAnalyzer.cpp src/parser/ParserDriver.cpp ${ANTLR4_GENERATED_FILES})
add_dependencies(embx_type_system_wp2_test GenerateAntlr)
target_include_directories(embx_type_system_wp2_test PRIVATE "${ANTLR4_OUTPUT_DIR}" "${ANTLR4_RUNTIME_DIR}/include/antlr4-runtime" src)
target_link_libraries(embx_type_system_wp2_test PRIVATE "${ANTLR4_LIB}")
target_compile_options(embx_type_system_wp2_test PRIVATE -Wall -Wextra -Wpedantic -Wno-overloaded-virtual)
add_test(NAME type_system_wp2_test COMMAND embx_type_system_wp2_test)

add_executable(embx_semantic_edge_cases_test tests/semantic_edge_cases_test.cpp
  src/decoder/Decoder.cpp src/encoder/Encoder.cpp src/runtime/Runtime.cpp
  src/runtime/ExpressionEvaluator.cpp src/value/Value.cpp
  src/diagnostics/Diagnostic.cpp src/semantic/SemanticAnalyzer.cpp
  src/parser/ParserDriver.cpp ${ANTLR4_GENERATED_FILES})
add_dependencies(embx_semantic_edge_cases_test GenerateAntlr)
target_include_directories(embx_semantic_edge_cases_test PRIVATE
  "${ANTLR4_OUTPUT_DIR}" "${ANTLR4_RUNTIME_DIR}/include/antlr4-runtime" src)
target_link_libraries(embx_semantic_edge_cases_test PRIVATE "${ANTLR4_LIB}")
target_compile_options(embx_semantic_edge_cases_test PRIVATE -Wall -Wextra -Wpedantic -Wno-overloaded-virtual)
add_test(NAME semantic_edge_cases_test COMMAND embx_semantic_edge_cases_test)

add_executable(embx_runtime_hardening_test tests/runtime_hardening_test.cpp
  src/runtime/Runtime.cpp src/encoder/Encoder.cpp src/runtime/ExpressionEvaluator.cpp
  src/value/Value.cpp src/diagnostics/Diagnostic.cpp)
target_include_directories(embx_runtime_hardening_test PRIVATE src)
target_compile_options(embx_runtime_hardening_test PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME runtime_hardening_test COMMAND embx_runtime_hardening_test)

add_executable(embx_codec_symmetry_test tests/codec_symmetry_test.cpp
  src/encoder/Encoder.cpp src/decoder/Decoder.cpp src/runtime/Runtime.cpp
  src/runtime/ExpressionEvaluator.cpp src/value/Value.cpp
  src/diagnostics/Diagnostic.cpp)
target_include_directories(embx_codec_symmetry_test PRIVATE src)
target_compile_options(embx_codec_symmetry_test PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME codec_symmetry_test COMMAND embx_codec_symmetry_test)

add_executable(embx_reflection_contract_test tests/reflection_contract_test.cpp
  src/reflection/Reflection.cpp)
target_include_directories(embx_reflection_contract_test PRIVATE src)
target_compile_options(embx_reflection_contract_test PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME reflection_contract_test COMMAND embx_reflection_contract_test)

# Release-readiness CLI contract: stable help/version/error behavior.
# A CMake script validates exit codes and both output streams directly.
add_test(NAME cli_help_test COMMAND ${CMAKE_COMMAND}
  -DEMBX_EXECUTABLE=$<TARGET_FILE:embx>
  -DEXPECTED_VERSION=${PROJECT_VERSION}
  -DCLI_CHECK=help
  -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/cli_contract.cmake
)
add_test(NAME cli_version_test COMMAND ${CMAKE_COMMAND}
  -DEMBX_EXECUTABLE=$<TARGET_FILE:embx>
  -DEXPECTED_VERSION=${PROJECT_VERSION}
  -DCLI_CHECK=version
  -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/cli_contract.cmake
)
add_test(NAME cli_unknown_option_test COMMAND ${CMAKE_COMMAND}
  -DEMBX_EXECUTABLE=$<TARGET_FILE:embx>
  -DEXPECTED_VERSION=${PROJECT_VERSION}
  -DCLI_CHECK=unknown
  -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/cli_contract.cmake
)
# All EmbX tests use Catch2's supplied main and assertion/reporting machinery.
set(EMBX_TEST_TARGETS
  embx_layout_size_64bit_test
  embx_runtime_test
  embx_ast_test
  embx_semantic_ir_test
  embx_expression_test
  embx_semantic_negative_test
  embx_ir_structural_test
  embx_plan_test
  embx_plan_executable_contract_test
  embx_runtime_lookup_audit_test
  embx_plan_execution_independence_test
  embx_dynamic_expression_determinism_audit_test
  embx_plan_negative_test
  embx_decoder_test
  embx_encoder_test
  embx_composite_type_test
  embx_nested_composite_test
  embx_dynamic_layout_test
  embx_value_reflection_test
  embx_diagnostics_pipeline_test
  embx_layout_graph_test
  embx_dependency_validation_test
  embx_foundation_hardening_test
  embx_namespace_import_test
  embx_type_system_test
  embx_enum_test
  embx_attribute_test
  embx_documentation_test
  embx_type_system_wp2_test
  embx_semantic_edge_cases_test
  embx_runtime_hardening_test
  embx_codec_symmetry_test
  embx_reflection_contract_test
)
foreach(test_target IN LISTS EMBX_TEST_TARGETS)
  target_link_libraries(${test_target} PRIVATE Catch2::Catch2WithMain)
endforeach()


add_executable(embx_cpp_generator_test tests/cpp_generator_test.cpp
  src/codegen/CppGenerator.cpp src/plan/PlanBuilder.cpp src/runtime/ExpressionEvaluator.cpp
  src/plan/LayoutGraph.cpp
  src/ir/IrBuilder.cpp src/semantic/SemanticAnalyzer.cpp
  src/parser/ParserDriver.cpp ${ANTLR4_GENERATED_FILES})
add_dependencies(embx_cpp_generator_test GenerateAntlr)
target_include_directories(embx_cpp_generator_test PRIVATE "${ANTLR4_OUTPUT_DIR}" "${ANTLR4_RUNTIME_DIR}/include/antlr4-runtime" src)
target_link_libraries(embx_cpp_generator_test PRIVATE "${ANTLR4_LIB}" Catch2::Catch2WithMain)
target_compile_options(embx_cpp_generator_test PRIVATE -Wall -Wextra -Wpedantic -Wno-overloaded-virtual)
add_test(NAME cpp_generator_test COMMAND embx_cpp_generator_test)

add_executable(embx_generated_codec_test tests/generated_codec_test.cpp
  src/codegen/CppGenerator.cpp src/plan/PlanBuilder.cpp src/runtime/ExpressionEvaluator.cpp
  src/plan/LayoutGraph.cpp
  src/ir/IrBuilder.cpp src/semantic/SemanticAnalyzer.cpp
  src/parser/ParserDriver.cpp ${ANTLR4_GENERATED_FILES})
add_dependencies(embx_generated_codec_test GenerateAntlr)
target_include_directories(embx_generated_codec_test PRIVATE "${ANTLR4_OUTPUT_DIR}" "${ANTLR4_RUNTIME_DIR}/include/antlr4-runtime" src)
target_link_libraries(embx_generated_codec_test PRIVATE "${ANTLR4_LIB}" Catch2::Catch2WithMain)
target_compile_options(embx_generated_codec_test PRIVATE -Wall -Wextra -Wpedantic -Wno-overloaded-virtual)
add_test(NAME generated_codec_test COMMAND embx_generated_codec_test)

add_executable(embx_generated_dynamic_codec_test tests/generated_dynamic_codec_test.cpp
  src/codegen/CppGenerator.cpp src/plan/PlanBuilder.cpp src/runtime/ExpressionEvaluator.cpp
  src/plan/LayoutGraph.cpp
  src/ir/IrBuilder.cpp src/semantic/SemanticAnalyzer.cpp
  src/parser/ParserDriver.cpp ${ANTLR4_GENERATED_FILES})
add_dependencies(embx_generated_dynamic_codec_test GenerateAntlr)
target_include_directories(embx_generated_dynamic_codec_test PRIVATE "${ANTLR4_OUTPUT_DIR}" "${ANTLR4_RUNTIME_DIR}/include/antlr4-runtime" src)
target_link_libraries(embx_generated_dynamic_codec_test PRIVATE "${ANTLR4_LIB}" Catch2::Catch2WithMain)
target_compile_options(embx_generated_dynamic_codec_test PRIVATE -Wall -Wextra -Wpedantic -Wno-overloaded-virtual)
add_test(NAME generated_dynamic_codec_test COMMAND embx_generated_dynamic_codec_test)

add_executable(embx_generated_nested_types_test tests/generated_nested_types_test.cpp
  src/codegen/CppGenerator.cpp src/plan/PlanBuilder.cpp src/runtime/ExpressionEvaluator.cpp
  src/plan/LayoutGraph.cpp
  src/ir/IrBuilder.cpp src/semantic/SemanticAnalyzer.cpp
  src/parser/ParserDriver.cpp ${ANTLR4_GENERATED_FILES})
add_dependencies(embx_generated_nested_types_test GenerateAntlr)
target_include_directories(embx_generated_nested_types_test PRIVATE "${ANTLR4_OUTPUT_DIR}" "${ANTLR4_RUNTIME_DIR}/include/antlr4-runtime" src)
target_link_libraries(embx_generated_nested_types_test PRIVATE "${ANTLR4_LIB}" Catch2::Catch2WithMain)
target_compile_options(embx_generated_nested_types_test PRIVATE -Wall -Wextra -Wpedantic -Wno-overloaded-virtual)
add_test(NAME generated_nested_types_test COMMAND embx_generated_nested_types_test)

add_executable(embx_generated_callback_test tests/generated_callback_test.cpp
  src/codegen/CppGenerator.cpp src/plan/PlanBuilder.cpp src/runtime/ExpressionEvaluator.cpp
  src/plan/LayoutGraph.cpp
  src/ir/IrBuilder.cpp src/semantic/SemanticAnalyzer.cpp
  src/parser/ParserDriver.cpp ${ANTLR4_GENERATED_FILES})
add_dependencies(embx_generated_callback_test GenerateAntlr)
target_include_directories(embx_generated_callback_test PRIVATE "${ANTLR4_OUTPUT_DIR}" "${ANTLR4_RUNTIME_DIR}/include/antlr4-runtime" src)
target_link_libraries(embx_generated_callback_test PRIVATE "${ANTLR4_LIB}" Catch2::Catch2WithMain)
target_compile_options(embx_generated_callback_test PRIVATE -Wall -Wextra -Wpedantic -Wno-overloaded-virtual)
add_test(NAME generated_callback_test COMMAND embx_generated_callback_test)

add_executable(embx_encoder_callback_symmetry_test
  tests/encoder_callback_symmetry_test.cpp
  src/encoder/Encoder.cpp
  src/runtime/Runtime.cpp
  src/runtime/ExpressionEvaluator.cpp
  src/value/Value.cpp
  src/diagnostics/Diagnostic.cpp
)
target_include_directories(embx_encoder_callback_symmetry_test PRIVATE src)
target_link_libraries(embx_encoder_callback_symmetry_test PRIVATE Catch2::Catch2WithMain)
target_compile_features(embx_encoder_callback_symmetry_test PRIVATE cxx_std_17)
target_compile_options(embx_encoder_callback_symmetry_test PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME encoder_callback_symmetry_test COMMAND embx_encoder_callback_symmetry_test)

add_executable(embx_compiler_pipeline_test tests/compiler_pipeline_test.cpp
  src/compiler/Compiler.cpp src/codegen/CppGenerator.cpp src/plan/PlanBuilder.cpp
  src/plan/LayoutGraph.cpp
  src/ir/IrBuilder.cpp src/semantic/SemanticAnalyzer.cpp
  src/parser/ParserDriver.cpp src/runtime/ExpressionEvaluator.cpp
  src/value/Value.cpp src/diagnostics/Diagnostic.cpp ${ANTLR4_GENERATED_FILES})
add_dependencies(embx_compiler_pipeline_test GenerateAntlr)
target_include_directories(embx_compiler_pipeline_test PRIVATE "${ANTLR4_OUTPUT_DIR}" "${ANTLR4_RUNTIME_DIR}/include/antlr4-runtime" src)
target_link_libraries(embx_compiler_pipeline_test PRIVATE "${ANTLR4_LIB}" Catch2::Catch2WithMain)
target_compile_options(embx_compiler_pipeline_test PRIVATE -Wall -Wextra -Wpedantic -Wno-overloaded-virtual)
add_test(NAME compiler_pipeline_test COMMAND embx_compiler_pipeline_test)

add_executable(embx_core_expression_test tests/core_expression_test.cpp src/runtime/ExpressionEvaluator.cpp)
add_executable(embx_expression_type_test tests/expression_type_test.cpp)
target_include_directories(embx_core_expression_test PRIVATE src)
target_compile_options(embx_core_expression_test PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME core_expression_test COMMAND embx_core_expression_test)
target_include_directories(embx_expression_type_test PRIVATE src)
target_link_libraries(embx_expression_type_test PRIVATE Catch2::Catch2WithMain)
target_compile_features(embx_expression_type_test PRIVATE cxx_std_17)
target_compile_options(embx_expression_type_test PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME expression_type_test COMMAND embx_expression_type_test)

add_executable(embx_core_type_test tests/core_type_test.cpp)
target_include_directories(embx_core_type_test PRIVATE src)
target_link_libraries(embx_core_type_test PRIVATE Catch2::Catch2WithMain)
target_compile_features(embx_core_type_test PRIVATE cxx_std_17)
target_compile_options(embx_core_type_test PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME core_type_test COMMAND embx_core_type_test)

add_executable(embx_symbol_identity_test
  tests/symbol_identity_test.cpp
  src/ir/IrBuilder.cpp
  src/parser/ParserDriver.cpp
  src/semantic/SemanticAnalyzer.cpp
  ${ANTLR4_GENERATED_FILES})
add_dependencies(embx_symbol_identity_test GenerateAntlr)
target_include_directories(embx_symbol_identity_test PRIVATE
  "${ANTLR4_OUTPUT_DIR}" "${ANTLR4_RUNTIME_DIR}/include/antlr4-runtime" src)
target_link_libraries(embx_symbol_identity_test PRIVATE Catch2::Catch2WithMain "${ANTLR4_LIB}")
target_compile_features(embx_symbol_identity_test PRIVATE cxx_std_17)
target_compile_options(embx_symbol_identity_test PRIVATE -Wall -Wextra -Wpedantic -Wno-overloaded-virtual)
add_test(NAME symbol_identity_test COMMAND embx_symbol_identity_test)

add_executable(embx_plan_symbol_identity_test tests/plan_symbol_identity_test.cpp src/plan/PlanBuilder.cpp src/plan/LayoutGraph.cpp src/runtime/ExpressionEvaluator.cpp)
target_include_directories(embx_plan_symbol_identity_test PRIVATE src)
target_link_libraries(embx_plan_symbol_identity_test PRIVATE Catch2::Catch2WithMain)
target_compile_features(embx_plan_symbol_identity_test PRIVATE cxx_std_17)
target_compile_options(embx_plan_symbol_identity_test PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME plan_symbol_identity_test COMMAND embx_plan_symbol_identity_test)

add_executable(embx_symbol_resolution_contract_test tests/symbol_resolution_contract_test.cpp src/runtime/ExpressionEvaluator.cpp)
target_include_directories(embx_symbol_resolution_contract_test PRIVATE src)
target_link_libraries(embx_symbol_resolution_contract_test PRIVATE Catch2::Catch2WithMain)
target_compile_features(embx_symbol_resolution_contract_test PRIVATE cxx_std_17)
target_compile_options(embx_symbol_resolution_contract_test PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME symbol_resolution_contract_test COMMAND embx_symbol_resolution_contract_test)

add_executable(embx_symbol_identity_codegen_test tests/symbol_identity_codegen_test.cpp
  src/codegen/CppGenerator.cpp src/plan/PlanBuilder.cpp src/plan/LayoutGraph.cpp src/runtime/ExpressionEvaluator.cpp)
target_include_directories(embx_symbol_identity_codegen_test PRIVATE src)
target_link_libraries(embx_symbol_identity_codegen_test PRIVATE Catch2::Catch2WithMain)
target_compile_features(embx_symbol_identity_codegen_test PRIVATE cxx_std_17)
target_compile_options(embx_symbol_identity_codegen_test PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME symbol_identity_codegen_test COMMAND embx_symbol_identity_codegen_test)

add_executable(embx_name_resolution_plan_test tests/name_resolution_plan_test.cpp
    src/encoder/Encoder.cpp src/decoder/Decoder.cpp src/runtime/Runtime.cpp
    src/runtime/ExpressionEvaluator.cpp src/value/Value.cpp
    src/diagnostics/Diagnostic.cpp src/plan/PlanBuilder.cpp src/plan/LayoutGraph.cpp
    src/ir/IrBuilder.cpp src/semantic/SemanticAnalyzer.cpp src/parser/ParserDriver.cpp
    ${ANTLR4_GENERATED_FILES})
add_dependencies(embx_name_resolution_plan_test GenerateAntlr)
target_include_directories(embx_name_resolution_plan_test PRIVATE
    "${ANTLR4_OUTPUT_DIR}" "${ANTLR4_RUNTIME_DIR}/include/antlr4-runtime" src)
target_link_libraries(embx_name_resolution_plan_test PRIVATE Catch2::Catch2WithMain "${ANTLR4_LIB}")
target_compile_features(embx_name_resolution_plan_test PRIVATE cxx_std_17)
target_compile_options(embx_name_resolution_plan_test PRIVATE -Wall -Wextra -Wpedantic -Wno-overloaded-virtual)
add_test(NAME name_resolution_plan_test COMMAND embx_name_resolution_plan_test)

add_executable(embx_semantic_type_resolution_contract_test tests/semantic_type_resolution_contract_test.cpp
  src/semantic/SemanticAnalyzer.cpp src/parser/ParserDriver.cpp ${ANTLR4_GENERATED_FILES})
add_dependencies(embx_semantic_type_resolution_contract_test GenerateAntlr)
target_include_directories(embx_semantic_type_resolution_contract_test PRIVATE
  "${ANTLR4_OUTPUT_DIR}" "${ANTLR4_RUNTIME_DIR}/include/antlr4-runtime" src)
target_link_libraries(embx_semantic_type_resolution_contract_test PRIVATE Catch2::Catch2WithMain "${ANTLR4_LIB}")
target_compile_features(embx_semantic_type_resolution_contract_test PRIVATE cxx_std_17)
target_compile_options(embx_semantic_type_resolution_contract_test PRIVATE -Wall -Wextra -Wpedantic -Wno-overloaded-virtual)
add_test(NAME semantic_type_resolution_contract_test COMMAND embx_semantic_type_resolution_contract_test)

add_executable(embx_runtime_parameters_test tests/runtime_parameters_test.cpp src/reflection/Reflection.cpp src/plan/PlanBuilder.cpp src/plan/LayoutGraph.cpp src/runtime/ExpressionEvaluator.cpp src/runtime/Runtime.cpp src/ir/IrBuilder.cpp src/semantic/SemanticAnalyzer.cpp src/parser/ParserDriver.cpp src/encoder/Encoder.cpp src/decoder/Decoder.cpp src/value/Value.cpp src/diagnostics/Diagnostic.cpp ${ANTLR4_GENERATED_FILES})
add_dependencies(embx_runtime_parameters_test GenerateAntlr)
target_include_directories(embx_runtime_parameters_test PRIVATE "${ANTLR4_OUTPUT_DIR}" "${ANTLR4_RUNTIME_DIR}/include/antlr4-runtime" src)
target_link_libraries(embx_runtime_parameters_test PRIVATE Catch2::Catch2WithMain "${ANTLR4_LIB}")
target_compile_features(embx_runtime_parameters_test PRIVATE cxx_std_17)
target_compile_options(embx_runtime_parameters_test PRIVATE -Wall -Wextra -Wpedantic -Wno-overloaded-virtual)
add_test(NAME runtime_parameters_test COMMAND embx_runtime_parameters_test)

add_executable(embx_midi_example_test tests/midi_example_test.cpp
  src/compiler/Compiler.cpp src/parser/ParserDriver.cpp src/semantic/SemanticAnalyzer.cpp
  src/ir/IrBuilder.cpp src/plan/PlanBuilder.cpp src/plan/LayoutGraph.cpp
  src/runtime/Runtime.cpp src/runtime/ExpressionEvaluator.cpp
  src/decoder/Decoder.cpp src/encoder/Encoder.cpp src/value/Value.cpp
  src/diagnostics/Diagnostic.cpp src/reflection/Reflection.cpp
  src/codegen/CppGenerator.cpp ${ANTLR4_GENERATED_FILES})
add_dependencies(embx_midi_example_test GenerateAntlr)
target_include_directories(embx_midi_example_test PRIVATE
  "${ANTLR4_OUTPUT_DIR}" "${ANTLR4_RUNTIME_DIR}/include/antlr4-runtime" src)
target_link_libraries(embx_midi_example_test PRIVATE Catch2::Catch2WithMain "${ANTLR4_LIB}")
target_compile_definitions(embx_midi_example_test PRIVATE EMBX_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_features(embx_midi_example_test PRIVATE cxx_std_17)
target_compile_options(embx_midi_example_test PRIVATE -Wall -Wextra -Wpedantic -Wno-overloaded-virtual)
add_test(NAME midi_example_test COMMAND embx_midi_example_test)

add_executable(embx_virtual_field_test tests/virtual_field_test.cpp src/plan/PlanBuilder.cpp src/plan/LayoutGraph.cpp src/runtime/ExpressionEvaluator.cpp src/runtime/Runtime.cpp src/ir/IrBuilder.cpp src/semantic/SemanticAnalyzer.cpp src/parser/ParserDriver.cpp src/encoder/Encoder.cpp src/decoder/Decoder.cpp src/value/Value.cpp src/diagnostics/Diagnostic.cpp ${ANTLR4_GENERATED_FILES})
add_executable(embx_field_alias_test tests/field_alias_test.cpp src/plan/PlanBuilder.cpp src/plan/LayoutGraph.cpp src/runtime/ExpressionEvaluator.cpp src/runtime/Runtime.cpp src/ir/IrBuilder.cpp src/semantic/SemanticAnalyzer.cpp src/parser/ParserDriver.cpp src/encoder/Encoder.cpp src/decoder/Decoder.cpp src/value/Value.cpp src/diagnostics/Diagnostic.cpp src/reflection/Reflection.cpp ${ANTLR4_GENERATED_FILES})
add_dependencies(embx_field_alias_test GenerateAntlr)
target_include_directories(embx_field_alias_test PRIVATE "${ANTLR4_OUTPUT_DIR}" "${ANTLR4_RUNTIME_DIR}/include/antlr4-runtime" src)
target_link_libraries(embx_field_alias_test PRIVATE Catch2::Catch2WithMain "${ANTLR4_LIB}")
target_compile_features(embx_field_alias_test PRIVATE cxx_std_17)
target_compile_options(embx_field_alias_test PRIVATE -Wall -Wextra -Wpedantic -Wno-overloaded-virtual)
add_dependencies(embx_virtual_field_test GenerateAntlr)
target_include_directories(embx_virtual_field_test PRIVATE "${ANTLR4_OUTPUT_DIR}" "${ANTLR4_RUNTIME_DIR}/include/antlr4-runtime" src)
target_link_libraries(embx_virtual_field_test PRIVATE Catch2::Catch2WithMain "${ANTLR4_LIB}")
target_compile_features(embx_virtual_field_test PRIVATE cxx_std_17)
target_compile_options(embx_virtual_field_test PRIVATE -Wall -Wextra -Wpedantic -Wno-overloaded-virtual)
add_test(NAME virtual_field_test COMMAND embx_virtual_field_test)
add_test(NAME field_alias_test COMMAND embx_field_alias_test)

add_executable(embx_conditional_runtime_test tests/conditional_runtime_test.cpp src/plan/PlanBuilder.cpp src/plan/LayoutGraph.cpp src/runtime/ExpressionEvaluator.cpp src/runtime/Runtime.cpp src/ir/IrBuilder.cpp src/semantic/SemanticAnalyzer.cpp src/parser/ParserDriver.cpp src/encoder/Encoder.cpp src/decoder/Decoder.cpp src/value/Value.cpp src/diagnostics/Diagnostic.cpp ${ANTLR4_GENERATED_FILES})
add_dependencies(embx_conditional_runtime_test GenerateAntlr)
target_include_directories(embx_conditional_runtime_test PRIVATE "${ANTLR4_OUTPUT_DIR}" "${ANTLR4_RUNTIME_DIR}/include/antlr4-runtime" src)
target_link_libraries(embx_conditional_runtime_test PRIVATE Catch2::Catch2WithMain "${ANTLR4_LIB}")
target_compile_features(embx_conditional_runtime_test PRIVATE cxx_std_17)
target_compile_options(embx_conditional_runtime_test PRIVATE -Wall -Wextra -Wpedantic -Wno-overloaded-virtual)
add_test(NAME conditional_runtime_test COMMAND embx_conditional_runtime_test)

add_executable(embx_transform_test tests/transform_test.cpp src/plan/PlanBuilder.cpp src/plan/LayoutGraph.cpp src/runtime/ExpressionEvaluator.cpp src/runtime/Runtime.cpp src/ir/IrBuilder.cpp src/semantic/SemanticAnalyzer.cpp src/parser/ParserDriver.cpp src/encoder/Encoder.cpp src/decoder/Decoder.cpp src/value/Value.cpp src/diagnostics/Diagnostic.cpp src/reflection/Reflection.cpp ${ANTLR4_GENERATED_FILES})
add_dependencies(embx_transform_test GenerateAntlr)
target_include_directories(embx_transform_test PRIVATE "${ANTLR4_OUTPUT_DIR}" "${ANTLR4_RUNTIME_DIR}/include/antlr4-runtime" src)
target_link_libraries(embx_transform_test PRIVATE Catch2::Catch2WithMain "${ANTLR4_LIB}")
target_compile_features(embx_transform_test PRIVATE cxx_std_17)
target_compile_options(embx_transform_test PRIVATE -Wall -Wextra -Wpedantic -Wno-overloaded-virtual)
add_test(NAME transform_test COMMAND embx_transform_test)

add_executable(embx_byte_order_closure_test tests/byte_order_closure_test.cpp
    src/plan/PlanBuilder.cpp src/plan/LayoutGraph.cpp src/runtime/ExpressionEvaluator.cpp
    src/runtime/Runtime.cpp src/ir/IrBuilder.cpp src/semantic/SemanticAnalyzer.cpp
    src/parser/ParserDriver.cpp src/reflection/Reflection.cpp src/codegen/CppGenerator.cpp
    src/value/Value.cpp src/diagnostics/Diagnostic.cpp ${ANTLR4_GENERATED_FILES})
add_dependencies(embx_byte_order_closure_test GenerateAntlr)
target_include_directories(embx_byte_order_closure_test PRIVATE "${ANTLR4_OUTPUT_DIR}" "${ANTLR4_RUNTIME_DIR}/include/antlr4-runtime" src)
target_link_libraries(embx_byte_order_closure_test PRIVATE Catch2::Catch2WithMain "${ANTLR4_LIB}")
target_compile_features(embx_byte_order_closure_test PRIVATE cxx_std_17)
target_compile_options(embx_byte_order_closure_test PRIVATE -Wall -Wextra -Wpedantic -Wno-overloaded-virtual)
add_test(NAME byte_order_closure_test COMMAND embx_byte_order_closure_test)

add_executable(embx_requires_test tests/requires_test.cpp
    src/plan/PlanBuilder.cpp src/plan/LayoutGraph.cpp src/runtime/ExpressionEvaluator.cpp
    src/runtime/Runtime.cpp src/ir/IrBuilder.cpp src/semantic/SemanticAnalyzer.cpp
    src/parser/ParserDriver.cpp src/reflection/Reflection.cpp src/codegen/CppGenerator.cpp src/encoder/Encoder.cpp src/decoder/Decoder.cpp
    src/value/Value.cpp src/diagnostics/Diagnostic.cpp ${ANTLR4_GENERATED_FILES})
add_dependencies(embx_requires_test GenerateAntlr)
target_include_directories(embx_requires_test PRIVATE "${ANTLR4_OUTPUT_DIR}" "${ANTLR4_RUNTIME_DIR}/include/antlr4-runtime" src)
target_link_libraries(embx_requires_test PRIVATE Catch2::Catch2WithMain "${ANTLR4_LIB}")
target_compile_features(embx_requires_test PRIVATE cxx_std_17)
target_compile_options(embx_requires_test PRIVATE -Wall -Wextra -Wpedantic -Wno-overloaded-virtual)
add_test(NAME requires_test COMMAND embx_requires_test)

add_executable(embx_variant_closure_test tests/variant_closure_test.cpp
    src/plan/PlanBuilder.cpp src/plan/LayoutGraph.cpp src/runtime/ExpressionEvaluator.cpp
    src/runtime/Runtime.cpp src/ir/IrBuilder.cpp src/semantic/SemanticAnalyzer.cpp
    src/parser/ParserDriver.cpp src/reflection/Reflection.cpp src/codegen/CppGenerator.cpp
    src/encoder/Encoder.cpp src/decoder/Decoder.cpp src/value/Value.cpp src/diagnostics/Diagnostic.cpp ${ANTLR4_GENERATED_FILES})
add_dependencies(embx_variant_closure_test GenerateAntlr)
target_include_directories(embx_variant_closure_test PRIVATE "${ANTLR4_OUTPUT_DIR}" "${ANTLR4_RUNTIME_DIR}/include/antlr4-runtime" src)
target_link_libraries(embx_variant_closure_test PRIVATE Catch2::Catch2WithMain "${ANTLR4_LIB}")
target_compile_features(embx_variant_closure_test PRIVATE cxx_std_17)
target_compile_options(embx_variant_closure_test PRIVATE -Wall -Wextra -Wpedantic -Wno-overloaded-virtual)
add_test(NAME variant_closure_test COMMAND embx_variant_closure_test)

add_executable(embx_lesson_01_first_format_test
  tests/lesson_01_first_format_test.cpp
  src/compiler/Compiler.cpp
  src/parser/ParserDriver.cpp
  src/semantic/SemanticAnalyzer.cpp
  src/ir/IrBuilder.cpp
  src/plan/PlanBuilder.cpp
  src/plan/LayoutGraph.cpp
  src/runtime/ExpressionEvaluator.cpp
  src/runtime/Runtime.cpp
  src/codegen/CppGenerator.cpp
  src/encoder/Encoder.cpp
  src/decoder/Decoder.cpp
  src/value/Value.cpp
  src/diagnostics/Diagnostic.cpp
  ${ANTLR4_GENERATED_FILES})
add_dependencies(embx_lesson_01_first_format_test GenerateAntlr)
target_include_directories(embx_lesson_01_first_format_test PRIVATE
  "${ANTLR4_OUTPUT_DIR}" "${ANTLR4_RUNTIME_DIR}/include/antlr4-runtime" src)
target_link_libraries(embx_lesson_01_first_format_test PRIVATE Catch2::Catch2WithMain "${ANTLR4_LIB}")
target_compile_options(embx_lesson_01_first_format_test PRIVATE -Wall -Wextra -Wpedantic -Wno-overloaded-virtual)
add_test(NAME lesson_01_first_format COMMAND embx_lesson_01_first_format_test)

add_executable(embx_lesson_02_integers_byte_order_test
  tests/lesson_02_integers_byte_order_test.cpp
  src/compiler/Compiler.cpp
  src/parser/ParserDriver.cpp
  src/semantic/SemanticAnalyzer.cpp
  src/ir/IrBuilder.cpp
  src/plan/PlanBuilder.cpp
  src/plan/LayoutGraph.cpp
  src/runtime/ExpressionEvaluator.cpp
  src/runtime/Runtime.cpp
  src/codegen/CppGenerator.cpp
  src/encoder/Encoder.cpp
  src/decoder/Decoder.cpp
  src/value/Value.cpp
  src/diagnostics/Diagnostic.cpp
  ${ANTLR4_GENERATED_FILES})
add_dependencies(embx_lesson_02_integers_byte_order_test GenerateAntlr)
target_include_directories(embx_lesson_02_integers_byte_order_test PRIVATE
  "${ANTLR4_OUTPUT_DIR}" "${ANTLR4_RUNTIME_DIR}/include/antlr4-runtime" src)
target_link_libraries(embx_lesson_02_integers_byte_order_test PRIVATE Catch2::Catch2WithMain "${ANTLR4_LIB}")
target_compile_options(embx_lesson_02_integers_byte_order_test PRIVATE -Wall -Wextra -Wpedantic -Wno-overloaded-virtual)
add_test(NAME lesson_02_integers_byte_order COMMAND embx_lesson_02_integers_byte_order_test)
add_executable(embx_lesson_03_bit_fields_test
  tests/lesson_03_bit_fields_test.cpp
  src/compiler/Compiler.cpp
  src/parser/ParserDriver.cpp
  src/semantic/SemanticAnalyzer.cpp
  src/ir/IrBuilder.cpp
  src/plan/PlanBuilder.cpp
  src/plan/LayoutGraph.cpp
  src/runtime/ExpressionEvaluator.cpp
  src/runtime/Runtime.cpp
  src/codegen/CppGenerator.cpp
  src/encoder/Encoder.cpp
  src/decoder/Decoder.cpp
  src/value/Value.cpp
  src/diagnostics/Diagnostic.cpp
  ${ANTLR4_GENERATED_FILES})
add_dependencies(embx_lesson_03_bit_fields_test GenerateAntlr)
target_include_directories(embx_lesson_03_bit_fields_test PRIVATE
  "${ANTLR4_OUTPUT_DIR}" "${ANTLR4_RUNTIME_DIR}/include/antlr4-runtime" src)
target_link_libraries(embx_lesson_03_bit_fields_test PRIVATE Catch2::Catch2WithMain "${ANTLR4_LIB}")
target_compile_features(embx_lesson_03_bit_fields_test PRIVATE cxx_std_17)
target_compile_options(embx_lesson_03_bit_fields_test PRIVATE -Wall -Wextra -Wpedantic -Wno-overloaded-virtual)
add_test(NAME lesson_03_bit_fields COMMAND embx_lesson_03_bit_fields_test)
add_executable(embx_lesson_04_arrays_dynamic_dimensions_test
  tests/lesson_04_arrays_dynamic_dimensions_test.cpp
  src/compiler/Compiler.cpp
  src/parser/ParserDriver.cpp
  src/semantic/SemanticAnalyzer.cpp
  src/ir/IrBuilder.cpp
  src/plan/PlanBuilder.cpp
  src/plan/LayoutGraph.cpp
  src/runtime/ExpressionEvaluator.cpp
  src/runtime/Runtime.cpp
  src/codegen/CppGenerator.cpp
  src/encoder/Encoder.cpp
  src/decoder/Decoder.cpp
  src/value/Value.cpp
  src/diagnostics/Diagnostic.cpp
  ${ANTLR4_GENERATED_FILES})
add_dependencies(embx_lesson_04_arrays_dynamic_dimensions_test GenerateAntlr)
target_include_directories(embx_lesson_04_arrays_dynamic_dimensions_test PRIVATE
  "${ANTLR4_OUTPUT_DIR}" "${ANTLR4_RUNTIME_DIR}/include/antlr4-runtime" src)
target_link_libraries(embx_lesson_04_arrays_dynamic_dimensions_test PRIVATE Catch2::Catch2WithMain "${ANTLR4_LIB}")
target_compile_features(embx_lesson_04_arrays_dynamic_dimensions_test PRIVATE cxx_std_17)
target_compile_options(embx_lesson_04_arrays_dynamic_dimensions_test PRIVATE -Wall -Wextra -Wpedantic -Wno-overloaded-virtual)
add_test(NAME lesson_04_arrays_dynamic_dimensions COMMAND embx_lesson_04_arrays_dynamic_dimensions_test)

add_executable(embx_lesson_05_nested_structures_test
  tests/lesson_05_nested_structures_test.cpp
  src/compiler/Compiler.cpp
  src/parser/ParserDriver.cpp
  src/semantic/SemanticAnalyzer.cpp
  src/ir/IrBuilder.cpp
  src/plan/PlanBuilder.cpp
  src/plan/LayoutGraph.cpp
  src/runtime/ExpressionEvaluator.cpp
  src/runtime/Runtime.cpp
  src/codegen/CppGenerator.cpp
  src/encoder/Encoder.cpp
  src/decoder/Decoder.cpp
  src/value/Value.cpp
  src/diagnostics/Diagnostic.cpp
  ${ANTLR4_GENERATED_FILES})
add_dependencies(embx_lesson_05_nested_structures_test GenerateAntlr)
target_include_directories(embx_lesson_05_nested_structures_test PRIVATE
  "${ANTLR4_OUTPUT_DIR}" "${ANTLR4_RUNTIME_DIR}/include/antlr4-runtime" src)
target_link_libraries(embx_lesson_05_nested_structures_test PRIVATE Catch2::Catch2WithMain "${ANTLR4_LIB}")
target_compile_features(embx_lesson_05_nested_structures_test PRIVATE cxx_std_17)
target_compile_options(embx_lesson_05_nested_structures_test PRIVATE -Wall -Wextra -Wpedantic -Wno-overloaded-virtual)
add_test(NAME lesson_05_nested_structures COMMAND embx_lesson_05_nested_structures_test)
add_executable(embx_lesson_06_variants_conditionals_test
  tests/lesson_06_variants_conditionals_test.cpp
  ${EMBX_CORE_SOURCES}
  ${ANTLR4_GENERATED_FILES})
add_dependencies(embx_lesson_06_variants_conditionals_test GenerateAntlr)
target_include_directories(embx_lesson_06_variants_conditionals_test PRIVATE
  "${ANTLR4_OUTPUT_DIR}" "${ANTLR4_RUNTIME_DIR}/include/antlr4-runtime" src)
target_link_libraries(embx_lesson_06_variants_conditionals_test PRIVATE Catch2::Catch2WithMain "${ANTLR4_LIB}")
target_compile_features(embx_lesson_06_variants_conditionals_test PRIVATE cxx_std_17)
target_compile_options(embx_lesson_06_variants_conditionals_test PRIVATE -Wall -Wextra -Wpedantic -Wno-overloaded-virtual)
add_test(NAME lesson_06_variants_conditionals COMMAND embx_lesson_06_variants_conditionals_test)

add_executable(embx_lesson_07_offsets_alignment_test
  tests/lesson_07_offsets_alignment_test.cpp
  ${EMBX_CORE_SOURCES}
  ${ANTLR4_GENERATED_FILES})
add_dependencies(embx_lesson_07_offsets_alignment_test GenerateAntlr)
target_include_directories(embx_lesson_07_offsets_alignment_test PRIVATE
  "${ANTLR4_OUTPUT_DIR}" "${ANTLR4_RUNTIME_DIR}/include/antlr4-runtime" src)
target_link_libraries(embx_lesson_07_offsets_alignment_test PRIVATE Catch2::Catch2WithMain "${ANTLR4_LIB}")
target_compile_features(embx_lesson_07_offsets_alignment_test PRIVATE cxx_std_17)
target_compile_options(embx_lesson_07_offsets_alignment_test PRIVATE -Wall -Wextra -Wpedantic -Wno-overloaded-virtual)
add_test(NAME lesson_07_offsets_alignment COMMAND embx_lesson_07_offsets_alignment_test)

add_executable(embx_lesson_08_symbol_dependencies_runtime_parameters_test
  tests/lesson_08_symbol_dependencies_runtime_parameters_test.cpp
  ${EMBX_CORE_SOURCES}
  src/runtime/Runtime.cpp
  src/codegen/CppGenerator.cpp
  src/encoder/Encoder.cpp
  src/decoder/Decoder.cpp
  src/value/Value.cpp
  src/diagnostics/Diagnostic.cpp
  ${ANTLR4_GENERATED_FILES})
add_dependencies(embx_lesson_08_symbol_dependencies_runtime_parameters_test GenerateAntlr)
target_include_directories(embx_lesson_08_symbol_dependencies_runtime_parameters_test PRIVATE
  "${ANTLR4_OUTPUT_DIR}" "${ANTLR4_RUNTIME_DIR}/include/antlr4-runtime" src)
target_link_libraries(embx_lesson_08_symbol_dependencies_runtime_parameters_test PRIVATE Catch2::Catch2WithMain "${ANTLR4_LIB}")
target_compile_features(embx_lesson_08_symbol_dependencies_runtime_parameters_test PRIVATE cxx_std_17)
target_compile_options(embx_lesson_08_symbol_dependencies_runtime_parameters_test PRIVATE -Wall -Wextra -Wpedantic -Wno-overloaded-virtual)
add_test(NAME lesson_08_symbol_dependencies_runtime_parameters COMMAND embx_lesson_08_symbol_dependencies_runtime_parameters_test)

add_executable(embx_lesson_09_virtual_fields_aliases_test
  tests/lesson_09_virtual_fields_aliases_test.cpp
  ${EMBX_CORE_SOURCES}
  ${ANTLR4_GENERATED_FILES})
add_dependencies(embx_lesson_09_virtual_fields_aliases_test GenerateAntlr)
target_include_directories(embx_lesson_09_virtual_fields_aliases_test PRIVATE
  "${ANTLR4_OUTPUT_DIR}" "${ANTLR4_RUNTIME_DIR}/include/antlr4-runtime" src)
target_link_libraries(embx_lesson_09_virtual_fields_aliases_test PRIVATE Catch2::Catch2WithMain "${ANTLR4_LIB}")
target_compile_features(embx_lesson_09_virtual_fields_aliases_test PRIVATE cxx_std_17)
target_compile_options(embx_lesson_09_virtual_fields_aliases_test PRIVATE -Wall -Wextra -Wpedantic -Wno-overloaded-virtual)
add_test(NAME lesson_09_virtual_fields_aliases COMMAND embx_lesson_09_virtual_fields_aliases_test)

add_executable(embx_lesson_10_callbacks_transforms_test
  tests/lesson_10_callbacks_transforms_test.cpp
  ${EMBX_CORE_SOURCES}
  ${ANTLR4_GENERATED_FILES})
add_dependencies(embx_lesson_10_callbacks_transforms_test GenerateAntlr)
target_include_directories(embx_lesson_10_callbacks_transforms_test PRIVATE
  "${ANTLR4_OUTPUT_DIR}" "${ANTLR4_RUNTIME_DIR}/include/antlr4-runtime" src)
target_link_libraries(embx_lesson_10_callbacks_transforms_test PRIVATE Catch2::Catch2WithMain "${ANTLR4_LIB}")
target_compile_features(embx_lesson_10_callbacks_transforms_test PRIVATE cxx_std_17)
target_compile_options(embx_lesson_10_callbacks_transforms_test PRIVATE -Wall -Wextra -Wpedantic -Wno-overloaded-virtual)
add_test(NAME lesson_10_callbacks_transforms COMMAND embx_lesson_10_callbacks_transforms_test)

add_executable(embx_lesson_11_ipv4_test
  tests/lesson_11_ipv4_test.cpp
  ${EMBX_CORE_SOURCES}
  ${ANTLR4_GENERATED_FILES})
add_dependencies(embx_lesson_11_ipv4_test GenerateAntlr)
target_include_directories(embx_lesson_11_ipv4_test PRIVATE
  "${ANTLR4_OUTPUT_DIR}" "${ANTLR4_RUNTIME_DIR}/include/antlr4-runtime" src)
target_link_libraries(embx_lesson_11_ipv4_test PRIVATE Catch2::Catch2WithMain "${ANTLR4_LIB}")
target_compile_features(embx_lesson_11_ipv4_test PRIVATE cxx_std_17)
target_compile_options(embx_lesson_11_ipv4_test PRIVATE -Wall -Wextra -Wpedantic -Wno-overloaded-virtual)
add_test(NAME lesson_11_ipv4 COMMAND embx_lesson_11_ipv4_test)

add_executable(embx_lesson_12_terminated_sequences_test
  tests/lesson_12_terminated_sequences_test.cpp
  ${EMBX_CORE_SOURCES}
  ${ANTLR4_GENERATED_FILES})
add_dependencies(embx_lesson_12_terminated_sequences_test GenerateAntlr)
target_include_directories(embx_lesson_12_terminated_sequences_test PRIVATE
  "${ANTLR4_OUTPUT_DIR}" "${ANTLR4_RUNTIME_DIR}/include/antlr4-runtime" src)
target_link_libraries(embx_lesson_12_terminated_sequences_test PRIVATE Catch2::Catch2WithMain "${ANTLR4_LIB}")
target_compile_features(embx_lesson_12_terminated_sequences_test PRIVATE cxx_std_17)
target_compile_options(embx_lesson_12_terminated_sequences_test PRIVATE -Wall -Wextra -Wpedantic -Wno-overloaded-virtual)
add_test(NAME lesson_12_terminated_sequences COMMAND embx_lesson_12_terminated_sequences_test)

add_executable(embx_lesson_14_tlv_test
  tests/lesson_14_tlv_test.cpp
  ${EMBX_CORE_SOURCES}
  ${ANTLR4_GENERATED_FILES})
add_dependencies(embx_lesson_14_tlv_test GenerateAntlr)
target_include_directories(embx_lesson_14_tlv_test PRIVATE
  "${ANTLR4_OUTPUT_DIR}" "${ANTLR4_RUNTIME_DIR}/include/antlr4-runtime" src)
target_link_libraries(embx_lesson_14_tlv_test PRIVATE Catch2::Catch2WithMain "${ANTLR4_LIB}")
target_compile_features(embx_lesson_14_tlv_test PRIVATE cxx_std_17)
target_compile_options(embx_lesson_14_tlv_test PRIVATE -Wall -Wextra -Wpedantic -Wno-overloaded-virtual)
add_test(NAME lesson_14_tlv COMMAND embx_lesson_14_tlv_test)

add_executable(embx_terminated_sequence_test
  tests/terminated_sequence_test.cpp
  ${EMBX_CORE_SOURCES}
  ${ANTLR4_GENERATED_FILES})
add_dependencies(embx_terminated_sequence_test GenerateAntlr)
target_include_directories(embx_terminated_sequence_test PRIVATE
  "${ANTLR4_OUTPUT_DIR}" "${ANTLR4_RUNTIME_DIR}/include/antlr4-runtime" src)
target_link_libraries(embx_terminated_sequence_test PRIVATE Catch2::Catch2WithMain "${ANTLR4_LIB}")
target_compile_features(embx_terminated_sequence_test PRIVATE cxx_std_17)
target_compile_options(embx_terminated_sequence_test PRIVATE -Wall -Wextra -Wpedantic -Wno-overloaded-virtual)
add_test(NAME terminated_sequence_test COMMAND embx_terminated_sequence_test)
