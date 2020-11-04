# ======================================================================
#  hep_concurrency main build file
#
#  cd .../path/to/build/directory
#  source .../path/to/hep_concurrency/ups/setup_for_development
#  cmake [-DCMAKE_INSTALL_PREFIX=/install/path]
#        -DCMAKE_BUILD_TYPE=$CETPKG_TYPE
#        $CETPKG_SOURCE
#  make
#  make test
#  make install
#  make package (builds distribution tarfile)
# ======================================================================
cmake_minimum_required(VERSION 3.18.2 FATAL_ERROR)

cmake_policy(VERSION 3.14)
# Policy CMP0096 must be set to NEW to preserve leading zeros in the
# VERSION set by project().
cmake_policy(SET CMP0096 NEW)
find_package(cetmodules 2.02.00 REQUIRED)
project(hep_concurrency VERSION 1.05.00 LANGUAGES CXX)

# cetbuildtools contains our cmake modules ### MIGRATE-ACTION-REQUIRED: remove

include(CetCMakeEnv)
cet_cmake_env()

cet_set_compiler_flags(DIAGS VIGILANT
  WERROR
  NO_UNDEFINED
  EXTRA_FLAGS -pedantic
  EXTRA_CXX_FLAGS -Wnon-virtual-dtor -Wdelete-non-virtual-dtor
  )

if($ENV{CET_SANITIZE_ADDRESS})
  #message(STATUS "CET_SANITIZE_ADDRESS=$ENV{CET_SANITIZE_ADDRESS}")
  if(NOT "$ENV{GCC_FQ_DIR}" STREQUAL "")
    #message(STATUS "GCC_FQ_DIR=$ENV{GCC_FQ_DIR}")
    find_file(ASAN_LIB libasan.so NO_DEFAULT_PATH PATHS $ENV{GCC_FQ_DIR}/lib64)
    if(NOT ASAN_LIB STREQUAL "ASAN_LIB-NOTFOUND")
      #message(STATUS "ASAN_LIB=${ASAN_LIB}")
    endif()
  endif()
endif()

if($ENV{CET_SANITIZE_THREAD})
  #message(STATUS "CET_SANITIZE_THREAD=$ENV{CET_SANITIZE_THREAD}")
  if(NOT "$ENV{GCC_FQ_DIR}" STREQUAL "")
    #message(STATUS "GCC_FQ_DIR=$ENV{GCC_FQ_DIR}")
    find_file(TSAN_LIB libtsan.so NO_DEFAULT_PATH PATHS $ENV{GCC_FQ_DIR}/lib64)
    if(NOT TSAN_LIB STREQUAL "TSAN_LIB-NOTFOUND")
      #message(STATUS "TSAN_LIB=${TSAN_LIB}")
    endif()
  endif()
endif()

if ($ENV{CET_SANITIZE_ADDRESS})
cet_add_compiler_flags(CXX -fsanitize=address)
cet_add_compiler_flags(C -fsanitize=address)
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -fsanitize=address")
set(CMAKE_MODULE_LINKER_FLAGS_DEBUG "${CMAKE_MODULE_LINKER_FLAGS_DEBUG} -fsanitize=address")
set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} -fsanitize=address")
endif()

if ($ENV{CET_SANITIZE_THREAD})
cet_add_compiler_flags(CXX -fsanitize=thread -D__SANITIZE_THREAD__)
cet_add_compiler_flags(C -fsanitize=thread -D__SANITIZE_THREAD__)
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -fsanitize=thread -D__SANITIZE_THREAD__")
set(CMAKE_MODULE_LINKER_FLAGS_DEBUG "${CMAKE_MODULE_LINKER_FLAGS_DEBUG} -fsanitize=thread -D__SANITIZE_THREAD__")
set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} -fsanitize=thread -D__SANITIZE_THREAD__")
endif()

cet_report_compiler_flags(REPORT_THRESHOLD VERBOSE)

# ----------------------------------------------------------------------
# Describe build prerequisites:

find_ups_product(cppunit) ### MIGRATE-ACTION-RECOMMENDED: use cet_find_package() with target semantics for linking
find_ups_product(tbb) ### MIGRATE-ACTION-RECOMMENDED: use cet_find_package() with target semantics for linking

# define cppunit
cet_find_library(CPPUNIT NAMES cppunit   PATHS ENV CPPUNIT_LIB   NO_DEFAULT_PATH) ### MIGRATE-ACTION-RECOMMENDED: use of ENV ... may be UPS-dependent ### MIGRATE-ACTION-RECOMMENDED: use cet_find_package() with target semantics for linking

# ----------------------------------------------------------------------
# Describe source tree layout:

add_subdirectory(${PROJECT_NAME})  # source code

# ----------------------------------------------------------------------
# Packaging utility


#
# ======================================================================
cet_cmake_config(COMPATIBILITY SameMajorVersion)
