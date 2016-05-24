#pragma once

#include "regression_experiments/solver.h"

#include "rosban_regression_forests/core/forest.h"

#include <Eigen/Core>

namespace regression_experiments
{

class PWCForestSolver : public Solver
{
public:

  virtual ~PWCForestSolver();

  /// Update internal structure according to the provided samples
  virtual void solve(const Eigen::MatrixXd & inputs,
                     const Eigen::VectorXd & observations,
                     const Eigen::MatrixXd & limits) override;

  /// Predict the outputs independently using internal structure
  virtual void predict(const Eigen::MatrixXd inputs,
                       Eigen::VectorXd & means,
                       Eigen::VectorXd & vars) override;

private:
  std::unique_ptr<regression_forests::Forest> forest;
};

}
