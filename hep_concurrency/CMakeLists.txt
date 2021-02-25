cet_make_library(
  LIBRARY_NAME
    ${PROJECT_NAME}
  SOURCE
    RecursiveMutex.cc
    SerialTaskQueue.cc
    SerialTaskQueueChain.cc
    ThreadSafeOutputFileStream.cc
    WaitingTask.cc
    WaitingTaskList.cc
    tsan.cc
  LIBRARIES
    TBB::tbb
)

install_headers()
install_source()

add_subdirectory(test)
