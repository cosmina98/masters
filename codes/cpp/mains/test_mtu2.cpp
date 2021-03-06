#include "mtu.hpp"
#include "test_common.hpp"

// Execute mtu2 algorithm (created by Martello and Toth and described in the
// book "Knapsack Problems" p. 100) over a hardcoded set of instances. Used to
// check if it is still working after a change.
int main(int argc, char** argv) {
  std::cout << "hbm::benchmark_pyasukp<size_t, size_t, size_t>(&hbm::mtu2, argc, argv)" << std::endl;
  return hbm::benchmark_pyasukp<size_t, size_t, size_t>(&hbm::mtu2, argc, argv);
}

