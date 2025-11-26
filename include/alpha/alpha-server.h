/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __ALPHA_SERVER_H
#define __ALPHA_SERVER_H

#include <alpha/alpha-common.h>
#include <alpha/alpha-backend.h>
#include <margo.h>

// TUTORIAL
// ********
//
// A alpha_provider_t object is an object that (1) holds a resource instance and
// (2) receives RPCs to be executed against this instance. It is initialized
// with a Margo instance ID, a provider ID, a configuration string, which
// should be JSON-formatted, and some other optional arguments such as an Argobots
// pool in which its RPCs will land.
// In more complex components, it may require other dependencies such as handles
// to other components, other Argobots pools, etc.

#ifdef __cplusplus
extern "C" {
#endif

#define ALPHA_ABT_POOL_DEFAULT ABT_POOL_NULL

typedef struct alpha_provider* alpha_provider_t;
#define ALPHA_PROVIDER_NULL ((alpha_provider_t)NULL)
#define ALPHA_PROVIDER_IGNORE ((alpha_provider_t*)NULL)

struct alpha_provider_args {
    ABT_pool            pool;    // Pool used to run RPCs
    alpha_backend_impl* backend; // Type of backend, will take priority over the "type" field in config
    // ...
};

#define ALPHA_PROVIDER_ARGS_INIT { \
    /* .pool = */ ABT_POOL_NULL, \
    /* .backend = */ NULL \
}

/**
 * @brief Creates a new ALPHA provider. If ALPHA_PROVIDER_IGNORE
 * is passed as last argument, the provider will be automatically
 * destroyed when calling margo_finalize.
 *
 * @param[in] mid Margo instance
 * @param[in] provider_id provider id
 * @param[in] args argument structure
 * @param[out] provider provider
 *
 * @return ALPHA_SUCCESS or error code defined in alpha-common.h
 */
alpha_return_t alpha_provider_register(
        margo_instance_id mid,
        uint16_t provider_id,
        const char* config,
        const struct alpha_provider_args* args,
        alpha_provider_t* provider);

/**
 * @brief Destroys the Alpha provider and deregisters its RPC.
 *
 * @param[in] provider Alpha provider
 *
 * @return ALPHA_SUCCESS or error code defined in alpha-common.h
 */
alpha_return_t alpha_provider_destroy(
        alpha_provider_t provider);

/**
 * @brief Returns a JSON-formatted configuration of the provider.
 *
 * The caller is responsible for freeing the returned pointer.
 *
 * @param provider Alpha provider
 *
 * @return a heap-allocated JSON string or NULL in case of an error.
 */
char* alpha_provider_get_config(
        alpha_provider_t provider);

#ifdef __cplusplus
}
#endif

#endif
