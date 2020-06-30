#include "alpha/alpha-server.h"
#include "provider.h"
#include "types.h"

// backends that we want to add at compile time
#include "beta/beta-backend.h"

static void alpha_finalize_provider(void* p);

/* Functions to manipulate the hash of resources */
static inline alpha_resource* find_resource(
        alpha_provider_t provider,
        const alpha_resource_id_t* id);

static inline alpha_return_t add_resource(
        alpha_provider_t provider,
        alpha_resource* resource);

static inline alpha_return_t remove_resource(
        alpha_provider_t provider,
        const alpha_resource_id_t* id);

static inline void remove_all_resources(
        alpha_provider_t provider);

/* Functions to manipulate the list of backend types */
static inline alpha_backend_impl* find_backend_impl(
        alpha_provider_t provider,
        const char* name);

static inline alpha_return_t add_backend_impl(
        alpha_provider_t provider,
        alpha_backend_impl* backend);

/* Admin RPCs */
static DECLARE_MARGO_RPC_HANDLER(alpha_create_resource_ult);
static void alpha_create_resource_ult(hg_handle_t h);
static DECLARE_MARGO_RPC_HANDLER(alpha_open_resource_ult);
static void alpha_open_resource_ult(hg_handle_t h);
static DECLARE_MARGO_RPC_HANDLER(alpha_close_resource_ult);
static void alpha_close_resource_ult(hg_handle_t h);
static DECLARE_MARGO_RPC_HANDLER(alpha_destroy_resource_ult);
static void alpha_destroy_resource_ult(hg_handle_t h);
static DECLARE_MARGO_RPC_HANDLER(alpha_list_resources_ult);
static void alpha_list_resources_ult(hg_handle_t h);

/* Client RPCs */
static DECLARE_MARGO_RPC_HANDLER(alpha_hello_ult);
static void alpha_hello_ult(hg_handle_t h);
static DECLARE_MARGO_RPC_HANDLER(alpha_sum_ult);
static void alpha_sum_ult(hg_handle_t h);

/* add other RPC declarations here */

int alpha_provider_register(
        margo_instance_id mid,
        uint16_t provider_id,
        const char* token,
        ABT_pool pool,
        abt_io_instance_id abtio,
        alpha_provider_t* provider)
{
    alpha_provider_t p;
    hg_id_t id;
    hg_bool_t flag;

    flag = margo_is_listening(mid);
    if(flag == HG_FALSE) {
        fprintf(stderr, "alpha_provider_register(): margo instance is not a server.");
        return ALPHA_ERR_INVALID_ARGS;
    }

    margo_provider_registered_name(mid, "alpha_sum", provider_id, &id, &flag);
    if(flag == HG_TRUE) {
        fprintf(stderr, "alpha_provider_register(): a provider with the same provider id (%d) already exists.\n", provider_id);
        return ALPHA_ERR_INVALID_PROVIDER;
    }

    p = (alpha_provider_t)calloc(1, sizeof(*p));
    if(p == NULL)
        return ALPHA_ERR_ALLOCATION;

    p->mid = mid;
    p->provider_id = provider_id;
    p->pool = pool;
    p->abtio = abtio;
    p->token = token ? strdup(token) : NULL;

    /* Admin RPCs */
    id = MARGO_REGISTER_PROVIDER(mid, "alpha_create_resource",
            create_resource_in_t, create_resource_out_t,
            alpha_create_resource_ult, provider_id, pool);
    margo_register_data(mid, id, (void*)p, NULL);
    p->create_resource_id = id;

    id = MARGO_REGISTER_PROVIDER(mid, "alpha_open_resource",
            open_resource_in_t, open_resource_out_t,
            alpha_open_resource_ult, provider_id, pool);
    margo_register_data(mid, id, (void*)p, NULL);
    p->open_resource_id = id;

    id = MARGO_REGISTER_PROVIDER(mid, "alpha_close_resource",
            close_resource_in_t, close_resource_out_t,
            alpha_close_resource_ult, provider_id, pool);
    margo_register_data(mid, id, (void*)p, NULL);
    p->close_resource_id = id;

    id = MARGO_REGISTER_PROVIDER(mid, "alpha_destroy_resource",
            destroy_resource_in_t, destroy_resource_out_t,
            alpha_destroy_resource_ult, provider_id, pool);
    margo_register_data(mid, id, (void*)p, NULL);
    p->destroy_resource_id = id;

    id = MARGO_REGISTER_PROVIDER(mid, "alpha_list_resources",
            list_resources_in_t, list_resources_out_t,
            alpha_list_resources_ult, provider_id, pool);
    margo_register_data(mid, id, (void*)p, NULL);
    p->list_resources_id = id;

    /* Client RPCs */

    id = MARGO_REGISTER_PROVIDER(mid, "alpha_hello",
            hello_in_t, void,
            alpha_hello_ult, provider_id, pool);
    margo_register_data(mid, id, (void*)p, NULL);
    p->hello_id = id;

    id = MARGO_REGISTER_PROVIDER(mid, "alpha_sum",
            sum_in_t, sum_out_t,
            alpha_sum_ult, provider_id, pool);
    margo_register_data(mid, id, (void*)p, NULL);
    p->sum_id = id;

    /* add other RPC registration here */
    /* ... */

    /* add backends available at compiler time (e.g. default/dummy backends) */
    alpha_provider_register_beta_backend(p); // function from "beta/beta-backend.h"

    margo_provider_push_finalize_callback(mid, p, &alpha_finalize_provider, p);

    if(provider)
        *provider = p;
    return ALPHA_SUCCESS;
}

