#include "regression_experiments/benchmark_function_factory.h"
#include "regression_experiments/solver_factory.h"
#include "regression_experiments/tools.h"

#include "rosban_regression_forests/algorithms/extra_trees.h"
#include "rosban_regression_forests/approximations/gp_approximation.h"

#include "rosban_random/tools.h"

#include <fstream>
#include <map>

using namespace regression_experiments;

using rosban_gp::GaussianProcess;

class BenchmarkConfig : public rosban_utils::Serializable
{
public:
  std::map<std::string, std::shared_ptr<Solver>> solvers;
  //TODO add other informations
  

  std::string class_name() const override
    {
      return "benchmark_config";
    }

  void to_xml(std::ostream &out) const override
    {
      throw std::logic_error("BenchmarkConfig::to_xml: Not implemented");
    }

  void from_xml(TiXmlNode *node)
    {
      // Read solvers
      SolverFactory sf;
      std::function<std::shared_ptr<Solver>(TiXmlNode*)> value_builder;
      value_builder = [&sf](TiXmlNode * node) { return std::shared_ptr<Solver>(sf.build(node)); };
      solvers = rosban_utils::xml_tools::read_map(node, "solvers", value_builder);
    }
};

int main()
{
  BenchmarkConfig conf;
  conf.load_file();

  // Setting benchmark properties
  int nb_prediction_points = 200;
  int nb_trials_per_type = 10;
  // Maximal learning time in [ms]
  double max_learning_time = std::pow(10,5);
  // Maximal prediction time in [ms] (10ms per predicted point)
  double max_prediction_time = nb_prediction_points * 5;

  std::vector<int> nb_samples_vec;
  for (int i = 1; i <= 8; i++) {
    nb_samples_vec.push_back(25 * std::pow(2,i-1));
  }
  std::vector<std::string> function_names =
    {
//      "abs",
//      "sin",
//      "binary",
//      "ternary",
//      "deterministic_abs",
//      "deterministic_sin",
//      "deterministic_binary",
//      "deterministic_ternary",
      "sinus_2dim",
//      "binary_2dim",
      "sinus_3dim"
//      "binary_3dim"
    };
//  std::map<std::string, std::shared_ptr<Solver>> solvers =
//    {
//      {"pwc_forest"    , std::shared_ptr<Solver>(new PWCForestSolver()                         )},
//      {"pwl_forest"    , std::shared_ptr<Solver>(new PWLForestSolver()                         )},
//      {"gp_forest_sqrt", std::shared_ptr<Solver>(new GPForestSolver(GPForestSolver::Type::SQRT))},
//      {"gp_forest_curt", std::shared_ptr<Solver>(new GPForestSolver(GPForestSolver::Type::CURT))},
//      {"gp_forest_log2", std::shared_ptr<Solver>(new GPForestSolver(GPForestSolver::Type::LOG2))},
//      {"gp"            , std::shared_ptr<Solver>(new GPSolver()                                )}
//    };

  // replace for debug
//  function_names = {"sinus_2dim",
//                    "binary_2dim",
//                    "sinus_3dim",
//                    "binary_3dim"};
//  function_names = {"sin", "sinus_2dim"};

  // Creating random engine
  auto engine = rosban_random::getRandomEngine();

  // Open and write header for regression benchmark
  std::ofstream out;
  out.open("benchmark_regression.csv");
  out << "function_name,"
      << "solver,"
      << "nb_samples,"
      << "smse,"
      << "learning_time,"
      << "prediction_time"
      << std::endl;
  
  // Open and write header for max benchmark
  std::ofstream max_out;
  max_out.open("benchmark_max.csv");
  max_out << "function_name,"
          << "solver,"
          << "nb_samples,"
          << "squared_loss,"
          << "squared_error,"
          << "learning_time,"
          << "compute_max_time"
          << std::endl;

  for (const std::string & function_name : function_names) {
    for (auto & solver_entry : conf.solvers) {
      const std::string & solver_name = solver_entry.first;
      std::shared_ptr<Solver> solver = solver_entry.second;
      for (int nb_samples : nb_samples_vec) {
        std::cout << "Fitting '" << function_name << "' with '" << solver_name
                  << "' (" << nb_samples << " samples)" << std::endl;
        double total_prediction_time = 0;
        double total_learning_time   = 0;
        for (int trial = 1; trial <= nb_trials_per_type; trial++) {
          std::cerr << "\ttrial: " << trial << "/" << nb_trials_per_type << std::endl;
          double smse, learning_time, prediction_time;
          double arg_max_loss, max_prediction_error, compute_max_time;
          runBenchmark(function_name,
                       nb_samples,
                       solver,
                       nb_prediction_points,
                       smse,
                       learning_time,
                       prediction_time,
                       arg_max_loss,
                       max_prediction_error,
                       compute_max_time,
                       &engine);
          double loss2, error2;
          loss2 = arg_max_loss * arg_max_loss;
          error2 = max_prediction_error * max_prediction_error;
          out << function_name   << ","
              << solver_name     << ","
              << nb_samples      << ","
              << smse            << ","
              << learning_time   << ","
              << prediction_time << std::endl;
          max_out << function_name    << ","
                  << solver_name      << ","
                  << nb_samples       << ","
                  << loss2            << ","
                  << error2           << ","
                  << learning_time    << ","
                  << compute_max_time << std::endl;
          // Cumulating time
          total_learning_time   += learning_time;
          total_prediction_time += prediction_time;
        }
        double avg_learning_time    = total_learning_time   / nb_trials_per_type;
        double avg_prediction_time  = total_prediction_time / nb_trials_per_type;
        // Do not compute with higher number of samples if time is already above the threshold
        if (avg_learning_time   > max_learning_time  ) break;
        if (avg_prediction_time > max_prediction_time) break;
      }
    }
  }
}
