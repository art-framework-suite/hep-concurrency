#include "hep_concurrency/SerialTaskQueue.h"

int num(int mun){
  mun = 2;
  return mun;
}

int main(){
  tbb::task_group group;
  hep::concurrency::SerialTaskQueue queue{group};
  queue.push(num(3));
  return 0;
}
  
