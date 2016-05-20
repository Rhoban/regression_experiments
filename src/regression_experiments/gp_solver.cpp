#include "regression_experiments/gp_solver.h"

#include "rosban_gp/auto_tuning.h"
#include "rosban_gp/tools.h"
#include "rosban_gp/core/squared_exponential.h"

using rosban_gp::CovarianceFunction;
using rosban_gp::GaussianProcess;
using rosban_gp::SquaredExponential;

namespace regression_experiments
{

void GPSolver::solve(const Eigen::MatrixXd & inputs,
                     const Eigen::VectorXd & observations,
                     const Eigen::MatrixXd & limits)
{
  (void) limits;
  // Creating GP
  std::unique_ptr<CovarianceFunction> cov_func(new SquaredExponential());
  gp = GaussianProcess(inputs, observations, std::move(cov_func));
  // Run gradient optimization
  double epsilon = std::pow(10,-6);
  rosban_gp::rProp(gp, gp.getParametersGuess(), gp.getParametersStep(),
                   gp.getParametersLimits(), epsilon);
  gp.updateInternal();
}

void GPSolver::predict(const Eigen::MatrixXd inputs,
                       Eigen::VectorXd & means,
                       Eigen::VectorXd & vars)
{
  Eigen::VectorXd result(inputs.cols());
  means = Eigen::VectorXd::Zero(inputs.cols());
  vars = Eigen::VectorXd::Zero(inputs.cols());
  for (int point = 0; point < inputs.cols(); point++) {
    Eigen::VectorXd prediction_input = inputs.col(point);
    double mean, var;
    gp.getDistribParameters(prediction_input, mean, var);
    means(point) = mean;
    vars(point) = var;
  }
}

}
