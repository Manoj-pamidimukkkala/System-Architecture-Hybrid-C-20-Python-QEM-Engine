#include "qem_engine.hpp"
#include <sstream>
#include <algorithm>

namespace qem {

std::string QEMEngine::fold_openqasm_gates(const std::string& qasm_str, double scale_factor) {
    int total_folds = static_cast<int>(std::round((scale_factor - 1.0) / 2.0));
    if (total_folds <= 0) return qasm_str;

    std::stringstream ss(qasm_str);
    std::string line;
    std::string result;

    while (std::getline(ss, line)) {
        result += line + "\n";
        // Apply folding to single/two-qubit gate instructions
        if (line.find("cx") != std::string::npos || line.find("rz") != std::string::npos || line.find("x ") != std::string::npos) {
            for (int i = 0; i < total_folds; ++i) {
                result += "// Fold Pair " + std::to_string(i + 1) + "\n";
                result += line + " // Inverse dagger\n";
                result += line + " // Forward fold\n";
            }
        }
    }
    return result;
}

double QEMEngine::compute_expectation(std::span<const double> counts, std::span<const int> eigenvalues) {
    if (counts.size() != eigenvalues.size()) {
        throw std::invalid_argument("Counts and eigenvalues dimensions must match.");
    }

    double total_shots = std::accumulate(counts.begin(), counts.end(), 0.0);
    if (total_shots == 0.0) return 0.0;

    double weighted_sum = 0.0;
    for (size_t i = 0; i < counts.size(); ++i) {
        weighted_sum += counts[i] * eigenvalues[i];
    }

    return weighted_sum / total_shots;
}

MitigationResult QEMEngine::extrapolate_zero_noise(
    std::span<const double> scale_factors,
    std::span<const double> expectations,
    ExtrapolationModel model
) {
    size_t n = scale_factors.size();
    if (n < 2 || n != expectations.size()) {
        throw std::invalid_argument("Extrapolation requires at least 2 scaling data points.");
    }

    double mitigated_val = 0.0;

    if (model == ExtrapolationModel::Linear) {
        double sum_x = 0, sum_y = 0, sum_xy = 0, sum_xx = 0;
        for (size_t i = 0; i < n; ++i) {
            sum_x += scale_factors[i];
            sum_y += expectations[i];
            sum_xy += scale_factors[i] * expectations[i];
            sum_xx += scale_factors[i] * scale_factors[i];
        }
        double slope = (n * sum_xy - sum_x * sum_y) / (n * sum_xx - sum_x * sum_x);
        double intercept = (sum_y - slope * sum_x) / n;
        mitigated_val = intercept; // Zero Noise Limit (x = 0)
    } else if (model == ExtrapolationModel::Exponential) {
        // Fit ln(y) = ln(A) + B*x
        double sum_x = 0, sum_log_y = 0, sum_x_log_y = 0, sum_xx = 0;
        for (size_t i = 0; i < n; ++i) {
            double log_y = std::log(std::max(expectations[i], 1e-8));
            sum_x += scale_factors[i];
            sum_log_y += log_y;
            sum_x_log_y += scale_factors[i] * log_y;
            sum_xx += scale_factors[i] * scale_factors[i];
        }
        double slope = (n * sum_x_log_y - sum_x * sum_log_y) / (n * sum_xx - sum_x * sum_x);
        double intercept = (sum_log_y - slope * sum_x) / n;
        mitigated_val = std::exp(intercept);
    }

    return MitigationResult{
        .mitigated_value = mitigated_val,
        .unmitigated_value = expectations[0],
        .noise_factors = std::vector<double>(scale_factors.begin(), scale_factors.end()),
        .expectation_values = std::vector<double>(expectations.begin(), expectations.end())
    };
}

} // namespace qem
