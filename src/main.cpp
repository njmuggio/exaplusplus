#include <chrono>
#include <iostream>

#include "epp.hpp"

using namespace epp;

int main(int argc, char** pArgv)
{
  if (argc != 2)
  {
    std::cout << "Usage: " << pArgv[0] << " <script>" << '\n';
    return 1;
  }

  try
  {
    auto start = std::chrono::steady_clock::now();
    Network network(pArgv[1]);
    auto stop = std::chrono::steady_clock::now();
    auto msec = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    std::cout << "Loaded program in " << msec.count() << "ms\n";

    start = std::chrono::steady_clock::now();
    RunStats stats = network.run();
    stop = std::chrono::steady_clock::now();
    msec = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    std::cout << "Executed program in " << msec.count() << "ms\n";

    std::cout << "Size:     " << stats.size << '\n';
    std::cout << "Cycles:   " << stats.cycles << '\n';
    std::cout << "Activity: " << stats.activity << '\n';
  }
  catch (const Error& exc)
  {
    std::cerr << exc.what() << '\n';
  }
}
