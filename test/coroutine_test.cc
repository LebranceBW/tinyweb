#include <boost/coroutine2/all.hpp>
#include <iostream>

typedef boost::coroutines2::coroutine<long long> coro_t;

int main() {
  coro_t::pull_type source([&](coro_t::push_type& sink) {
    long long first = 1, second = 1;
    sink(first);
    sink(second);
    while (true) {
      long long third = first + second;
      first = second;
      second = third;
      sink(third);
    }
  });

  coro_t::push_type sink([&](coro_t::pull_type& source) { std::cout << source().get() << " "; });

  source(sink);
  //   for (int i = 0; i < 20; i++) {

  // source();
  //   }
  //   !source;
}