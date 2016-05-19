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

void GPForestSolver::solve(const Eigen::MatrixXd & inputs,
                           const Eigen::VectorXd & observations,
                           const Eigen::MatrixXd & limits)
{
  ExtraTrees solver;
  solver.conf =  ExtraTrees::Config::generateAuto(limits,
                                                  observations.rows(),
                                                  ApproximationType::GP);
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

}
