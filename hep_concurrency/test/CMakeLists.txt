include(CetTest)
cet_enable_asserts()

find_package(Threads REQUIRED)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)

cet_test(assert_only_one_thread_test
  LIBRARIES PRIVATE Threads::Threads
  TEST_PROPERTIES PASS_REGULAR_EXPRESSION "Failed assert--more than one thread accessing location")
cet_test(cache_handle_t USE_CATCH_MAIN LIBRARIES ${TBB} cetlib_except)
cet_test(cache_mt_t USE_CATCH_MAIN LIBRARIES ${TBB} cetlib_except)
cet_test(cache_t USE_CATCH_MAIN LIBRARIES ${TBB} cetlib_except)
cet_test(simultaneous_function_spawner_t USE_CATCH_MAIN LIBRARIES Threads::Threads)

#set(source_libraries hep_concurrency ${CPPUNIT} ${TBB} pthread)

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
