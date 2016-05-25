#include "regression_experiments/gp_forest_solver.h"

#include "rosban_regression_forests/algorithms/extra_trees.h"
#include "rosban_regression_forests/approximations/approximation_type.h"
#include "rosban_regression_forests/approximations/gp_approximation.h"

#include "rosban_gp/auto_tuning.h"
#include "rosban_gp/tools.h"
#include "rosban_gp/core/gaussian_process.h"

#include "rosban_random/tools.h"

using regression_forests::ApproximationType;
using regression_forests::GPApproximation;
using regression_forests::ExtraTrees;
using regression_forests::TrainingSet;
using regression_forests::Forest;
using regression_forests::Tree;
using regression_forests::Node;

using rosban_gp::GaussianProcess;

namespace regression_experiments
{

GPForestSolver::GPForestSolver(Type t)
  : type(t)
{
}
GPForestSolver::~GPForestSolver() {}

void GPForestSolver::solve(const Eigen::MatrixXd & inputs,
                           const Eigen::VectorXd & observations,
                           const Eigen::MatrixXd & limits)
{
  int nb_samples = observations.rows();
  ExtraTrees solver;
  solver.conf =  ExtraTrees::Config::generateAuto(limits, nb_samples,
                                                  ApproximationType::GP);

  // Updating nmin with respect to type
  switch(type)
  {
    case GPForestSolver::Type::SQRT:
      solver.conf.n_min = std::sqrt(nb_samples);
      break;
    case GPForestSolver::Type::CURT:
      solver.conf.n_min = std::pow(nb_samples, 1.0 / 3);
      break;
    case GPForestSolver::Type::LOG2:
      solver.conf.n_min = std::log2(nb_samples);
      break;
  }

  TrainingSet ts(inputs, observations);

  forest = solver.solve(ts, limits);
}

void GPForestSolver::predict(const Eigen::MatrixXd & inputs,
                             Eigen::VectorXd & means,
                             Eigen::VectorXd & vars)
{
  Eigen::VectorXd result(inputs.cols());
  means = Eigen::VectorXd::Zero(inputs.cols());
  vars = Eigen::VectorXd::Zero(inputs.cols());
  for (int point = 0; point < inputs.cols(); point++) {
    Eigen::VectorXd prediction_input = inputs.col(point);
    // Retrieving gaussian processes at the given point
    std::vector<GaussianProcess> gps;
    for (size_t tree_id = 0; tree_id < forest->nbTrees(); tree_id++) {
      const Tree & tree = forest->getTree(tree_id);
      const Node * leaf = tree.root->getLeaf(prediction_input);
      const GPApproximation * gp_approximation = dynamic_cast<const GPApproximation *>(leaf->a);
      if (gp_approximation == nullptr) {
        throw std::runtime_error("Found an approximation which is not a gaussian process");
      }
      gps.push_back(gp_approximation->gp);
    }
    // Averaging gaussian processes
    double mean, var;
    rosban_gp::getDistribParameters(prediction_input, gps, mean, var);
    means(point) = mean;
    vars(point) = var;
  }
}

void GPForestSolver::gradients(const Eigen::MatrixXd & inputs,
                               Eigen::MatrixXd & gradients)
{
  gradients = Eigen::MatrixXd::Zero(inputs.rows(), inputs.cols());
  for (int col = 0; col < inputs.cols(); col++) {
    Eigen::VectorXd input = inputs.col(col);
    Eigen::VectorXd grad = Eigen::VectorXd::Zero(input.rows());
    double total_weight = 0;
    for (size_t tree_id = 0; tree_id < forest->nbTrees(); tree_id++) {
      const Tree & tree = forest->getTree(tree_id);
      const Node * leaf = tree.root->getLeaf(input);
      const GPApproximation * gp_approximation = dynamic_cast<const GPApproximation *>(leaf->a);
      if (gp_approximation == nullptr) {
        throw std::runtime_error("Found an approximation which is not a gaussian process");
      }
      double var = gp_approximation->gp.getVariance(input);
      if (var == 0) throw std::runtime_error("GPForestSolver::gradients: var == 0");
      double weight = 1.0 / var;
      grad += gp_approximation->gp.getGradient(input) * weight;
      total_weight += weight;
    }
    gradients.col(col) = grad / total_weight;
  }
}

void GPForestSolver::getMaximum(const Eigen::MatrixXd & limits,
                                Eigen::VectorXd & input, double & output)
{
  // rProp properties
  int nb_trials = 25;
  double epsilon = std::pow(10, -6);
  int max_nb_guess = 1000;
  // Creating random initial guesses and random initial steps
  Eigen::MatrixXd initial_guesses;
  Eigen::MatrixXd initial_steps;
  initial_guesses = rosban_random::getUniformSamplesMatrix(limits, nb_trials);
  initial_steps   = rosban_random::getUniformSamplesMatrix(limits, nb_trials) / 100;
  // Preparing common data
  std::function<Eigen::VectorXd(const Eigen::VectorXd)> gradient_func;
  gradient_func = [this](const Eigen::VectorXd & guess)
    {
      Eigen::MatrixXd gradient;
      this->gradients(guess, gradient);
      // If gradient.col(0) is returned directly, results are weird
      return Eigen::VectorXd(gradient.col(0));
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
    Eigen::VectorXd values, vars;
    predict(current_guess, values, vars);
    double value = values(0);
    if (value > best_value) {
      best_value = value;
      best_guess = current_guess;
    }
  }
  input = best_guess;
  output = best_value;
}

GPForestSolver::Type loadType(const std::string &s)
{
  if (s == "SQRT") return GPForestSolver::Type::SQRT;
  if (s == "CURT") return GPForestSolver::Type::CURT;
  if (s == "LOG2") return GPForestSolver::Type::LOG2;
  throw std::runtime_error("Unknown GPForestSolver::Type: '" + s + "'");
}

std::string to_string(GPForestSolver::Type type)
{
  switch(type)
  {
    case GPForestSolver::Type::SQRT: return "SQRT";
    case GPForestSolver::Type::CURT: return "CURT";
    case GPForestSolver::Type::LOG2: return "LOG2";
  }
  throw std::runtime_error("GPForestSolver::Type unknown in to_string");
}

}
