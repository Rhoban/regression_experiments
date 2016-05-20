#include "regression_experiments/solver_factory.h"

#include "regression_experiments/gp_forest_solver.h"
#include "regression_experiments/gp_solver.h"

namespace regression_experiments
{

SolverFactory::SolverFactory()
{
  registerBuilder("gp_forest",
                  [](TiXmlNode * node)
                  {
                    (void)node;
                    return new GPForestSolver();
                  });
  registerBuilder("gp",
                  [](TiXmlNode * node)
                  {
                    (void)node;
                    return new GPSolver();
                  });
}

}
