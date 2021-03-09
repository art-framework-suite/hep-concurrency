if (HEP_CONCURRENCY)
  list(PREPEND HEP_CONCURRENCY
    hep_concurrency::thread_sanitize
  )
  list(APPEND HEP_CONCURRENCY
    hep_concurrency::cache
    hep_concurrency::simultaneous_function_spawner
 )
endif()