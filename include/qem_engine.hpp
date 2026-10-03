#pragma once

#include <vector>
#include <string>
#include <span>
#include <numeric>
#include <cmath>
#include <stdexcept>

namespace qem {

enum class ExtrapolationModel {
    Linear,
    Exponential,
    PolynomialGrad
};

struct MitigationResult {
    double mitigated_value;
    double unmitigated_value;
    std::vector<double> noise_factors;
    std::vector<double> expectation_values;
};

class QEMEngine {
public:
    QEMEngine() = default;

    // Apply digital gate folding logic to OpenQASM 3.0 representation
    std::string fold_openqasm_gates(const std::string& qasm_str, double scale_factor);

    // Compute expectation value from bitstring shot distribution
    double compute_expectation(std::span<const double> counts, std::span<const int> eigenvalues);

    // Extrapolate expectations to zero-noise limit
    MitigationResult extrapolate_zero_noise(
        std::span<const double> scale_factors,
        std::span<const double> expectations,
        ExtrapolationModel model
    );
};

} // namespace qem
