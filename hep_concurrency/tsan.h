#ifndef hep_concurrency_tsan_h
#define hep_concurrency_tsan_h
// vim: set sw=2 expandtab :

#include <iostream>
#include <sstream>
#include <thread>

#if defined(__SANITIZE_THREAD__)
#define ANNOTATE_HAPPENS_BEFORE(addr)                                          \
  AnnotateHappensBefore(std::source_location::current().file(),                \
                        std::source_location::current().line(),                \
                        (void*)(addr))
#define ANNOTATE_HAPPENS_AFTER(addr)                                           \
  AnnotateHappensAfter(std::source_location::current().file(),                 \
                       std::source_location::current().line(),                 \
                       (void*)(addr))
#define ANNOTATE_BENIGN_RACE_SIZED(addr, size, desc)                           \
  AnnotateBenignRaceSized(std::source_location::current().file(),              \
                          std::source_location::current().line(),              \
                          (void*)(addr),                                       \
                          (unsigned long)(size),                               \
                          (desc))
#define ANNOTATE_BENIGN_RACE(addr, desc)                                       \
  AnnotateBenignRace(std::source_location::current().file(),                   \
                     std::source_location::current().line(),                   \
                     (void*)(addr),                                            \
                     (desc))
#define ANNOTATE_THREAD_IGNORE_READS_BEGIN                                     \
  AnnotateIgnoreReadsBegin(std::source_location::current().file(),             \
                           std::source_location::current().line())
#define ANNOTATE_THREAD_IGNORE_READS_END                                       \
  AnnotateIgnoreReadsEnd(std::source_location::current().file(),               \
                         std::source_location::current().line())
#define ANNOTATE_THREAD_IGNORE_WRITES_BEGIN                                    \
  AnnotateIgnoreWritesBegin(std::source_location::current().file(),            \
                            std::source_location::current().line())
#define ANNOTATE_THREAD_IGNORE_WRITES_END                                      \
  AnnotateIgnoreWritesEnd(std::source_location::current().file(),              \
                          std::source_location::current().line())
#define ANNOTATE_THREAD_IGNORE_SYNC_BEGIN                                      \
  AnnotateIgnoreSyncBegin(std::source_location::current().file(),              \
                          std::source_location::current().line())
#define ANNOTATE_THREAD_IGNORE_SYNC_END                                        \
  AnnotateIgnoreSyncEnd(std::source_location::current().file(),                \
                        std::source_location::current().line())
#define ANNOTATE_THREAD_IGNORE_BEGIN                                           \
  ANNOTATE_THREAD_IGNORE_READS_BEGIN;                                          \
  ANNOTATE_THREAD_IGNORE_WRITES_BEGIN;                                         \
  ANNOTATE_THREAD_IGNORE_SYNC_BEGIN;                                           \
  if (getenv("ART_DEBUG_IGNORE") != nullptr) {                                 \
    ++hep::concurrency::ignoreBalance_;                                        \
    std::ostringstream msg;                                                    \
    msg << "-----> tid: " << std::this_thread::get_id() << " "                 \
        << hep::concurrency::ignoreBalance_                                    \
        << " ANNOTATE_THREAD_IGNORE_BEGIN: "                                   \
        << std::source_location::current().file() << ":"                       \
        << std::source_location::current().line() << " "                       \
        << std::source_location::current().function_name() << "\n";            \
    std::cerr << msg.str();                                                    \
  }
#define ANNOTATE_THREAD_IGNORE_END                                             \
  ANNOTATE_THREAD_IGNORE_READS_END;                                            \
  ANNOTATE_THREAD_IGNORE_WRITES_END;                                           \
  ANNOTATE_THREAD_IGNORE_SYNC_END;                                             \
  if (getenv("ART_DEBUG_IGNORE") != nullptr) {                                 \
    --hep::concurrency::ignoreBalance_;                                        \
    std::ostringstream msg;                                                    \
    msg << "-----> tid: " << std::this_thread::get_id() << " "                 \
        << hep::concurrency::ignoreBalance_                                    \
        << " ANNOTATE_THREAD_IGNORE_END:   "                                   \
        << std::source_location::current().file() << ":"                       \
        << std::source_location::current().line() << " "                       \
        << std::source_location::current().function_name() << "\n";            \
    std::cerr << msg.str();                                                    \
  }
extern "C" {
void AnnotateHappensBefore(char const*, int, void*);
void AnnotateHappensAfter(char const*, int, void*);
void AnnotateBenignRaceSized(char const*,
                             int,
                             void*,
                             unsigned long,
                             char const*);
void AnnotateBenignRace(char const*, int, void*, char const*);
void AnnotateIgnoreReadsBegin(char const*, int);
void AnnotateIgnoreReadsEnd(char const*, int);
void AnnotateIgnoreWritesBegin(char const*, int);
void AnnotateIgnoreWritesEnd(char const*, int);
void AnnotateIgnoreSyncBegin(char const*, int);
void AnnotateIgnoreSyncEnd(char const*, int);
} // extern "C"
#else // defined(__SANITIZE_THREAD__)
#define ANNOTATE_HAPPENS_BEFORE(addr)
#define ANNOTATE_HAPPENS_AFTER(addr)
#define ANNOTATE_BENIGN_RACE_SIZED(addr, size, desc)
#define ANNOTATE_BENIGN_RACE(addr, desc)
#define ANNOTATE_THREAD_IGNORE_READS_BEGIN
#define ANNOTATE_THREAD_IGNORE_READS_END
#define ANNOTATE_THREAD_IGNORE_WRITES_BEGIN
#define ANNOTATE_THREAD_IGNORE_WRITES_END
#define ANNOTATE_THREAD_IGNORE_SYNC_BEGIN
#define ANNOTATE_THREAD_IGNORE_SYNC_END
#define ANNOTATE_THREAD_IGNORE_BEGIN
#define ANNOTATE_THREAD_IGNORE_END
#endif // defined(__SANITIZE_THREAD__)

namespace hep::concurrency {
  extern int intentionalDataRace_;
  extern thread_local int ignoreBalance_;
} // namespace hep::concurrency

#define INTENTIONAL_DATA_RACE(ENV_VAR)                                         \
  {                                                                            \
    if (std::getenv(#ENV_VAR)) {                                               \
      if (getenv("ART_DEBUG_DR") != nullptr) {                                 \
        std::ostringstream buf;                                                \
        buf << "-----> Making data race " #ENV_VAR " "                         \
            << "tid: " << std::this_thread::get_id() << " "                    \
            << hep::concurrency::ignoreBalance_ << " "                         \
            << std::source_location::current().file() << ":"                   \
            << std::source_location::current().line() << "\n";                 \
        std::cerr << buf.str();                                                \
      }                                                                        \
      hep::concurrency::intentionalDataRace_ =                                 \
        hep::concurrency::intentionalDataRace_ + 1;                            \
    }                                                                          \
  }

// Local Variables:
// mode: c++
// End:

#endif /* hep_concurrency_tsan_h */
