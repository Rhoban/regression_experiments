#pragma once

#include "regression_experiments/benchmark_function.h"

namespace regression_experiments
{

class SinusSum : public BenchmarkFunction
{
public:
  SinusSum(int nb_cycles = 1, int nb_dimensions = 1);

  virtual Eigen::MatrixXd getLimits() override;
  virtual double sample(const Eigen::VectorXd & input) override;
  virtual double getMax() override;

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

  virtual Eigen::MatrixXd getLimits() override;
  virtual double sample(const Eigen::VectorXd & input) override;
  virtual double getMax() override;

  virtual std::string class_name() const override;
  virtual void to_xml(std::ostream &out) const override;
  virtual void from_xml(TiXmlNode *node) override;

private:
  /// Limits: [-input_max, input_max]
  double input_max;
  /// Input dimensions
  int nb_dimensions;
};

}
