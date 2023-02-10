#include "hep_concurrency/ThreadSafeOutputFileStream.h"
// vim: set sw=2 expandtab :

#include <fstream>
#include <string>

using namespace std;

namespace hep {
  namespace concurrency {

    ThreadSafeOutputFileStream::~ThreadSafeOutputFileStream() { file_.close(); }

    ThreadSafeOutputFileStream::ThreadSafeOutputFileStream(
      string const& filename)
      : file_{filename}
    {}

    ThreadSafeOutputFileStream::operator bool() const
    {
      std::lock_guard sentry{mutex_};
      return static_cast<bool>(file_);
    }

    void
    ThreadSafeOutputFileStream::write(string&& msg)
    {
      std::lock_guard sentry{mutex_};
      file_ << std::forward<string>(msg);
    }

  } // namespace concurrency
} // namespace hep
