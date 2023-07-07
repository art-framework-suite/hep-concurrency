find_package(TBB REQUIRED EXPORT)
find_package(cetlib_except REQUIRED EXPORT)

cet_make_library(LIBRARY_NAME macros INTERFACE
  SOURCE
    assert_only_one_thread.h
    hardware_pause.h
    thread_safety_macros.h
)

cet_make_library(HEADERS_TARGET
  SOURCE
    SerialTaskQueue.cc
    SerialTaskQueueChain.cc
    ThreadSafeOutputFileStream.cc
    WaitingTask.cc
    WaitingTaskList.cc
    tsan.cc
  LIBRARIES
  PUBLIC TBB::tbb Threads::Threads
  PRIVATE hep_concurrency::macros
)

cet_make_library(LIBRARY_NAME thread_sanitize INTERFACE
  SOURCE thread_sanitize.h
  LIBRARIES INTERFACE hep_concurrency::hep_concurrency
)

cet_make_library(LIBRARY_NAME simultaneous_function_spawner INTERFACE
  SOURCE simultaneous_function_spawner.h
  LIBRARIES INTERFACE Threads::Threads
)

cet_make_library(LIBRARY_NAME cache INTERFACE
  SOURCE
    cache.h
    cache_handle.h
    detail/cache_entry.h
    detail/cache_hashers.h
    # detail/cache_key_supports.h
  LIBRARIES INTERFACE TBB::tbb cetlib_except::cetlib_except
)

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)

if (BUILD_TESTING)
  add_subdirectory(test)
endif()
