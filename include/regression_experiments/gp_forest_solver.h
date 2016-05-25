#pragma once

#include "regression_experiments/solver.h"

#include "rosban_regression_forests/core/forest.h"

#include <Eigen/Core>

namespace regression_experiments
{

class GPForestSolver : public Solver
{
public:
  /// Which type of GPForest is used
  /// - SQRT: |samples|^{1/2} samples per node min
  /// - CURT: |samples|^{1/3} samples per node min
  /// - LOG2: log_2(|samples|)  sampels per node min
  enum class Type
  { SQRT, CURT, LOG2};

  GPForestSolver(Type t = Type::SQRT);

  virtual ~GPForestSolver();

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
  Type type;
};
GPForestSolver::Type loadType(const std::string &s);
std::string to_string(GPForestSolver::Type type);

}
