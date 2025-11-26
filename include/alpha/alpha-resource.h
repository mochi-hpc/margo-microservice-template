/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __ALPHA_RESOURCE_H
#define __ALPHA_RESOURCE_H

#include <margo.h>
#include <alpha/alpha-common.h>
#include <alpha/alpha-client.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct alpha_resource_handle *alpha_resource_handle_t;
#define ALPHA_RESOURCE_HANDLE_NULL ((alpha_resource_handle_t)NULL)

/**
 * @brief Creates a ALPHA resource handle.
 *
 * @param[in] client ALPHA client responsible for the resource handle
 * @param[in] addr Mercury address of the provider
 * @param[in] provider_id id of the provider
 * @param[in] check If true, will send an RPC to check that the provider exists
 * @param[out] handle resource handle
 *
 * @return ALPHA_SUCCESS or error code defined in alpha-common.h
 */
alpha_return_t alpha_resource_handle_create(
        alpha_client_t client,
        hg_addr_t addr,
        uint16_t provider_id,
        bool check,
        alpha_resource_handle_t* handle);

/**
 * @brief Increments the reference counter of a resource handle.
 *
 * @param handle resource handle
 *
 * @return ALPHA_SUCCESS or error code defined in alpha-common.h
 */
alpha_return_t alpha_resource_handle_ref_incr(
        alpha_resource_handle_t handle);

/**
 * @brief Releases the resource handle. This will decrement the
 * reference counter, and free the resource handle if the reference
 * counter reaches 0.
 *
 * @param[in] handle resource handle to release.
 *
 * @return ALPHA_SUCCESS or error code defined in alpha-common.h
 */
alpha_return_t alpha_resource_handle_release(alpha_resource_handle_t handle);

/**
 * @brief Makes the target ALPHA resource compute the sum of the
 * two numbers and return the result.
 *
 * @param[in] handle resource handle.
 * @param[in] x first number.
 * @param[in] y second number.
 * @param[out] result resulting value.
 *
 * @return ALPHA_SUCCESS or error code defined in alpha-common.h
 */
alpha_return_t alpha_compute_sum(
        alpha_resource_handle_t handle,
        int32_t x,
        int32_t y,
        int32_t* result);

/**
 * @brief Same as alpha_compute_sum but allows passing a timeout.
 *
 * @param[in] handle resource handle.
 * @param[in] x first number.
 * @param[in] y second number.
 * @param[in] timeout_ms timeout (in milliseconds).
 * @param[out] result resulting value.
 *
 * @return ALPHA_SUCCESS or error code defined in alpha-common.h
 */
alpha_return_t alpha_compute_sum_timed(
        alpha_resource_handle_t handle,
        int32_t x,
        int32_t y,
        double timeout_ms,
        int32_t* result);

/**
 * @brief Makes the target ALPHA resource compute the pair-wise sum of the
 * numbers in the x and y arrays and set the results in the result array.
 *
 * @param[in] handle resource handle.
 * @param[in] x first array of numbers.
 * @param[in] y second array of numbers.
 * @param[out] result resulting values.
 *
 * @return ALPHA_SUCCESS or error code defined in alpha-common.h
 */
alpha_return_t alpha_compute_sum_multi(
        alpha_resource_handle_t handle,
        size_t count,
        const int32_t* x,
        const int32_t* y,
        int32_t* result);

/**
 * @brief Low-level version of alpha_compute_sum_multi based on user-provided
 * bulk handles.
 *
 * @param[in] handle resource handle.
 * @param[in] x bulk location of the first array of numbers.
 * @param[in] y bulk location of the second array of numbers.
 * @param[out] result bulk location of the resulting values.
 *
 * @return ALPHA_SUCCESS or error code defined in alpha-common.h
 */
alpha_return_t alpha_compute_sum_bulk(
        alpha_resource_handle_t handle,
        size_t count,
        const alpha_bulk_location_t* x,
        const alpha_bulk_location_t* y,
        const alpha_bulk_location_t* result);

#ifdef __cplusplus
}
#endif

#endif
