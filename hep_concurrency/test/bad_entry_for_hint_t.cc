#include "hep_concurrency/cache.h"

using namespace hep::concurrency;

namespace {
  struct test_key{ 
    int num;  

    std::size_t hash() const {
      return std::hash<int>{}(num);
    }

    bool operator==(test_key const & other) const {
      return num == other.num;
    }
  };
}

int main(){
  cache<test_key, int> cache;
  cache.emplace(test_key{1}, 0);
  cache.entry_for(cache_handle<test_key, int>::invalid(), 2);

}
