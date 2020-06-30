#ifndef _PARAMS_H
#define _PARAMS_H

#include <mercury.h>
#include <mercury_macros.h>
#include <mercury_proc.h>
#include <mercury_proc_string.h>
#include "alpha/alpha-common.h"

static inline hg_return_t hg_proc_alpha_resource_id_t(hg_proc_t proc, alpha_resource_id_t *id);

/* Admin RPC types */

MERCURY_GEN_PROC(create_resource_in_t,
        ((hg_string_t)(type))\
        ((hg_string_t)(config))\
        ((hg_string_t)(token)))

MERCURY_GEN_PROC(create_resource_out_t,
        ((int32_t)(ret))\
        ((alpha_resource_id_t)(id)))

MERCURY_GEN_PROC(open_resource_in_t,
        ((hg_string_t)(type))\
        ((hg_string_t)(config))\
        ((hg_string_t)(token)))

MERCURY_GEN_PROC(open_resource_out_t,
        ((int32_t)(ret))\
        ((alpha_resource_id_t)(id)))

MERCURY_GEN_PROC(close_resource_in_t,
        ((hg_string_t)(token))\
        ((alpha_resource_id_t)(id)))

MERCURY_GEN_PROC(close_resource_out_t,
        ((int32_t)(ret)))

MERCURY_GEN_PROC(destroy_resource_in_t,
        ((hg_string_t)(token))\
        ((alpha_resource_id_t)(id)))

MERCURY_GEN_PROC(destroy_resource_out_t,
        ((int32_t)(ret)))

MERCURY_GEN_PROC(list_resources_in_t,
        ((hg_string_t)(token))\
        ((hg_bulk_t)(memory_handle))\
        ((hg_size_t)(max_ids)))

MERCURY_GEN_PROC(list_resources_out_t,
        ((int32_t)(ret))\
        ((hg_size_t)(ids_count)))

/* Client RPC types */

MERCURY_GEN_PROC(sum_in_t,
        ((alpha_resource_id_t)(resource_id))\
        ((int32_t)(x))\
        ((int32_t)(y)))

MERCURY_GEN_PROC(sum_out_t,
        ((int32_t)(ret)))

MERCURY_GEN_PROC(hello_in_t,
        ((alpha_resource_id_t)(resource_id)))

/* Extra hand-coded serialization functions */

static inline hg_return_t hg_proc_alpha_resource_id_t(
        hg_proc_t proc, alpha_resource_id_t *id)
{
    return hg_proc_memcpy(proc, id, sizeof(*id));
}

#endif
