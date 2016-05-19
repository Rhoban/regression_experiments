#include "regression_experiments/benchmark_function.h"


namespace regression_experiments
{

BenchmarkFunction::BenchmarkFunction(const std::string & name_,
                                     EvaluationFunction f_,
                                     const Eigen::MatrixXd limits_)
  : name(name_), f(f_), limits(limits_)
{}

BenchmarkFunction::BenchmarkFunction(const std::string & name_,
                                     std::function<double(double)> f,
                                     const Eigen::MatrixXd limits_)
  : BenchmarkFunction(name_,
                      [f](const Eigen::VectorXd & input)
                      { return f(input(0)); },
                      limits_)
{}

}
