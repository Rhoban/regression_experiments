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
};


}
