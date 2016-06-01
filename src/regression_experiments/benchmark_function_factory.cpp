#include "regression_experiments/benchmark_function_factory.h"
#include "regression_experiments/basic_functions.h"

namespace regression_experiments
{

BenchmarkFunctionFactory::BenchmarkFunctionFactory()
{
  registerBuilder("sinus_sum",
                  [](TiXmlNode * node)
                  {
                    BenchmarkFunction * f = new SinusSum();
                    f->from_xml(node);
                    return f;
                  });
  registerBuilder("abs_diff",
                  [](TiXmlNode * node)
                  {
                    BenchmarkFunction * f = new AbsDiff();
                    f->from_xml(node);
                    return f;
                  });
}

}
