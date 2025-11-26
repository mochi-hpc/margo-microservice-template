
#include <alpha/alpha-client.h>
#include "util.hpp"
#include <alpha/alpha-resource.h>
#include <pybind11/pybind11.h>
#include <stdexcept>

namespace py = pybind11;
using namespace pybind11::literals;

struct AlphaException : public std::runtime_error {

    template<typename ... Args>
    AlphaException(Args&&... args)
    : std::runtime_error{std::forward<Args>(args)...} {}
};


PYBIND11_MODULE(_pyalpha_client, m) {
    m.doc() = "Python binding for the Alpha client library";

    using AlphaClient = AlphaWrapper<alpha_client_t, alpha_client_finalize>;
    using AlphaResourceHandle = AlphaWrapper<alpha_resource_handle_t, alpha_resource_handle_release>;

    py::register_exception<AlphaException>(m, "Exception", PyExc_RuntimeError);

    py::class_<AlphaClient, std::shared_ptr<AlphaClient>>(m, "Client")
        .def(py::init([](const py::object& pyMargoEngine) {
            py::capsule mid = pyMargoEngine.attr("get_internal_mid")();
            alpha_client_t client = ALPHA_CLIENT_NULL;
            alpha_return_t ret = alpha_client_init(mid, &client);
            if(ret != ALPHA_SUCCESS) {
                throw AlphaException{
                    std::string{"Could not initialize alpha_client_t, "
                                "alpha_client_init returned "}
                    + std::to_string(ret)
                };
            }
            return std::make_shared<AlphaClient>(client);
        }), py::keep_alive<1, 2>(),
            R"(
            Client constructor.

            Parameters
            ----------

            engine (pymargo.Engine): PyMargo Engine to use.

            Returns
            -------

            A alpha.Client instance.
            )",
            "engine"_a)
        .def("make_resource_handle",
             [](const AlphaClient& client, const py::object& pyAddr, uint16_t provider_id, bool check) {
                py::capsule addr = pyAddr.attr("get_internal_hg_addr")();
                alpha_resource_handle_t handle = ALPHA_RESOURCE_HANDLE_NULL;
                alpha_return_t ret = alpha_resource_handle_create(
                    client, addr, provider_id, check, &handle);
                if(ret != ALPHA_SUCCESS) {
                    throw AlphaException{
                        std::string{"Could not create alpha_resource_handle_t, "
                                    "alpha_resource_handle_create returned "}
                        + std::to_string(ret)
                    };
                }
                return std::make_shared<AlphaResourceHandle>(handle);
             },
            R"(
            Create a ResourceHandle.

            Parameters
            ----------

            address (str): Address of the process owning the resource.
            provider_id (int): Provider ID of the resource.
            check (Optional[bool]): Check that the provider exists.

            Returns
            -------

            A alpha.ResourceHandle instance.
            )",
            "address"_a, "provider_id"_a, "check"_a=false)
        ;

    py::class_<AlphaResourceHandle, std::shared_ptr<AlphaResourceHandle>>(m, "ResourceHandle")
        .def("compute_sum",
            [](const AlphaResourceHandle& handle, int x, int y) {
                int32_t result = 0;
                alpha_return_t ret = alpha_compute_sum(handle, x, y, &result);
                if(ret != ALPHA_SUCCESS) {
                    throw AlphaException{
                        std::string{"alpha_compute_sum failed with error code "}
                        + std::to_string(ret)
                    };
                }
                return result;
            },
            R"(
            "Compute the sum of two numbers.

            Parameters
            ----------

            x (int): First number.
            y (int): Second number.

            Returns
            -------

            The result of the sum.
            )", "x"_a, "y"_a)
        .def("compute_sum_with_timeout",
             [](const AlphaResourceHandle& handle, int32_t x, int32_t y, double timeout_ms) -> py::object {
                int32_t result = 0;
                alpha_return_t ret = alpha_compute_sum_timed(handle, x, y, timeout_ms, &result);
                if(ret == ALPHA_TIMEOUT) return py::none();
                if(ret != ALPHA_SUCCESS) {
                    throw AlphaException{
                        std::string{"alpha_compute_sum_timed failed with error code "}
                        + std::to_string(ret)
                    };
                }
                return py::cast(result);
             },
            R"(
            "Compute the sum of two numbers.

            Parameters
            ----------

            x (int): First number.
            y (int): Second number.
            timeout (float): Timeout in milliseconds.

            Returns
            -------

            The result of the sum, or None if the operation timed out.

            )", "x"_a, "y"_a, "timeout"_a)
        .def("compute_sums",
             [](const AlphaResourceHandle& handle,
                const py::buffer& x, const py::buffer& y,
                const py::buffer& r) {

                    auto check_array_valid = [](const py::buffer& buf) {
                        auto info = buf.request();
                        if(info.ndim != 1) throw AlphaException{"Invalid array dimenion (should be 1)"};
                        if(info.itemsize != sizeof(int32_t)) throw AlphaException{"Invalid itemsize"};
                        if(info.format != py::format_descriptor<int32_t>::format())
                            throw AlphaException{"Invalid array content type"};
                        return info;
                    };

                    py::buffer_info x_info = check_array_valid(x);
                    py::buffer_info y_info = check_array_valid(y);
                    py::buffer_info r_info = check_array_valid(r);

                    if(x_info.size != y_info.size || y_info.size != r_info.size) {
                        throw AlphaException{"Buffers should be the same size"};
                    }

                    alpha_return_t ret = alpha_compute_sum_multi(handle,
                            x_info.size,
                            (const int32_t*)x_info.ptr,
                            (const int32_t*)y_info.ptr,
                            (int32_t*)r_info.ptr);
                    if(ret != ALPHA_SUCCESS) {
                        throw AlphaException{
                            std::string{"alpha_compute_sum_multi failed with error code "}
                            + std::to_string(ret)
                        };
                    }
                },
            R"(
            "Compute the sum of numbers in two arrays.

            Parameters
            ----------

            x (array[int]): First array of numbers.
            y (array[int]): Second array of number.
            r (array[int]): Wrray in which to place the results.

            Returns
            -------

            None
            )", "x"_a, "y"_a, "r"_a)
    ;
}
