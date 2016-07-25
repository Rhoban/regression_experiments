#include "regression_experiments/solver_factory.h"

#include "regression_experiments/gp_forest_solver.h"
#include "regression_experiments/gp_solver.h"
#include "regression_experiments/pwc_forest_solver.h"
#include "regression_experiments/pwl_forest_solver.h"

namespace regression_experiments
{

SolverFactory::SolverFactory()
{
  registerBuilder("gp_forest" ,[](){return std::unique_ptr<Solver>(new GPForestSolver );});
  registerBuilder("pwc_forest",[](){return std::unique_ptr<Solver>(new PWCForestSolver);});
  registerBuilder("pwl_forest",[](){return std::unique_ptr<Solver>(new PWLForestSolver);});
  registerBuilder("gp"        ,[](){return std::unique_ptr<Solver>(new GPSolver       );});
}

}
