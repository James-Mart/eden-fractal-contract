# All cmake projects need this
cmake_minimum_required(VERSION 3.16)

###################################################################
# Main variable configurations - Customized with each project
## Basic configuration -- Only set the varables in this section
set(PROJ eden_fractal)
list(APPEND FILENAMES "agreement-contract")
###################################################################



######################################################
######################################################
#        SHOULD NOT NEED TO MODIFY BELOW HERE        #
######################################################
######################################################



###################################################################
# Configure contract directory structure 
set(SOURCEDIR "src/" )
set(TESTDIR "test/" )
set(RICARDIANDIR "ricardian/")
list(APPEND INCLUDE_DIRS "include/" "schema/")
###################################################################

# Needed in every cmake project
project(${PROJ})

## Advanced configuration -- Only change these if you know what you're doing
### Generate test- file variable (Only one per project)
string(REPLACE "${PROJ}" "${TESTDIR}test-${PROJ}.cpp" TESTNAME ${PROJ})

### Generate -ricardian files variable (One per entry in FILENAMES)
foreach (var ${FILENAMES})
  string(REPLACE "${var}" "${RICARDIANDIR}${var}-ricardian.cpp" temp ${var})
  list(APPEND RICARDIANLIST ${temp})
endforeach()

### Generate source files variable (One per entry in FILENAMES)
foreach (var ${FILENAMES})
  string(REPLACE "${var}" "${SOURCEDIR}${var}.cpp" temp ${var})
  list(APPEND SOURCELIST ${temp})
endforeach()

# Debug outputs
# message("Test file: ${TESTNAME}\n")
# message("Ricardian files: ${RICARDIANLIST}\n")
# message("Source files: ${SOURCELIST}\n")

# clsdk requires C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Libraries for building contracts and tests
find_package(clsdk REQUIRED)

# Set output directory
set(ARTIFACTS_DIR "artifacts")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${ARTIFACTS_DIR})

# Builds main ${PROJ}.wasm contract
# Contracts may link to either:
#   * eosio-contract-simple-malloc: This library builds contracts with
#     small and fast memory allocation. free() is a no-op. Most contracts
#     should use this option.
#   * eosio-contract-full-malloc: This library builds contracts with
#     full memory allocation and reuse. Using this in your contract may
#     help expose memory bugs that eosio-contract-simple-malloc hides.
#     The downsides of eosio-contract-full-malloc are that contracts
#     will be larger and slower.
add_executable(${PROJ} ${SOURCELIST} ${RICARDIANLIST})
target_include_directories(${PROJ} PRIVATE ${INCLUDE_DIRS})
target_link_libraries(${PROJ} eosio-contract-simple-malloc)


# Builds ${PROJ}-debug.wasm
#
# This is like ${PROJ}.wasm, but includes debugging information.
# Debug contracts can't normally be installed on chains using `set code`.
# Instead, cltester loads them using its `-s/--subst` option.
#
# Create a debugging contract by linking to either
# eosio-contract-simple-malloc-debug or eosio-contract-full-malloc-debug.
set(DEBUG_PROJ ${PROJ}-debug)
add_executable(${DEBUG_PROJ} ${SOURCELIST} ${RICARDIANLIST})
target_include_directories(${DEBUG_PROJ} PRIVATE ${INCLUDE_DIRS})
target_link_libraries(${DEBUG_PROJ} eosio-contract-simple-malloc-debug)

# Generate ${PROJ}.abi
# This is a 2-step process:
#   * Build ${PROJ}.abi.wasm. This must link to eosio-contract-abigen.
#   * Run the wasm to generate the abi
set(ABIGEN_PROJ ${PROJ}-abigen)
add_executable(${ABIGEN_PROJ} ${SOURCELIST} ${RICARDIANLIST})
target_include_directories(${ABIGEN_PROJ} PRIVATE ${INCLUDE_DIRS})
target_link_libraries(${ABIGEN_PROJ} eosio-contract-abigen)
add_custom_command(TARGET ${ABIGEN_PROJ} POST_BUILD
     COMMAND cltester ${ARTIFACTS_DIR}/${ABIGEN_PROJ}.wasm > ${ARTIFACTS_DIR}/${PROJ}.abi )

# Builds test-${PROJ}.wasm
# Tests must link to either cltestlib (runs faster) or cltestlib-debug
# (shows stack traces on failure).
set(TEST_PROJ test-${PROJ})
add_executable(${TEST_PROJ} ${TESTNAME} ${RICARDIANLIST})
target_include_directories(${TEST_PROJ} PRIVATE ${INCLUDE_DIRS})
target_link_libraries(${TEST_PROJ} cltestlib-debug)

# ctest rule which runs test-${PROJ}.wasm. The -v and -s
# options provide detailed logging. ctest hides this detail;
# use `ctest -V` so show it.
enable_testing()
add_test(
    NAME ${PROJ}_TEST
    COMMAND cltester -v ${ARTIFACTS_DIR}/${TEST_PROJ}.wasm -s
)

# These symlinks help keep absolute paths outside of the files in .vscode/
execute_process(COMMAND ln -sf ${clsdk_DIR} ${CMAKE_CURRENT_BINARY_DIR}/clsdk)
execute_process(COMMAND ln -sf ${WASI_SDK_PREFIX} ${CMAKE_CURRENT_BINARY_DIR}/wasi-sdk)

# Generate compile_commands.json during build to enable
# symbol lookup in some editors
set(CMAKE_EXPORT_COMPILE_COMMANDS on)
