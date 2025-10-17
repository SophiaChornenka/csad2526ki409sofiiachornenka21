cmake_minimum_required(VERSION 3.16)

# Replace "MyProject" with your project's real name (or call cmake -DPROJECT_NAME=...).
project(MyProject VERSION 0.1.0 LANGUAGES CXX)

# -------------------------
# Configuration options
# -------------------------
option(BUILD_SHARED_LIBS "Build libraries as shared instead of static" OFF)
option(BUILD_TESTING "Build unit tests" ON)
option(BUILD_EXAMPLES "Build example programs" ON)

set(CMAKE_CXX_STANDARD 17 CACHE STRING "C++ standard to use (11, 14, 17, 20, ...)")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Make builds reproducible / nice defaults
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
endif()

# -------------------------
# Basic project layout
# -------------------------
# Source layout expectations (adjust as needed):
#  - src/       : project sources and public headers
#  - include/   : optional public headers (installed to <prefix>/include)
#  - tests/     : unit tests
#  - examples/  : example executables
#
# You can override SRC_DIR/INCLUDE_DIR from the command line:
#  cmake -DSRC_DIR=/path/to/src -DINCLUDE_DIR=/path/to/include ..

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(EXAMPLES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/examples")
set(TESTS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/tests")

# Gather sources (CONFIGURE_DEPENDS keeps it updated when files change)
file(GLOB_RECURSE PROJECT_SOURCES
     CONFIGURE_DEPENDS
     ${SRC_DIR}/*.cpp
     ${SRC_DIR}/*.cc
     ${SRC_DIR}/*.cxx
     ${SRC_DIR}/*.c
)

# Also pick up headers for IDEs (optional)
file(GLOB_RECURSE PROJECT_HEADERS
     CONFIGURE_DEPENDS
     ${SRC_DIR}/*.h
     ${SRC_DIR}/*.hpp
     ${INCLUDE_DIR}/*.h
     ${INCLUDE_DIR}/*.hpp
)

if(PROJECT_SOURCES)
  message(STATUS "Found ${PROJECT_SOURCES}")
else()
  message(WARNING "No source files found in ${SRC_DIR}. Edit CMakeLists.txt or add sources.")
endif()

# If a main file exists, make an executable; otherwise make a library
set(MAIN_FOUND FALSE)
foreach(ext IN LISTS CXX_SOURCE_FILE_EXTENSIONS)
  if(EXISTS "${SRC_DIR}/main.${ext}")
    set(MAIN_FOUND TRUE)
    break()
  endif()
endforeach()

if(MAIN_FOUND)
  add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} ${PROJECT_HEADERS})
else()
  add_library(${PROJECT_NAME} ${PROJECT_SOURCES} ${PROJECT_HEADERS})
endif()

# Public include paths and usage requirements
target_include_directories(${PROJECT_NAME}
  PUBLIC
    $<BUILD_INTERFACE:${INCLUDE_DIR}>
    $<BUILD_INTERFACE:${SRC_DIR}>
    $<INSTALL_INTERFACE:include>
)

# Set compile features from CMake, prefer target properties over global flags
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_${CMAKE_CXX_STANDARD})

# Example: add common compile options (customize/guard per-compiler)
if(MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE /W4 /permissive-)
else()
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
  # enable link-time optimization in Release if desired:
  include(CheckIPOSupported)
  check_ipo_supported(RESULT ipo_supported OUTPUT ipo_error)
  if(ipo_supported)
    set_target_properties(${PROJECT_NAME} PROPERTIES INTERPROCEDURAL_OPTIMIZATION $<BOOL:$<CONFIG:Release>>)
  endif()
endif()

# -------------------------
# Installation
# -------------------------
include(GNUInstallDirs)

# Install target and export
install(TARGETS ${PROJECT_NAME}
  EXPORT ${PROJECT_NAME}Targets
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Install headers from include/ (if present)
if(EXISTS ${INCLUDE_DIR})
  install(DIRECTORY ${INCLUDE_DIR}/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
endif()

# Export the targets for find_package
install(EXPORT ${PROJECT_NAME}Targets
  FILE ${PROJECT_NAME}Targets.cmake
  NAMESPACE ${PROJECT_NAME}::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

# Generate a basic <project>Config.cmake file for consumers
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
  "${CMAKE_CURRENT_LIST_DIR}/cmake/Config.cmake.in" # optional, see note below
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
  NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

# NOTE:
# If you don't have a cmake/Config.cmake.in file, you can either create a minimal one
# or remove the configure_package_config_file call and consumer-style installation.

# -------------------------
# Tests (GoogleTest via FetchContent)
# -------------------------
if(BUILD_TESTING)
  enable_testing()
  include(FetchContent)

  # Fetch GoogleTest (version pin can be changed)
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip
  )

  # Prefer shared CRT on MSVC to match project CRT settings (only set on MSVC)
  if(MSVC)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  endif()

  FetchContent_MakeAvailable(googletest)

  # Gather test sources (test_*.cpp and *_test.cpp)
  file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS
    ${TESTS_DIR}/test_*.cpp
    ${TESTS_DIR}/*_test.cpp
  )

  if(TEST_SOURCES)
    add_executable(${PROJECT_NAME}_tests ${TEST_SOURCES})
    # Link against the modern imported target provided by GoogleTest
    target_link_libraries(${PROJECT_NAME}_tests PRIVATE GTest::gtest_main ${PROJECT_NAME})
    target_include_directories(${PROJECT_NAME}_tests PRIVATE ${SRC_DIR} ${INCLUDE_DIR})

    # Make sure tests are built into a predictable output directory
    set_target_properties(${PROJECT_NAME}_tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests)

    # Use CTest + GoogleTest integration so individual tests are discovered
    include(GoogleTest)
    gtest_discover_tests(${PROJECT_NAME}_tests
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
      PROPERTIES TIMEOUT 10
    )

    # Convenience target to run all tests with a single target
    add_custom_target(check
      COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
      DEPENDS ${PROJECT_NAME}_tests
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
  else()
    message(STATUS "No test sources found in ${TESTS_DIR}. Skipping test target generation.")
  endif()
endif()

# -------------------------
# Examples
# -------------------------
if(BUILD_EXAMPLES AND EXISTS ${EXAMPLES_DIR})
  file(GLOB_RECURSE EXAMPLE_SOURCES CONFIGURE_DEPENDS ${EXAMPLES_DIR}/*.cpp ${EXAMPLES_DIR}/*.cxx)
  foreach(example_src IN LISTS EXAMPLE_SOURCES)
    get_filename_component(example_name ${example_src} NAME_WE)
    add_executable(${example_name} ${example_src})
    target_link_libraries(${example_name} PRIVATE ${PROJECT_NAME})
    target_include_directories(${example_name} PRIVATE ${INCLUDE_DIR} ${SRC_DIR})
    set_target_properties(${example_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/examples)
  endforeach()
endif()

# -------------------------
# Packaging helpers
# -------------------------
include(CPack)
# You can configure CPack variables here (CPACK_*), e.g. CPACK_PACKAGE_NAME, CPACK_PACKAGE_VERSION, etc.

# -------------------------
# Helpful summary
# -------------------------
message(STATUS "Project: ${PROJECT_NAME} ${PROJECT_VERSION}")
message(STATUS "C++ standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Build tests: ${BUILD_TESTING}")
message(STATUS "Build examples: ${BUILD_EXAMPLES}")