static void alpha_finalize_provider(void* p)
{
    alpha_provider_t provider = (alpha_provider_t)p;
    margo_deregister(provider->mid, provider->create_resource_id);
    margo_deregister(provider->mid, provider->open_resource_id);
    margo_deregister(provider->mid, provider->close_resource_id);
    margo_deregister(provider->mid, provider->destroy_resource_id);
    margo_deregister(provider->mid, provider->list_resources_id);
    margo_deregister(provider->mid, provider->hello_id);
    margo_deregister(provider->mid, provider->sum_id);
    /* deregister other RPC ids ... */
    remove_all_resources(provider);
    free(provider->backend_types);
    free(provider->token);
    free(provider);
}

int alpha_provider_destroy(
        alpha_provider_t provider)
{
    /* pop the finalize callback */
    margo_provider_pop_finalize_callback(provider->mid, provider);
    /* call the callback */
    alpha_finalize_provider(provider);

    return ALPHA_SUCCESS;
}

alpha_return_t alpha_provider_register_backend(
        alpha_provider_t provider,
        alpha_backend_impl* backend_impl)
{
    return add_backend_impl(provider, backend_impl);
}

static void alpha_create_resource_ult(hg_handle_t h)
{
    hg_return_t ret;
    create_resource_in_t  in;
    create_resource_out_t out;

    margo_instance_id mid = margo_hg_handle_get_instance(h);

    const struct hg_info* info = margo_get_info(h);
    alpha_provider_t provider = (alpha_provider_t)margo_registered_data(mid, info->id);

    ret = margo_get_input(h, &in);

    // TODO

    ret = margo_respond(h, &out);
    ret = margo_free_input(h, &in);
    margo_destroy(h);
}
static DEFINE_MARGO_RPC_HANDLER(alpha_create_resource_ult)

static void alpha_open_resource_ult(hg_handle_t h)
{
    hg_return_t ret;
    open_resource_in_t  in;
    open_resource_out_t out;

    margo_instance_id mid = margo_hg_handle_get_instance(h);

    const struct hg_info* info = margo_get_info(h);
    alpha_provider_t provider = (alpha_provider_t)margo_registered_data(mid, info->id);

    ret = margo_get_input(h, &in);

    // TODO

    ret = margo_respond(h, &out);
    ret = margo_free_input(h, &in);
    margo_destroy(h);
}
static DEFINE_MARGO_RPC_HANDLER(alpha_open_resource_ult)

static void alpha_close_resource_ult(hg_handle_t h)
{
    hg_return_t ret;
    close_resource_in_t  in;
    close_resource_out_t out;

    margo_instance_id mid = margo_hg_handle_get_instance(h);

    const struct hg_info* info = margo_get_info(h);
    alpha_provider_t provider = (alpha_provider_t)margo_registered_data(mid, info->id);

    ret = margo_get_input(h, &in);

    // TODO

    ret = margo_respond(h, &out);
    ret = margo_free_input(h, &in);
    margo_destroy(h);
}
static DEFINE_MARGO_RPC_HANDLER(alpha_close_resource_ult)

