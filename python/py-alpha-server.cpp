#include "util.hpp"
#include <alpha/alpha-server.h>
#include <pybind11/pybind11.h>
#include <stdexcept>

namespace py = pybind11;
using namespace pybind11::literals;

static std::string dict_to_json(const py::dict& d) {
    py::module json = py::module::import("json");
    py::object dumps = json.attr("dumps");
    py::object res = dumps(d);
    return res.cast<std::string>();
}


PYBIND11_MODULE(_pyalpha_server, m) {
    m.doc() = "Python binding for the Alpha server library";

    using AlphaProvider = AlphaWrapper<alpha_provider_t, alpha_provider_destroy>;

    py::class_<AlphaProvider, std::shared_ptr<AlphaProvider>>(m, "Provider")
        .def(py::init([](py::object pyMargoEngine,
                         uint16_t provider_id,
                         const py::dict& config) {
            py::capsule mid = pyMargoEngine.attr("get_internal_mid")();
            auto config_str = dict_to_json(config);
            alpha_provider_t provider = ALPHA_PROVIDER_NULL;
            alpha_return_t ret = alpha_provider_register(
                mid, provider_id, config_str.c_str(), NULL, &provider);
            if(ret != ALPHA_SUCCESS) {
                throw std::runtime_error{
                    std::string{"Could not register Alpha provider. alpha_provider_register returned "}
                    + std::to_string(ret)
                };
            }
            return std::make_shared<AlphaProvider>(provider);
        }),
            R"(
            Provider constructor.

            Parameters
            ----------

            engine (pymargo.Engine): PyMargo Engine to use.
            provider_id (int): Provider ID.
            config (dict): Configuration of the provider.

            Returns
            -------

            A alpha.Provider instance.
            )",
            "engine"_a,
            "provider_id"_a,
            "config"_a,
            py::keep_alive<1, 2>())
        ;
}
