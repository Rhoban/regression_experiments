#pragma once

#include "rosban_utils/serializable.h"

#include <Eigen/Core>

#include <random>

namespace regression_experiments
{

class BenchmarkFunction : public rosban_utils::Serializable
{
public:

  BenchmarkFunction(double observation_noise = 0);

  /// Return the limits for the inputs parameters
  virtual Eigen::MatrixXd getLimits() const = 0;

  /// Return the value at given input without any noise observation
  virtual double sample(const Eigen::VectorXd & input) const = 0;

  /// Return the maximal value of the function, throw a runtime_error if it is not overriden
  virtual double getMax() const;

  /// Create samples and place them in the provided arguments
  /// Use engine if provided, otherwise, it creates its own engine
  void getUniformSamples(int nb_samples,
                         Eigen::MatrixXd & samples,
                         Eigen::VectorXd & observations,
                         std::default_random_engine * engine = NULL,
                         bool apply_noise = true) const;

  virtual void to_xml(std::ostream &out) const override;
  virtual void from_xml(TiXmlNode *node) override;

protected:
  double observation_noise;

};


}
