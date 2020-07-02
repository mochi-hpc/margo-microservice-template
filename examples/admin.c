#include <stdio.h>
#include <assert.h>
#include <margo.h>
#include <alpha/alpha-admin.h>
#include "../src/logging.h"

int main(int argc, char** argv)
{
    if(argc != 3) {
        fprintf(stderr,"Usage: %s <server address> <provider id>\n", argv[0]);
        exit(0);
    }

    hg_return_t hret;
    alpha_return_t ret;
    alpha_admin_t admin;
    hg_addr_t svr_addr;
    const char* svr_addr_str = argv[1];
    uint16_t    provider_id  = atoi(argv[2]);
    alpha_resource_id_t id;

    margo_instance_id mid = margo_init("tcp", MARGO_CLIENT_MODE, 0, 0);
    assert(mid);

    hret = margo_addr_lookup(mid, svr_addr_str, &svr_addr);
    if(hret != HG_SUCCESS) {
        LOG_FATAL("margo_addr_lookup failed (ret = %d)", hret);
    }

    LOG_INFO("Initializing admin");
    ret = alpha_admin_init(mid, &admin);
    if(ret != ALPHA_SUCCESS) {
        LOG_FATAL("alpha_admin_init failed (ret = %d)", ret);
    }

    LOG_INFO("Creating resource");
    ret = alpha_create_resource(admin, svr_addr, provider_id, NULL,
                                "beta", "This is a config", &id);
    if(ret != ALPHA_SUCCESS) {
        LOG_FATAL("alpha_create_resource failed (ret = %d)", ret);
    }

    LOG_INFO("Listing resources");
    alpha_resource_id_t ids[4];
    size_t count = 4;
    ret = alpha_list_resources(admin, svr_addr, provider_id, NULL,
                               ids, &count);
    if(ret != ALPHA_SUCCESS) {
        LOG_FATAL("alpha_list_resources failed (ret = %d)", ret);
    }
    LOG_INFO("Returned %ld resource ids", count);

    unsigned i;
    for(i=0; i < count; i++) {
        char id_str[37];
        alpha_resource_id_to_string(ids[i], id_str);
        LOG_INFO("ID %d = %s", i, id_str);
    }

    LOG_INFO("Finalizing admin");
    ret = alpha_admin_finalize(admin);
    if(ret != ALPHA_SUCCESS) {
        LOG_FATAL("alpha_admin_finalize failed (ret = %d)", ret);
    }

    hret = margo_addr_free(mid, svr_addr);
    if(hret != HG_SUCCESS) {
        LOG_FATAL("margo_addr_free failed (ret = %d)", ret);
    }

    margo_finalize(mid);

    return 0;
}
