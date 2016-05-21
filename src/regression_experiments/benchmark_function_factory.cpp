#include "regression_experiments/benchmark_function_factory.h"

namespace regression_experiments
{

BenchmarkFunctionFactory::BenchmarkFunctionFactory()
{
  registerBuilder("sin",
                  [](TiXmlNode * node)
                  {
                    (void)node;
                    Eigen::MatrixXd limits(1,2);
                    limits << - 4 * M_PI, 4 * M_PI; 
                    return new BenchmarkFunction("sin", &sin, limits, 0.05);
                  });
  registerBuilder("deterministic_sin",
                  [](TiXmlNode * node)
                  {
                    (void)node;
                    Eigen::MatrixXd limits(1,2);
                    limits << - 4 * M_PI, 4 * M_PI; 
                    return new BenchmarkFunction("deterministic_sin", &sin, limits, 0.0);
                  });
  registerBuilder("abs",
                  [](TiXmlNode * node)
                  {
                    (void)node;
                    Eigen::MatrixXd limits(1,2);
                    limits << - 2 , 2; 
                    return new BenchmarkFunction("abs", &fabs, limits, 0.05);
                  });
  registerBuilder("deterministic_abs",
                  [](TiXmlNode * node)
                  {
                    (void)node;
                    Eigen::MatrixXd limits(1,2);
                    limits << - 2 , 2; 
                    return new BenchmarkFunction("deterministic_abs", &fabs, limits, 0.0);
                  });
  registerBuilder("binary",
                  [](TiXmlNode * node)
                  {
                    (void)node;
                    Eigen::MatrixXd limits(1,2);
                    limits << - 2 , 2; 
                    return new BenchmarkFunction("binary",
                                                 [](const Eigen::VectorXd & input)
                                                 { return input(0) > 0; },
                                                 limits, 0.05);
                  });
  registerBuilder("deterministic_binary",
                  [](TiXmlNode * node)
                  {
                    (void)node;
                    Eigen::MatrixXd limits(1,2);
                    limits << - 2 , 2; 
                    return new BenchmarkFunction("deterministic_binary",
                                                 [](const Eigen::VectorXd & input)
                                                 { return input(0) > 0; },
                                                 limits, 0.0);
                  });
}

}
