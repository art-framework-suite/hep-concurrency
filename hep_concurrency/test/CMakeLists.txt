include(CetTest)
cet_enable_asserts()

find_package(Catch2 REQUIRED)

# Put everything in a different export set.
cet_register_export_set(SET_NAME Testing
  NAMESPACE hep_concurrency_test SET_DEFAULT)

# Test script executes exec.
cet_make_exec(NAME ThreadSafeOutputFileStream_t NO_INSTALL
  LIBRARIES PRIVATE hep_concurrency::hep_concurrency Threads::Threads)
cet_test(runThreadSafeOutputFileStream_t.sh PREBUILT
  TEST_ARGS $<TARGET_FILE:ThreadSafeOutputFileStream_t>)

cet_test(assert_only_one_thread_test
  LIBRARIES PRIVATE
    simultaneous_function_spawner
    hep_concurrency::macros
    Threads::Threads
  TEST_PROPERTIES
    PASS_REGULAR_EXPRESSION
      "Failed assert--more than one thread accessing location"
)

foreach(test IN ITEMS
    serial_task_queue_chain_t
    serial_task_queue_t
    waiting_task_list_t
)
  cet_test(${test} USE_CATCH2_MAIN
    LIBRARIES PRIVATE hep_concurrency::hep_concurrency)
endforeach()

target_link_libraries(waiting_task_list_t PRIVATE TBB::tbb)

cet_test(simultaneous_function_spawner_t USE_CATCH2_MAIN
  LIBRARIES PRIVATE hep_concurrency::simultaneous_function_spawner
)

# Test compile-only features.
foreach(test IN ITEMS
    serial_task_queue_push_constraints_t
    serial_task_queue_chain_push_constraints_t
    make_waiting_task_t
    sanitizer_compatible_t
  )
  cet_test(${test} LIBRARIES PRIVATE hep_concurrency::hep_concurrency USE_CATCH2_MAIN)
endforeach()

# Test concurrent caching facility.
foreach (target IN ITEMS cache_handle_t cache_mt_t cache_t)
  cet_test(${target} USE_CATCH2_MAIN
    LIBRARIES PRIVATE hep_concurrency::cache TBB::tbb)
endforeach()

target_link_libraries(cache_handle_t PRIVATE cetlib_except::Catch2Matchers)
target_link_libraries(cache_t PRIVATE cetlib_except::Catch2Matchers)
