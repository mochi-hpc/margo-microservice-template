/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include "types.h"
#include "client.h"
#include "alpha/alpha-client.h"

// TUTORIAL
// ********
//
// This file contains the implementation of the client library.
// Several example RPCs are provided which pass their data as RPC arguments
// or using RDMA.
//
// Note that contrary the the Thallium microservice template, which provides
// a non-blocking API (with functions returning Future objects that must be waited
// on), this template provide blocking functions. Non-blocking functions can crafted
// using Margo functions such as margo_provider_iforward (instead of margo_provider_forward),
// margo_provider_iforward_timed (instead of margo_provider_forward_timed), and so on.
// These functions would return a margo_request object that the caller needs to wait
// on using margo_wait. Making a non-blocking API in C is arguably more complex since
// the margo_request needs to be awaited before functions such as margo_get_output,
// margo_free_output, and margo_destroy can be called to complete the RPC flow.
// One way to make a non-blocking API would be to encapsulate a margo_request along
// with other data (such as the hg_handle_t of the on-going RPC) in an opaque alpha_request_t
// object, and provide an alpha_request_wait function to complete the operation and cleanup.

alpha_return_t alpha_client_init(margo_instance_id mid, alpha_client_t* client)
{
    alpha_client_t c = (alpha_client_t)calloc(1, sizeof(*c));
    if(!c) return ALPHA_ERR_ALLOCATION;

    c->mid = mid;

    hg_bool_t flag;
    hg_id_t id;
    margo_registered_name(mid, "alpha_sum", &id, &flag);

    if(flag == HG_TRUE) {
        margo_registered_name(mid, "alpha_sum", &c->sum_id, &flag);
        margo_registered_name(mid, "alpha_sum_multi", &c->sum_multi_id, &flag);
    } else {
        c->sum_id = MARGO_REGISTER(mid, "alpha_sum", sum_in_t, sum_out_t, NULL);
        c->sum_multi_id = MARGO_REGISTER(mid, "alpha_sum_multi", sum_multi_in_t, sum_multi_out_t, NULL);
    }

    *client = c;
    return ALPHA_SUCCESS;
}

alpha_return_t alpha_client_finalize(alpha_client_t client)
{
    if(client->num_resource_handles != 0) {
        margo_warning(client->mid,
            "%ld resource handles not released when alpha_client_finalize was called",
            client->num_resource_handles);
    }
    free(client);
    return ALPHA_SUCCESS;
}

alpha_return_t alpha_resource_handle_create(
        alpha_client_t client,
        hg_addr_t addr,
        uint16_t provider_id,
        bool check,
        alpha_resource_handle_t* handle)
{
    if(client == ALPHA_CLIENT_NULL)
        return ALPHA_ERR_INVALID_ARGS;

    hg_return_t ret;

    if(check) {
        char buffer[sizeof("alpha")];
        size_t bufsize = sizeof("alpha");
        ret = margo_provider_get_identity(client->mid, addr, provider_id, buffer, &bufsize);
        if(ret != HG_SUCCESS || strcmp("alpha", buffer) != 0)
            return ALPHA_ERR_INVALID_PROVIDER;
    }

    alpha_resource_handle_t rh =
        (alpha_resource_handle_t)calloc(1, sizeof(*rh));

    if(!rh) return ALPHA_ERR_ALLOCATION;

    ret = margo_addr_dup(client->mid, addr, &(rh->addr));
    if(ret != HG_SUCCESS) {
        free(rh);
        return ALPHA_ERR_FROM_MERCURY;
    }

    rh->client      = client;
    rh->provider_id = provider_id;
    rh->refcount    = 1;

    client->num_resource_handles += 1;

    *handle = rh;
    return ALPHA_SUCCESS;
}

alpha_return_t alpha_resource_handle_ref_incr(
        alpha_resource_handle_t handle)
{
    if(handle == ALPHA_RESOURCE_HANDLE_NULL)
        return ALPHA_ERR_INVALID_ARGS;
    handle->refcount += 1;
    return ALPHA_SUCCESS;
}

alpha_return_t alpha_resource_handle_release(alpha_resource_handle_t handle)
{
    if(handle == ALPHA_RESOURCE_HANDLE_NULL)
        return ALPHA_ERR_INVALID_ARGS;
    handle->refcount -= 1;
    if(handle->refcount == 0) {
        margo_addr_free(handle->client->mid, handle->addr);
        handle->client->num_resource_handles -= 1;
        free(handle);
    }
    return ALPHA_SUCCESS;
}

alpha_return_t alpha_compute_sum(
        alpha_resource_handle_t handle,
        int32_t x,
        int32_t y,
        int32_t* result)
{
    hg_handle_t   h;
    sum_in_t     in;
    sum_out_t   out;
    hg_return_t hret;
    alpha_return_t ret;

    in.x = x;
    in.y = y;

    hret = margo_create(handle->client->mid, handle->addr, handle->client->sum_id, &h);
    if(hret != HG_SUCCESS)
        return ALPHA_ERR_FROM_MERCURY;

    hret = margo_provider_forward(handle->provider_id, h, &in);
    if(hret != HG_SUCCESS) {
        ret = ALPHA_ERR_FROM_MERCURY;
        goto finish;
    }

    hret = margo_get_output(h, &out);
    if(hret != HG_SUCCESS) {
        ret = ALPHA_ERR_FROM_MERCURY;
        goto finish;
    }

    ret = out.ret;
    if(ret == ALPHA_SUCCESS)
        *result = out.result;

    margo_free_output(h, &out);

finish:
    margo_destroy(h);
    return ret;
}

