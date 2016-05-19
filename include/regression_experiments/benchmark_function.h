#pragma once

#include <Eigen/Core>

#include <functional>

namespace regression_experiments
{

class BenchmarkFunction
{
public:
  typedef std::function<double(const Eigen::VectorXd &)> EvaluationFunction;

  std::string name;
  EvaluationFunction f;
  Eigen::MatrixXd limits;

  BenchmarkFunction(const std::string & name,
                    EvaluationFunction f,
                    const Eigen::MatrixXd limits);
  /// Easy binding for one dimensional functions
  BenchmarkFunction(const std::string & name,
                    std::function<double(double)> f,
                    const Eigen::MatrixXd limits);

  /// Create samples and place them in the provided arguments
  /// Use engine if provided, otherwise, it creates its own engine
  void getUniformSamples(int nb_samples,
                         Eigen::MatrixXd & samples,
                         Eigen::VectorXd & observations,
                         std::default_random_engine * engine = NULL);

};


}
