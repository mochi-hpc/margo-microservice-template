#include <string.h>
#include "alpha/alpha-backend.h"
#include "beta-backend.h"

typedef struct beta_context {
    char* config;
    /* ... */
} beta_context;

static alpha_return_t beta_create_resource(
        alpha_provider_t provider,
        const char* config,
        void** context)
{
    (void)provider;
    beta_context* ctx = (beta_context*)calloc(1, sizeof(*ctx));
    ctx->config = strdup(config);
    *context = (void*)ctx;
    return ALPHA_SUCCESS;
}

static alpha_return_t beta_open_resource(
        alpha_provider_t provider,
        const char* config,
        void** context)
{
    (void)provider;
    beta_context* ctx = (beta_context*)calloc(1, sizeof(*ctx));
    ctx->config = strdup(config);
    *context = (void*)ctx;
    return ALPHA_SUCCESS;
}

static alpha_return_t beta_close_resource(void* ctx)
{
    beta_context* context = (beta_context*)ctx;
    free(context->config);
    free(context);
    return ALPHA_SUCCESS;
}

static alpha_return_t beta_destroy_resource(void* ctx)
{
    beta_context* context = (beta_context*)ctx;
    free(context->config);
    free(context);
    return ALPHA_SUCCESS;
}

static void beta_say_hello(void* ctx)
{
    beta_context* context = (beta_context*)ctx;
    printf("Hello World from Beta resource\n");
    printf("My configuration is:\n %s\n", context->config);
}

static int32_t beta_compute_sum(void* ctx, int32_t x, int32_t y)
{
    (void)ctx;
    return x+y;
}

static alpha_backend_impl beta_backend = {
    .name             = "beta",

    .create_resource  = beta_create_resource,
    .open_resource    = beta_open_resource,
    .close_resource   = beta_close_resource,
    .destroy_resource = beta_destroy_resource,

    .hello            = beta_say_hello,
    .sum              = beta_compute_sum
};

alpha_return_t alpha_provider_register_beta_backend(alpha_provider_t provider)
{
    return alpha_provider_register_backend(provider, &beta_backend);
}
