#pragma once

#include "regression_experiments/benchmark_function.h"

namespace regression_experiments
{

class SinusSum : public BenchmarkFunction
{
public:
  SinusSum(int nb_cycles = 1, int nb_dimensions = 1);

  virtual Eigen::MatrixXd getLimits() const override;
  virtual double sample(const Eigen::VectorXd & input) const override;
  virtual double getMax() const override;

  virtual std::string class_name() const override;
  virtual void to_xml(std::ostream &out) const override;
  virtual void from_xml(TiXmlNode *node) override;

private:
  /// Limits: [-pi * nb_cycles, pi * nb_cycles]
  int nb_cycles;
  /// Input dimensions
  int nb_dimensions;
};

class AbsDiff : public BenchmarkFunction
{
public:
  AbsDiff(double input_max = 1.0, int nb_dimensions = 1);

  virtual Eigen::MatrixXd getLimits() const override;
  virtual double sample(const Eigen::VectorXd & input) const override;
  virtual double getMax() const override;

  virtual std::string class_name() const override;
  virtual void to_xml(std::ostream &out) const override;
  virtual void from_xml(TiXmlNode *node) override;

private:
  /// Limits: [-input_max, input_max]
  double input_max;
  /// Input dimensions
  int nb_dimensions;
};

/// In this class, f(x) is divided in two parts:
/// if Exists x_i > threshold_i:
/// -> failure_value
/// else
/// -> sum(a_i * x_i)
class Discontinuity : public BenchmarkFunction
{
public:
  Discontinuity(int nb_dimensions = 1);

  virtual void setNbDimensions(int new_nb_dimensions);

  virtual Eigen::MatrixXd getLimits() const override;
  virtual double sample(const Eigen::VectorXd & input) const override;
  virtual double getMax() const override;

  virtual std::string class_name() const override;
  virtual void to_xml(std::ostream &out) const override;
  virtual void from_xml(TiXmlNode *node) override;

private:
  /// Limits: [-input_max, input_max]
  double input_max;
  /// Input dimensions
  int nb_dimensions;
  /// Thresholds
  Eigen::VectorXd thresholds;
  /// Value on failure
  double failure_value;
  /// Coefficients
  Eigen::VectorXd coeffs;
};

}
