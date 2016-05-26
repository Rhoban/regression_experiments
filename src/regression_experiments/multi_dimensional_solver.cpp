#include "regression_experiments/multi_dimensional_solver.h"

#include "regression_experiments/solver_factory.h"

namespace regression_experiments
{

MultiDimensionalSolver::MultiDimensionalSolver(const std::string & solver_name_)
  : solver_name(solver_name_)
{
}

void MultiDimensionalSolver::solve(const Eigen::MatrixXd & inputs,
                                   const Eigen::MatrixXd & observations,
                                   const Eigen::MatrixXd & limits)
{
  if (inputs.cols() != observations.rows() ||
      inputs.rows() != observations.cols()) {
    throw std::runtime_error("MultiDimensionalSolver::solve: dimension mismatch");
  }
  solvers.clear();
  SolverFactory sf;
  for (int dim = 0; dim < inputs.rows(); dim++) {
    std::unique_ptr<Solver> solver(sf.build(solver_name));
    solver->solve(inputs, observations.col(dim), limits);
    solvers.push_back(std::move(solver));
  }
}
void MultiDimensionalSolver::predict(const Eigen::MatrixXd & inputs,
                                     Eigen::MatrixXd & means,
                                     Eigen::MatrixXd & vars)
{
  if (inputs.rows() != (int)solvers.size()) {
    throw std::runtime_error("MultiDimensionalSolver::predict: dimension mismatch");
  }
  means = Eigen::MatrixXd::Zero(inputs.cols(), inputs.rows());
  vars = Eigen::MatrixXd::Zero(inputs.cols(), inputs.rows());
  for (int dim = 0; dim < inputs.rows(); dim++) {
    Eigen::VectorXd col_means, col_vars;
    solvers[dim]->predict(inputs, col_means, col_vars);
    means.col(dim) = col_means;
    vars.col(dim) = col_vars;
  }
}

}
