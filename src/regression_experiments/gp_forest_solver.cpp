#include "regression_experiments/gp_forest_solver.h"

#include "rosban_regression_forests/algorithms/extra_trees.h"
#include "rosban_regression_forests/approximations/approximation_type.h"
#include "rosban_regression_forests/approximations/gp_approximation.h"

#include "rosban_gp/tools.h"
#include "rosban_gp/core/gaussian_process.h"

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

void GPForestSolver::predict(const Eigen::MatrixXd inputs,
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

void GPForestSolver::gradients(const Eigen::MatrixXd inputs,
                               Eigen::MatrixXd & gradients)
{
  throw std::runtime_error("GPForestSolver::gradients: not implemented");
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
