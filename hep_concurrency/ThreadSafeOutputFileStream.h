#ifndef hep_concurrency_ThreadSafeOutputFileStream_h
#define hep_concurrency_ThreadSafeOutputFileStream_h
// vim: set sw=2 expandtab :

#include <fstream>
#include <mutex>
#include <string>

namespace hep::concurrency {

  class ThreadSafeOutputFileStream {
    // Data Members.
  private:
    mutable std::recursive_mutex mutex_{};
    std::ofstream file_{};
    // Special Member Functions.
  public:
    ~ThreadSafeOutputFileStream();
    ThreadSafeOutputFileStream(std::string const& filename);
    // API
  public:
    explicit operator bool() const;
    void write(std::string&& msg);
  };

} // namespace hep::concurrency

#endif /* hep_concurrency_ThreadSafeOutputFileStream_h */

// Local Variables:
// mode: c++
// End:
