#include <stdio.h>
#include <margo.h>
#include <alpha/alpha-admin.h>

int main(int argc, char** argv)
{
    if(argc != 3) {
        fprintf(stderr,"Usage: %s <server address> <provider id>\n", argv[0]);
        exit(0);
    }

    alpha_return_t ret;
    alpha_admin_t admin;
    hg_addr_t svr_addr;
    const char* svr_addr_str = argv[1];
    uint16_t    provider_id  = atoi(argv[2]);
    alpha_resource_id_t id1, id2;

    margo_instance_id mid = margo_init("tcp", MARGO_CLIENT_MODE, 0, 0);

    margo_addr_lookup(mid, svr_addr_str, &svr_addr);

    printf("Initializing admin\n");
    ret = alpha_admin_init(mid, &admin);
    printf("ret = %d\n", ret);

    printf("Creating resource\n");
    ret = alpha_create_resource(admin, svr_addr, provider_id, NULL,
                                "beta", "This is a config", &id1);
    printf("ret = %d\n", ret);
    printf("Creating resource\n");
    ret = alpha_create_resource(admin, svr_addr, provider_id, NULL,
                                "beta", "This is a config", &id2);
    printf("ret = %d\n", ret);

    printf("Listing resources\n");
    alpha_resource_id_t ids[4];
    size_t count = 4;
    ret = alpha_list_resources(admin, svr_addr, provider_id, NULL,
                               ids, &count);
    printf("ret = %d\n", ret);
    printf("Returned %ld resource ids\n", count);

    printf("Destroying a resource\n");
    ret = alpha_destroy_resource(admin, svr_addr, provider_id, NULL, id1);
    printf("ret = %d\n", ret);

    printf("Listing resources\n");
    count = 4;
    ret = alpha_list_resources(admin, svr_addr, provider_id, NULL,
                               ids, &count);
    printf("ret = %d\n", ret);
    printf("Returned %ld resource ids\n", count);

    printf("Finalizing admin\n");
    ret = alpha_admin_finalize(admin);
    printf("ret = %d\n", ret);

    margo_addr_free(mid, svr_addr);

    margo_finalize(mid);

    return 0;
}
