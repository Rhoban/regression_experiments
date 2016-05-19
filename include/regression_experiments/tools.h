#pragma once

#include <Eigen/Core>

namespace regression_experiments
{

/// Return a matrix containing product(samples_by_dim) columns and limits.rows() rows
/// Each column is a different sample
Eigen::MatrixXd discretizeSpace(const Eigen::MatrixXd & limits,
                                const std::vector<int> & samples_by_dim);

}
