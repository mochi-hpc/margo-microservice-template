/*
 * (C) 2020 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */
#include <string.h>
#include <jansson.h>
#include "alpha/alpha-backend.h"
#include "../logging.h"
#include "dummy-backend.h"

typedef struct dummy_context {
    json_t* config;
    /* ... */
} dummy_context;

static alpha_return_t dummy_create_resource(
        alpha_provider_t provider,
        const char* config,
        void** context)
{
    (void)provider;

    json_t *root;
    json_error_t error;
    root = json_loads(config, 0, &error);
    if(!root) {
        LOG_ERROR("JSON configuration error on line %d: %s", error.line, error.text);
        return ALPHA_ERR_INVALID_CONFIG;
    }

    dummy_context* ctx = (dummy_context*)calloc(1, sizeof(*ctx));
    ctx->config = root;
    *context = (void*)ctx;
    return ALPHA_SUCCESS;
}

static alpha_return_t dummy_open_resource(
        alpha_provider_t provider,
        const char* config,
        void** context)
{
    (void)provider;

    json_t *root;
    json_error_t error;
    root = json_loads(config, 0, &error);
    if(!root) {
        LOG_ERROR("JSON configuration error on line %d: %s", error.line, error.text);
        return ALPHA_ERR_INVALID_CONFIG;
    }

    dummy_context* ctx = (dummy_context*)calloc(1, sizeof(*ctx));
    ctx->config = root;
    *context = (void*)ctx;
    return ALPHA_SUCCESS;
}

static alpha_return_t dummy_close_resource(void* ctx)
{
    dummy_context* context = (dummy_context*)ctx;
    json_decref(context->config);
    free(context);
    return ALPHA_SUCCESS;
}

static alpha_return_t dummy_destroy_resource(void* ctx)
{
    dummy_context* context = (dummy_context*)ctx;
    json_decref(context->config);
    free(context);
    return ALPHA_SUCCESS;
}

static void dummy_say_hello(void* ctx)
{
    dummy_context* context = (dummy_context*)ctx;
    (void)context;
    printf("Hello World from Dummy resource\n");
}

static int32_t dummy_compute_sum(void* ctx, int32_t x, int32_t y)
{
    (void)ctx;
    return x+y;
}

static alpha_backend_impl dummy_backend = {
    .name             = "dummy",

    .create_resource  = dummy_create_resource,
    .open_resource    = dummy_open_resource,
    .close_resource   = dummy_close_resource,
    .destroy_resource = dummy_destroy_resource,

    .hello            = dummy_say_hello,
    .sum              = dummy_compute_sum
};

alpha_return_t alpha_provider_register_dummy_backend(alpha_provider_t provider)
{
    return alpha_provider_register_backend(provider, &dummy_backend);
}
