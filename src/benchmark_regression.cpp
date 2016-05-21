#include "regression_experiments/benchmark_function_factory.h"
#include "regression_experiments/solver_factory.h"
#include "regression_experiments/tools.h"

#include "rosban_regression_forests/algorithms/extra_trees.h"
#include "rosban_regression_forests/approximations/gp_approximation.h"

#include "rosban_random/tools.h"

#include <fstream>

using namespace regression_experiments;

using rosban_gp::GaussianProcess;

int main(int argc, char ** argv)
{
  // Setting benchmark properties
  int nb_prediction_points = 200;
  int nb_trials_per_type = 20;

  std::vector<int> nb_samples_vec;
  for (int i = 1; i <= 8; i++) {
    nb_samples_vec.push_back(25 * i);
  }
  std::vector<std::string> function_names = {"abs",
                                             "sin",
                                             "binary",
                                             "deterministic_abs",
                                             "deterministic_sin",
                                             "deterministic_binary"};
  std::vector<std::string> solver_names = {"gp_forest","gp"};

  // Creating random engine
  auto engine = rosban_random::getRandomEngine();

  std::ofstream out;
  out.open("benchmark_regression.csv");
  // Write csv header
  out << "function_name,"
      << "solver,"
      << "nb_samples,"
      << "smse,"
      << "learning_time,"
      << "prediction_time"
      << std::endl;

  for (const std::string & function_name : function_names) {
    for (const std::string & solver_name : solver_names) {
      for (int nb_samples : nb_samples_vec) {
        for (int trial = 0; trial < nb_trials_per_type; trial++) {
          double smse, learning_time, prediction_time;
          runBenchmark(function_name,
                       nb_samples,
                       solver_name,
                       nb_prediction_points,
                       smse,
                       learning_time,
                       prediction_time,
                       &engine);
          out << function_name   << ","
              << solver_name     << ","
              << nb_samples      << ","
              << smse            << ","
              << learning_time   << ","
              << prediction_time << std::endl;
        }
      }
    }
  }
}
