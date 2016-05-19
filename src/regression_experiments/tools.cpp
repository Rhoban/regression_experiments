#include "regression_experiments/tools.h"

namespace regression_experiments
{

Eigen::MatrixXd discretizeSpace(const Eigen::MatrixXd & limits,
                                const std::vector<int> & samples_by_dim)
{
  // Checking consistency
  if (limits.rows() != (int)samples_by_dim.size()) {
    throw std::runtime_error("discretizeSpace: inconsistency: limits.rows() != samples_by_dim");
  }
  // Preparing preliminary data
  int total_points = 1;
  std::vector<int> intervals(samples_by_dim.size());
  Eigen::VectorXd delta = limits.col(1) - limits.col(0);
  for (size_t dim = 0; dim < samples_by_dim.size(); dim++)
  {
    intervals[dim] = total_points;
    total_points *= samples_by_dim[dim];
  }
  // Preparing points
  Eigen::MatrixXd points(limits.rows(), total_points);
  for (int dim = 0; dim < limits.rows(); dim++) {
    for (int point = 0; point < total_points; point++) {
      // Determining index inside given dimension
      int dim_index = point / intervals[dim];
      dim_index = dim_index % samples_by_dim[dim];
      // Computing value
      double value = (limits(dim, 1) + limits(dim, 0)) / 2;// default value
      if (samples_by_dim[dim] != 1) {
        double step_size = delta(dim) / (samples_by_dim[dim] - 1);
        value = limits(dim, 0) + step_size * dim_index;
      }
      points(dim, point) = value;
    }
  }
  return points;
}

}
