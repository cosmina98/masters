#include <iostream>
#include <fstream>
#include <array>

#include "test_common.hpp"

using namespace std;
using namespace std::chrono;

int run_ukp(void(*ukp_solver)(ukp_instance_t &, ukp_solution_t &, bool), const string& path, run_t &run, size_t num_times/* = 1*/) {
  ifstream f(path);

  if (f.is_open())
  {
    ukp_instance_t ukpi;
    ukp_solution_t &ukps = run.result;

    read_sukp_instance(f, ukpi);

    for (size_t i = 0; i < num_times; ++i) {
      steady_clock::time_point t1 = steady_clock::now();
      (*ukp_solver)(ukpi, ukps, false);
      steady_clock::time_point t2 = steady_clock::now();
      duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
      run.times.push_back(time_span);
    }
  } else {
    cout << "Couldn't open file" << path << endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

int benchmark_pyasukp(void(*ukp_solver)(ukp_instance_t &, ukp_solution_t &, bool), size_t num_times/* = 10*/) {
  array<instance_data_t, /*sizeof(instance_data_t)**/8> instances_data = {{
    { "corepb", 10077782 },
    { "exnsd16", 1029680 },
    { "exnsd18", 1112131 },
    { "exnsd20",  1026086 },
    { "exnsd26", 1027564 },
    { "exnsdbis10", 1028035 },
    { "exnsdbis18",  1037156 },
    { "exnsds12", 3793952 }
  }};

  bool everything_ok = true;
  for (size_t i = 0; i < instances_data.size(); ++i) {
    string path = "../../data/sukp/" + instances_data[i].name + ".sukp";
    cout << path << endl;

    run_t run;
    int status = run_ukp(ukp_solver, path, run, num_times);

    if (status == EXIT_SUCCESS) {
      size_t expected = instances_data[i].expected_opt;
      size_t obtained = run.result.opt;
      cout << "Expected result: " << expected << endl;
      cout << "Obtained result: " << obtained << endl;
      everything_ok = everything_ok && expected == obtained;

      for (auto it = run.times.begin(); it != run.times.end(); ++it) {
        cout << it->count() << endl;
      }

      cout << endl;
    } else {
      return EXIT_FAILURE;
    }
  }

  if (everything_ok) {
    cout << "Everything is ok." << endl;
  } else {
    cout << "The expected and obtained optimal values of some instances differ, check." << endl;
  }

  return EXIT_SUCCESS;
}

int main_take_path(void(*ukp_solver)(ukp_instance_t &, ukp_solution_t &, bool), int argc, char** argv) {
  if (argc != 2) {
    cout << "usage: a.out data.sukp" << endl;
    return EXIT_FAILURE;
  }

  string path(argv[1]);
  cout << path << endl;

  run_t run;

  int status = run_ukp(ukp_solver, path, run);

  if (status == EXIT_SUCCESS) {
    cout << run.result.opt << endl;
    
    for (auto it = run.times.begin(); it != run.times.end(); ++it) {
      cout << it->count() << endl;
    }
    cout << endl;
    return EXIT_SUCCESS;
  } else {
    cout << "There was some problem with this instance" << endl;
    return EXIT_FAILURE;
  }
}