static void alpha_destroy_resource_ult(hg_handle_t h)
{
    hg_return_t ret;
    destroy_resource_in_t  in;
    destroy_resource_out_t out;

    margo_instance_id mid = margo_hg_handle_get_instance(h);

    const struct hg_info* info = margo_get_info(h);
    alpha_provider_t provider = (alpha_provider_t)margo_registered_data(mid, info->id);

    ret = margo_get_input(h, &in);

    // TODO

    ret = margo_respond(h, &out);
    ret = margo_free_input(h, &in);
    margo_destroy(h);
}
static DEFINE_MARGO_RPC_HANDLER(alpha_destroy_resource_ult)

static void alpha_list_resources_ult(hg_handle_t h)
{
    hg_return_t ret;
    list_resources_in_t  in;
    list_resources_out_t out;

    margo_instance_id mid = margo_hg_handle_get_instance(h);

    const struct hg_info* info = margo_get_info(h);
    alpha_provider_t provider = (alpha_provider_t)margo_registered_data(mid, info->id);

    ret = margo_get_input(h, &in);

    // TODO

    ret = margo_respond(h, &out);
    ret = margo_free_input(h, &in);
    margo_destroy(h);
}
static DEFINE_MARGO_RPC_HANDLER(alpha_list_resources_ult)

static void alpha_hello_ult(hg_handle_t h)
{
    hg_return_t ret;
    margo_instance_id mid = margo_hg_handle_get_instance(h);

    const struct hg_info* info = margo_get_info(h);
    alpha_provider_t provider = (alpha_provider_t)margo_registered_data(mid, info->id);

    printf("Hello World!\n");

    margo_destroy(h);
}
static DEFINE_MARGO_RPC_HANDLER(alpha_hello_ult)

static void alpha_sum_ult(hg_handle_t h)
{
    hg_return_t ret;
    sum_in_t     in;
    sum_out_t   out;

    margo_instance_id mid = margo_hg_handle_get_instance(h);

    const struct hg_info* info = margo_get_info(h);
    alpha_provider_t provider = (alpha_provider_t)margo_registered_data(mid, info->id);

    ret = margo_get_input(h, &in);

    out.ret = in.x + in.y;
    printf("Computed %d + %d = %d\n",in.x,in.y,out.ret);

    ret = margo_respond(h, &out);
    ret = margo_free_input(h, &in);
    margo_destroy(h);
}
static DEFINE_MARGO_RPC_HANDLER(alpha_sum_ult)

static inline alpha_resource* find_resource(
        alpha_provider_t provider,
        const alpha_resource_id_t* id)
{
    alpha_resource* resource = NULL;
    HASH_FIND(hh, provider->resources, id, sizeof(alpha_resource_id_t), resource);
    return resource;
}

static inline alpha_return_t add_resource(
        alpha_provider_t provider,
        alpha_resource* resource)
{
    alpha_resource* existing = find_resource(provider, &(resource->id));
    if(existing) {
        return ALPHA_ERR_INVALID_RESOURCE;
    }
    HASH_ADD(hh, provider->resources, id, sizeof(alpha_resource_id_t), resource);
    return ALPHA_SUCCESS;
}

static inline alpha_return_t remove_resource(
        alpha_provider_t provider,
        const alpha_resource_id_t* id)
{
    alpha_resource* resource = find_resource(provider, id);
    if(!resource) {
        return ALPHA_ERR_INVALID_RESOURCE;
    }
    resource->fn->close_resource(resource->ctx);
    HASH_DEL(provider->resources, resource);
    free(resource);
    return ALPHA_SUCCESS;
}

static inline void remove_all_resources(
        alpha_provider_t provider)
{
    alpha_resource *r, *tmp;
    HASH_ITER(hh, provider->resources, r, tmp) {
        HASH_DEL(provider->resources, r);
        r->fn->close_resource(r->ctx);
        free(r);
    }
}

static inline alpha_backend_impl* find_backend_impl(
        alpha_provider_t provider,
        const char* name)
{
    size_t i;
    for(i = 0; i < provider->num_backend_types; i++) {
        alpha_backend_impl* impl = provider->backend_types[i];
        if(strcmp(name, impl->name) == 0)
            return impl;
    }
    return NULL;
}

static inline alpha_return_t add_backend_impl(
        alpha_provider_t provider,
        alpha_backend_impl* backend)
{
    provider->num_backend_types += 1;
    provider->backend_types = realloc(provider->backend_types,
                                      provider->num_backend_types);
    provider->backend_types[provider->num_backend_types-1] = backend;
    return ALPHA_SUCCESS;
}

