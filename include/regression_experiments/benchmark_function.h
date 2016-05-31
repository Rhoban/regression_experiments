#pragma once

#include <Eigen/Core>

#include <functional>
#include <random>

namespace regression_experiments
{

class BenchmarkFunction
{
public:
  typedef std::function<double(const Eigen::VectorXd &)> EvaluationFunction;

  std::string name;
  EvaluationFunction f;
  Eigen::MatrixXd limits;
  double observation_noise;
  /// max_{i \in limits}E(f(i))
  double function_max;

  BenchmarkFunction(const std::string & name,
                    EvaluationFunction f,
                    const Eigen::MatrixXd limits,
                    double observation_noise,
                    double function_max);
  /// Easy binding for one dimensional functions
  BenchmarkFunction(const std::string & name,
                    std::function<double(double)> f,
                    const Eigen::MatrixXd limits,
                    double observation_noise,
                    double function_max);

  /// Create samples and place them in the provided arguments
  /// Use engine if provided, otherwise, it creates its own engine
  void getUniformSamples(int nb_samples,
                         Eigen::MatrixXd & samples,
                         Eigen::VectorXd & observations,
                         std::default_random_engine * engine = NULL,
                         bool apply_noise = true);

};


}
