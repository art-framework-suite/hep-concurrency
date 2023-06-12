#include "WaitingTask.h"

int foo(int b, int a, int r){
  return b + a + r;
}

int main(){
  hep::concurrency::make_waiting_task(foo, 1, 2, 3);
}
