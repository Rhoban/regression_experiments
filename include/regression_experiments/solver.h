#pragma once

#include "rosban_utils/serializable.h"

#include <Eigen/Core>

#include <ostream>
#include <stdexcept>

namespace regression_experiments
{

class Solver : public rosban_utils::Serializable
{
public:

  virtual ~Solver() {}

  /// Update internal structure according to the provided samples
  virtual void solve(const Eigen::MatrixXd & inputs,
                     const Eigen::VectorXd & observations,
                     const Eigen::MatrixXd & limits) = 0;

  /// Predict the outputs independently using internal structure
  virtual void predict(const Eigen::MatrixXd & inputs,
                       Eigen::VectorXd & means,
                       Eigen::VectorXd & vars) = 0;

  /// Predict the gradients independently using internal structure
  virtual void gradients(const Eigen::MatrixXd & inputs,
                         Eigen::MatrixXd & gradients) = 0;

  /// Return argmax(f) and max(f) inside the provided limits
  virtual void getMaximum(const Eigen::MatrixXd & limits,
                          Eigen::VectorXd & input, double & output) = 0;

  virtual void debugPrediction(const Eigen::VectorXd & input, std::ostream & out)
    {
      (void)input;(void)out;
      throw std::logic_error("Solver::debugPrediction: Unimplemented Method:");
    }

};

}
