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
                    limits << - M_PI, M_PI; 
                    return new BenchmarkFunction("sin", &sin, limits, 0.05, 1);
                  });
  registerBuilder("deterministic_sin",
                  [](TiXmlNode * node)
                  {
                    (void)node;
                    Eigen::MatrixXd limits(1,2);
                    limits << - M_PI, M_PI; 
                    return new BenchmarkFunction("deterministic_sin", &sin, limits, 0.0, 1);
                  });
  registerBuilder("abs",
                  [](TiXmlNode * node)
                  {
                    (void)node;
                    Eigen::MatrixXd limits(1,2);
                    limits << - 2 , 2; 
                    return new BenchmarkFunction("abs", &fabs, limits, 0.05, 2);
                  });
  registerBuilder("deterministic_abs",
                  [](TiXmlNode * node)
                  {
                    (void)node;
                    Eigen::MatrixXd limits(1,2);
                    limits << - 2 , 2; 
                    return new BenchmarkFunction("deterministic_abs", &fabs, limits, 0.0, 2);
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
                                                 limits, 0.05, 1);
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
                                                 limits, 0.0, 1);
                  });
  registerBuilder("ternary",
                  [](TiXmlNode * node)
                  {
                    (void)node;
                    Eigen::MatrixXd limits(1,2);
                    limits << - 3 , 3; 
                    return new BenchmarkFunction("ternary",
                                                 [](const Eigen::VectorXd & input)
                                                 { return std::fabs(input(0)) < 1; },
                                                 limits, 0.05, 1);
                  });
  registerBuilder("deterministic_ternary",
                  [](TiXmlNode * node)
                  {
                    (void)node;
                    Eigen::MatrixXd limits(1,2);
                    limits << - 3 , 3; 
                    return new BenchmarkFunction("deterministic_ternary",
                                                 [](const Eigen::VectorXd & input)
                                                 { return std::fabs(input(0)) < 1; },
                                                 limits, 0.0, 1);
                  });
  registerBuilder("sinus_2dim",
                  [](TiXmlNode * node)
                  {
                    (void)node;
                    Eigen::MatrixXd limits(2,2);
                    limits << -M_PI , M_PI, -M_PI , M_PI; 
                    return new BenchmarkFunction("sinus_2dim",
                                                 [](const Eigen::VectorXd & input)
                                                 { 
                                                   return sin(input(0))
                                                     + sin(input(1));
                                                 },
                                                 limits, 0.01, 2);
                  });
  registerBuilder("binary_2dim",
                  [](TiXmlNode * node)
                  {
                    (void)node;
                    Eigen::MatrixXd limits(2,2);
                    limits << -3 , 3, -3 , 3; 
                    return new BenchmarkFunction("binary_2dim",
                                                 [](const Eigen::VectorXd & input)
                                                 { 
                                                   return input(0) > 0
                                                     && input(1) > 1;
                                                 },
                                                 limits, 0.01, 1);
                  });
  registerBuilder("sinus_3dim",
                  [](TiXmlNode * node)
                  {
                    (void)node;
                    Eigen::MatrixXd limits(3,2);
                    limits << -M_PI , M_PI, -M_PI , M_PI, -M_PI , M_PI; 
                    return new BenchmarkFunction("sinus_3dim",
                                                 [](const Eigen::VectorXd & input)
                                                 { 
                                                   return sin(input(0))
                                                     + sin(input(1))
                                                     + sin(input(2));
                                                 },
                                                 limits, 0.01, 3);
                  });
  registerBuilder("binary_3dim",
                  [](TiXmlNode * node)
                  {
                    (void)node;
                    Eigen::MatrixXd limits(3,2);
                    limits << -1 , 1, -1 , 1, -1 , 1; 
                    return new BenchmarkFunction("binary_3dim",
                                                 [](const Eigen::VectorXd & input)
                                                 { 
                                                   return input(0) > 0
                                                     && input(1) > 0
                                                     && input(2) > 0;
                                                 },
                                                 limits, 0.01, 1);
                  });
}

}
