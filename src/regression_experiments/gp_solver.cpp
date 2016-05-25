#include "regression_experiments/gp_solver.h"

#include "rosban_gp/auto_tuning.h"
#include "rosban_gp/tools.h"
#include "rosban_gp/core/squared_exponential.h"

#include "rosban_random/tools.h"

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
void GPSolver::gradients(const Eigen::MatrixXd inputs,
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
  int nb_trials = 10;
  double epsilon = std::pow(10, -6);
  int max_nb_guess = 1000;
  // Creating random initial guesses and random initial steps
  Eigen::MatrixXd initial_guesses;
  Eigen::MatrixXd initial_steps;
  initial_guesses = rosban_random::getUniformSamplesMatrix(limits, nb_trials);
  initial_steps   = rosban_random::getUniformSamplesMatrix(limits, nb_trials);
  // Preparing common data
  std::function<Eigen::VectorXd(const Eigen::VectorXd)> gradient_func;
  gradient_func = [this](const Eigen::VectorXd & guess)
    {
      return this->gp.getGradient(guess);
    };
  double best_value = std::numeric_limits<double>::lowest();
  Eigen::VectorXd best_guess = (limits.col(0) + limits.col(1)) / 2;
  // Running several rProp optimization with different starting points
  for (int trial = 0; trial < nb_trials; trial++) {
    Eigen::VectorXd current_guess;
    current_guess = rosban_gp::rProp(gradient_func,
                                     initial_guesses.col(trial),
                                     initial_steps.col(trial),
                                     limits,
                                     epsilon,
                                     max_nb_guess);
    double value = gp.getPrediction(current_guess);
    if (value > best_value) {
      best_value = value;
      best_guess = current_guess;
    }
  }
  input = best_guess;
  output = best_value;
}

}
