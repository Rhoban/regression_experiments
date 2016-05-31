#pragma once

#include "regression_experiments/solver.h"

#include "rosban_gp/core/gaussian_process.h"
#include "rosban_gp/gradient_ascent/randomized_rprop.h"


#include <Eigen/Core>

namespace regression_experiments
{

class GPSolver : public Solver
{
public:

  virtual ~GPSolver() {}

  /// Update internal structure according to the provided samples
  virtual void solve(const Eigen::MatrixXd & inputs,
                     const Eigen::VectorXd & observations,
                     const Eigen::MatrixXd & limits) override;

  /// Predict the outputs independently using internal structure
  virtual void predict(const Eigen::MatrixXd & inputs,
                       Eigen::VectorXd & means,
                       Eigen::VectorXd & vars) override;

  virtual void gradients(const Eigen::MatrixXd & inputs,
                         Eigen::MatrixXd & gradients) override;

  virtual void getMaximum(const Eigen::MatrixXd & limits,
                          Eigen::VectorXd & input, double & output) override;

  virtual std::string class_name() const override;
  virtual void to_xml(std::ostream &out) const override;
  virtual void from_xml(TiXmlNode *node) override;

private:
  rosban_gp::GaussianProcess gp;
  rosban_gp::RandomizedRProp::Config conf;
};

}
