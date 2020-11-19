#pragma once
extern "C" {
#include "coroutine.h"
}
#include <functional>

using CoSheduler = struct schedule;
using Corot_t = int;

class Scheduler {
public:
  CoSheduler* cosheduler;
  Scheduler() {
    cosheduler = coroutine_open();
  }
  ~Scheduler() {
    coroutine_close(cosheduler);
  }

  Corot_t new_coroutine(coroutine_func func, void* args) {
    return coroutine_new(cosheduler, func, args);
  }
  void yield() {
    coroutine_yield(cosheduler);
  };
  void resume(Corot_t id) {
    coroutine_resume(cosheduler, id);
  }
  bool isActive(Corot_t id) {
    return coroutine_status(cosheduler, id);
  }
};