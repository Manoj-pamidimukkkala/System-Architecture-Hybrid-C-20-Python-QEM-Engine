#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "qem_engine.hpp"

namespace py = pybind11;

PYBIND11_MODULE(qem_engine_cpp, m) {
    m.doc() = "Low-latency C++20 Zero-Noise Extrapolation Engine";

    py::enum_<qem::ExtrapolationModel>(m, "ExtrapolationModel")
        .value("Linear", qem::ExtrapolationModel::Linear)
        .value("Exponential", qem::ExtrapolationModel::Exponential)
        .export_values();

    py::class_<qem::MitigationResult>(m, "MitigationResult")
        .def_readonly("mitigated_value", &qem::MitigationResult::mitigated_value)
        .def_readonly("unmitigated_value", &qem::MitigationResult::unmitigated_value)
        .def_readonly("noise_factors", &qem::MitigationResult::noise_factors)
        .def_readonly("expectation_values", &qem::MitigationResult::expectation_values);

    py::class_<qem::QEMEngine>(m, "QEMEngine")
        .def(py::init<>())
        .def("fold_openqasm_gates", &qem::QEMEngine::fold_openqasm_gates)
        .def("compute_expectation", [](qem::QEMEngine& self, std::vector<double> counts, std::vector<int> eigenvalues) {
            return self.compute_expectation(counts, eigenvalues);
        })
        .def("extrapolate_zero_noise", [](qem::QEMEngine& self, std::vector<double> scale_factors, std::vector<double> expectations, qem::ExtrapolationModel model) {
            return self.extrapolate_zero_noise(scale_factors, expectations, model);
        });
}
