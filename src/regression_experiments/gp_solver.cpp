#include "regression_experiments/gp_solver.h"

#include "rosban_gp/auto_tuning.h"
#include "rosban_gp/tools.h"
#include "rosban_gp/core/squared_exponential.h"

#include "rosban_random/tools.h"

#include <iostream>

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
  std::unique_ptr<CovarianceFunction> cov_func(new SquaredExponential(inputs.rows()));
  gp = GaussianProcess(inputs, observations, std::move(cov_func));
  gp.autoTune(conf);
}

void GPSolver::predict(const Eigen::MatrixXd & inputs,
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
void GPSolver::gradients(const Eigen::MatrixXd & inputs,
                         Eigen::MatrixXd & gradients)
{
  gradients = Eigen::MatrixXd::Zero(inputs.rows(), inputs.cols());
  for (int col = 0; col < inputs.cols(); col++) {
    gradients.col(col) = gp.getGradient(inputs.col(col));
  }
}

void GPSolver::getMaximum(const Eigen::MatrixXd & limits,
                          Eigen::VectorXd & input, double & output)
{
  // rProp properties
  int nb_trials = 100;
  double epsilon = std::pow(10, -6);
  int max_nb_guess = 2000;
  // Preparing functions
  std::function<Eigen::VectorXd(const Eigen::VectorXd)> gradient_func;
  gradient_func = [this](const Eigen::VectorXd & guess)
    {
      Eigen::MatrixXd gradient;
      this->gradients(guess, gradient);
      return Eigen::VectorXd(gradient.col(0));
    };
  std::function<double(const Eigen::VectorXd)> scoring_func;
  scoring_func = [this](const Eigen::VectorXd & guess)
    {
      Eigen::VectorXd values, vars;
      this->predict(guess, values, vars);
      return values(0);
    };
  // Performing multiple rProp and conserving the best candidate
  Eigen::VectorXd best_guess;
  best_guess = rosban_gp::randomizedRProp(gradient_func, scoring_func, limits,
                                          epsilon, nb_trials, max_nb_guess);
  input = best_guess;
  output = scoring_func(best_guess);
}

std::string GPSolver::class_name() const
{
  return "gp_solver";
}

void GPSolver::to_xml(std::ostream &out) const
{
  conf.write("conf", out);
}

void GPSolver::from_xml(TiXmlNode *node)
{
  conf.tryRead(node, "conf");
}

}
