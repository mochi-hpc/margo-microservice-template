/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __ALPHA_CLIENT_H
#define __ALPHA_CLIENT_H

#include <margo.h>
#include <alpha/alpha-common.h>

#ifdef __cplusplus
extern "C" {
#endif

// TUTORIAL
// ********
//
// The client library is composed of the alpha_lient_t and alpha_resource_handle_t objects.
// The alpha_client_t is an object that registers RPCs with the Margo instance. It can then
// be used create alpha_resource_handle_t instances using alpha_resource_handle_create.
// The alpha_client_t will keep a count of the created resource handles, and will print
// a warning if it is destroyed before all the resource handles have been destroyed.

typedef struct alpha_client* alpha_client_t;
#define ALPHA_CLIENT_NULL ((alpha_client_t)NULL)

/**
 * @brief Creates a ALPHA client.
 *
 * @param[in] mid Margo instance
 * @param[out] client ALPHA client
 *
 * @return ALPHA_SUCCESS or error code defined in alpha-common.h
 */
alpha_return_t alpha_client_init(margo_instance_id mid, alpha_client_t* client);

/**
 * @brief Finalizes a ALPHA client.
 *
 * @param[in] client ALPHA client to finalize
 *
 * @return ALPHA_SUCCESS or error code defined in alpha-common.h
 */
alpha_return_t alpha_client_finalize(alpha_client_t client);

#ifdef __cplusplus
}
#endif

#endif
