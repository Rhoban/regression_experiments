#include "regression_experiments/benchmark_function_factory.h"
#include "regression_experiments/solver_factory.h"
#include "regression_experiments/tools.h"

#include "rosban_regression_forests/algorithms/extra_trees.h"
#include "rosban_regression_forests/approximations/gp_approximation.h"

#include <fstream>

using namespace regression_experiments;

using rosban_gp::GaussianProcess;

int main()
{
  // Setting problem properties
  int nb_samples = 50;
  int nb_prediction_points = 1000;
  std::string function_name("sin");
  std::string solver_name("gp_forest");

  Eigen::MatrixXd samples_inputs;
  Eigen::VectorXd samples_outputs;

  Eigen::MatrixXd prediction_points, gradients;
  Eigen::VectorXd prediction_means, prediction_vars;

  buildPrediction(function_name,
                  nb_samples,
                  solver_name,
                  {nb_prediction_points},
                  samples_inputs,
                  samples_outputs,
                  prediction_points,
                  prediction_means,
                  prediction_vars,
                  gradients);

  std::ostringstream oss;
  oss << function_name << "_" << nb_samples << "_" << solver_name << ".csv";

  writePrediction(oss.str(),
                  samples_inputs,
                  samples_outputs,
                  prediction_points,
                  prediction_means,
                  prediction_vars,
                  gradients);
}
