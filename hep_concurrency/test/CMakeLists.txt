include(CetTest)
cet_enable_asserts()

cet_find_package(Threads REQUIRED)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)

cet_test(assert_only_one_thread_test
  LIBRARIES PRIVATE Threads::Threads
  TEST_PROPERTIES
  PASS_REGULAR_EXPRESSION "Failed assert--more than one thread accessing location")

# Catch2 tests
set(catch2_tests
    cache_handle_t
    cache_mt_t
    cache_t
    serial_task_queue_t
    serial_task_queue_chain_t
    simultaneous_function_spawner_t
    waiting_task_list_t)

foreach(test IN LISTS catch2_tests)
  cet_test(${test}
           USE_CATCH2_MAIN
           LIBRARIES PRIVATE
             hep_concurrency::hep_concurrency
             cetlib_except::cetlib_except
             Catch2::Catch2
             Threads::Threads)
endforeach()

#cet_make_exec(ThreadSafeOutputFileStream_t LIBRARIES pthread hep_concurrency)
#cet_test(runThreadSafeOutputFileStream_t.sh PREBUILT)
