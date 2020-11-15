include(CetTest)
cet_enable_asserts()

cet_find_package(Threads REQUIRED)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)

cet_test(assert_only_one_thread_test
  LIBRARIES Threads::Threads
  TEST_PROPERTIES
  PASS_REGULAR_EXPRESSION "Failed assert--more than one thread accessing location")

# Catch2 tests
set(catch2_tests
    cache_handle_t
    cache_mt_t
    cache_t
    simultaneous_function_spawner_t)

foreach(test IN LISTS catch2_tests)
  cet_test(${test}
           USE_CATCH2_MAIN
           LIBRARIES PRIVATE TBB::tbb cetlib_except::cetlib_except Catch2::Catch2)
endforeach()

# Don't forget the threads library where needed.
target_link_libraries(simultaneous_function_spawner_t PRIVATE Threads::Threads)

#set(source_libraries hep_concurrency ${CPPUNIT} TBB::tbb pthread)

#cet_make_exec(ThreadSafeOutputFileStream_t LIBRARIES pthread hep_concurrency)
#cet_test(runThreadSafeOutputFileStream_t.sh PREBUILT)

# Unfortunately, the SerialTaskQueue tests expose TBB assertion
# failures, that appear to be internal issues.  Until TBB fixes
# things, we are forced to disable the tests for the DEBUG build.

#string(TOUPPER ${CMAKE_BUILD_TYPE} BTYPE_UC)
#if(NOT ${BTYPE_UC} STREQUAL "DEBUG")
  #cet_test(serialtaskqueuechain_t LIBRARIES ${source_libraries})
  #cet_test(serialtaskqueue_t LIBRARIES ${source_libraries})
#endif()

#cet_test(waitingtasklist_t LIBRARIES ${source_libraries})
