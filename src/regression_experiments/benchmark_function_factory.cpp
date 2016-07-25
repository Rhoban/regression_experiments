#include "regression_experiments/benchmark_function_factory.h"
#include "regression_experiments/basic_functions.h"

namespace regression_experiments
{

BenchmarkFunctionFactory::BenchmarkFunctionFactory()
{
  registerBuilder("sinus_sum", [](){return std::unique_ptr<BenchmarkFunction>(new SinusSum);});
  registerBuilder("abs_diff" , [](){return std::unique_ptr<BenchmarkFunction>(new AbsDiff); });
}

}
