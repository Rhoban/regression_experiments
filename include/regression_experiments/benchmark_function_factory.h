#include "regression_experiments/benchmark_function.h"

#include "rosban_utils/factory.h"

namespace regression_experiments
{

class BenchmarkFunctionFactory : public rosban_utils::Factory<BenchmarkFunction>
{
public:
  BenchmarkFunctionFactory();
};


}
