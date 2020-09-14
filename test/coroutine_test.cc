#include <boost/coroutine2/all.hpp>
#include <iostream>

typedef boost::coroutines2::coroutine<int> coro_t;

int main() {
  coro_t::push_type sink2([&](coro_t::pull_type& source) {
    while (true) {
      source();
      std::cout << source.get() << " " << std::endl;
    }
  });

  coro_t::pull_type source([&](coro_t::push_type& sink) {
    int first = 1, second = 1;
    sink2(first);
    sink2(second);

    for (int i = 0; i < 8; i++) {
      int third = first + second;
      first = second;
      second = third;
      sink2(third);
    }
  });

}