alpha_return_t alpha_compute_sum_timed(
        alpha_resource_handle_t handle,
        int32_t x,
        int32_t y,
        double timeout_ms,
        int32_t* result)
{
    hg_handle_t   h;
    sum_in_t     in;
    sum_out_t   out;
    hg_return_t hret;
    alpha_return_t ret;

    in.x = x;
    in.y = y;

    hret = margo_create(handle->client->mid, handle->addr, handle->client->sum_id, &h);
    if(hret != HG_SUCCESS)
        return ALPHA_ERR_FROM_MERCURY;

    hret = margo_provider_forward_timed(handle->provider_id, h, &in, timeout_ms);

    if(hret == HG_TIMEOUT) {
        ret = ALPHA_TIMEOUT;
        goto finish;
    }

    if(hret != HG_SUCCESS) {
        ret = ALPHA_ERR_FROM_MERCURY;
        goto finish;
    }

    hret = margo_get_output(h, &out);

    if(hret != HG_SUCCESS) {
        ret = ALPHA_ERR_FROM_MERCURY;
        goto finish;
    }

    ret = out.ret;
    if(ret == ALPHA_SUCCESS)
        *result = out.result;

    margo_free_output(h, &out);

finish:
    margo_destroy(h);
    return ret;
}

alpha_return_t alpha_compute_sum_multi(
        alpha_resource_handle_t handle,
        size_t count,
        const int32_t* x,
        const int32_t* y,
        int32_t* result)
{
    alpha_return_t ret    = ALPHA_SUCCESS;
    hg_return_t hret      = HG_SUCCESS;
    hg_addr_t self_addr   = HG_ADDR_NULL;
    hg_bulk_t input_bulk  = HG_BULK_NULL;
    hg_bulk_t output_bulk = HG_BULK_NULL;
    margo_instance_id mid = handle->client->mid;

    void* input_ptrs[] = {(void*)x, (void*)y};
    hg_size_t input_sizes[] = {count*sizeof(int32_t), count*sizeof(int32_t)};
    void* output_ptrs[] = {(void*)result};
    hg_size_t output_sizes[] = {count*sizeof(int32_t)};

    char address[256];
    hg_size_t address_size = 256;
    hret = margo_addr_self(mid, &self_addr);
    if(hret != HG_SUCCESS) {
        ret = ALPHA_ERR_FROM_MERCURY;
        goto finish;
    }
    margo_addr_to_string(mid, address, &address_size, self_addr);
    if(hret != HG_SUCCESS) {
        ret = ALPHA_ERR_FROM_MERCURY;
        goto finish;
    }

    hret = margo_bulk_create(mid, 2, input_ptrs, input_sizes, HG_BULK_READ_ONLY, &input_bulk);
    if(hret != HG_SUCCESS) {
        ret = ALPHA_ERR_FROM_MERCURY;
        goto finish;
    }
    hret = margo_bulk_create(mid, 1, output_ptrs, output_sizes, HG_BULK_WRITE_ONLY, &output_bulk);
    if(hret != HG_SUCCESS) {
        ret = ALPHA_ERR_FROM_MERCURY;
        goto finish;
    }

    alpha_bulk_location_t x_bl = {
        .bulk = input_bulk,
        .address = address,
        .offset = 0,
        .size = count*sizeof(int32_t)
    };
    alpha_bulk_location_t y_bl = {
        .bulk = input_bulk,
        .address = address,
        .offset = count*sizeof(int32_t),
        .size = count*sizeof(int32_t)
    };
    alpha_bulk_location_t result_bl = {
        .bulk = output_bulk,
        .address = address,
        .offset = 0,
        .size = count*sizeof(int32_t)
    };

    ret = alpha_compute_sum_bulk(handle, count, &x_bl, &y_bl, &result_bl);

finish:

    margo_addr_free(mid, self_addr);
    margo_bulk_free(input_bulk);
    margo_bulk_free(output_bulk);

    return ret;
}

alpha_return_t alpha_compute_sum_bulk(
          alpha_resource_handle_t handle,
          size_t count,
          const alpha_bulk_location_t* x,
          const alpha_bulk_location_t* y,
          const alpha_bulk_location_t* result)
{

    hg_handle_t     h;
    sum_multi_in_t  in = {
        .count  = count,
        .x      = *x,
        .y      = *y,
        .result = *result
    };
    sum_multi_out_t out;
    hg_return_t hret;
    alpha_return_t ret;

    hret = margo_create(handle->client->mid, handle->addr, handle->client->sum_multi_id, &h);
    if(hret != HG_SUCCESS)
        return ALPHA_ERR_FROM_MERCURY;

    hret = margo_provider_forward(handle->provider_id, h, &in);
    if(hret != HG_SUCCESS) {
        ret = ALPHA_ERR_FROM_MERCURY;
        goto finish;
    }

    hret = margo_get_output(h, &out);
    if(hret != HG_SUCCESS) {
        ret = ALPHA_ERR_FROM_MERCURY;
        goto finish;
    }

    ret = out.ret;

    margo_free_output(h, &out);

finish:
    margo_destroy(h);
    return ret;
}
