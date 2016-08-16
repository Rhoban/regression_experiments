#include "regression_experiments/benchmark_function_factory.h"
#include "regression_experiments/tools.h"

#include "rosban_regression_forests/algorithms/extra_trees.h"
#include "rosban_regression_forests/approximations/gp_approximation.h"

#include "rosban_fa/trainer_factory.h"

#include <fstream>

using namespace regression_experiments;

using rosban_gp::GaussianProcess;

int main(int argc, char ** argv)
{
  if (argc < 3) {
    std::cerr << "Usage: " << argv[0] << " <function_name> <trainer_name>" << std::endl;
    exit(EXIT_FAILURE);
  }
  std::string function_name(argv[1]);
  std::string trainer_name(argv[2]);
  // Setting problem properties
  int nb_samples = 50;
  int nb_prediction_points = 1000;

  Eigen::MatrixXd samples_inputs;
  Eigen::VectorXd samples_outputs;

  Eigen::MatrixXd prediction_points, gradients;
  Eigen::VectorXd prediction_means, prediction_vars;

  buildPrediction(function_name,
                  nb_samples,
                  trainer_name,
                  {nb_prediction_points},
                  samples_inputs,
                  samples_outputs,
                  prediction_points,
                  prediction_means,
                  prediction_vars,
                  gradients);

  std::ostringstream oss;
  oss << function_name << "_" << nb_samples << "_" << trainer_name << ".csv";

  writePrediction(oss.str(),
                  samples_inputs,
                  samples_outputs,
                  prediction_points,
                  prediction_means,
                  prediction_vars,
                  gradients);
}
