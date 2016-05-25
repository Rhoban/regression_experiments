#include "regression_experiments/pwl_forest_solver.h"

#include "rosban_regression_forests/algorithms/extra_trees.h"
#include "rosban_regression_forests/approximations/approximation_type.h"
#include "rosban_regression_forests/approximations/pwl_approximation.h"

using regression_forests::ApproximationType;
using regression_forests::PWLApproximation;
using regression_forests::ExtraTrees;
using regression_forests::TrainingSet;
using regression_forests::Forest;
using regression_forests::Tree;
using regression_forests::Node;

namespace regression_experiments
{

PWLForestSolver::~PWLForestSolver() {}

void PWLForestSolver::solve(const Eigen::MatrixXd & inputs,
                            const Eigen::VectorXd & observations,
                            const Eigen::MatrixXd & limits)
{
  ExtraTrees solver;
  solver.conf =  ExtraTrees::Config::generateAuto(limits,
                                                  observations.rows(),
                                                  ApproximationType::PWL);
  TrainingSet ts(inputs, observations);

  forest = solver.solve(ts, limits);
}

void PWLForestSolver::predict(const Eigen::MatrixXd inputs,
                             Eigen::VectorXd & means,
                             Eigen::VectorXd & vars)
{
  Eigen::VectorXd result(inputs.cols());
  means = Eigen::VectorXd::Zero(inputs.cols());
  vars = Eigen::VectorXd::Zero(inputs.cols());
  for (int point = 0; point < inputs.cols(); point++) {
    means(point) = forest->getValue(inputs.col(point));
    vars(point)  = forest->getVar(inputs.col(point));
  }
}

void PWLForestSolver::gradients(const Eigen::MatrixXd inputs,
                               Eigen::MatrixXd & gradients)
{
  throw std::runtime_error("PWLForestSolver::gradients: not implemented");
}

void PWLForestSolver::getMaximum(const Eigen::MatrixXd & limits,
                                 Eigen::VectorXd & input, double & output)
{
  throw std::runtime_error("PWLForestSolver::getMaximum: not implemented");
}

}
