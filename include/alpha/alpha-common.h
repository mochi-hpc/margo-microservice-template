/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __ALPHA_COMMON_H
#define __ALPHA_COMMON_H

#include <stdint.h>
#include <margo.h>
#include <mercury_proc.h>
#include <mercury_proc_bulk.h>
#include <mercury_proc_string.h>

#ifdef __cplusplus
extern "C" {
#endif

// TUTORIAL
// ********
//
// This header hosts common definitions used by both the client and server libraries.

/**
 * @brief Error codes that can be returned by ALPHA functions.
 */
typedef enum alpha_return_t {
    ALPHA_SUCCESS,
    ALPHA_TIMEOUT,               /* Timeout */
    ALPHA_ERR_ALLOCATION,        /* Allocation error */
    ALPHA_ERR_INVALID_ARGS,      /* Invalid argument */
    ALPHA_ERR_INVALID_PROVIDER,  /* Invalid provider id */
    ALPHA_ERR_INVALID_RESOURCE,  /* Invalid resource id */
    ALPHA_ERR_INVALID_BACKEND,   /* Invalid backend type */
    ALPHA_ERR_INVALID_CONFIG,    /* Invalid configuration */
    ALPHA_ERR_INVALID_TOKEN,     /* Invalid token */
    ALPHA_ERR_FROM_MERCURY,      /* Mercurt error */
    ALPHA_ERR_FROM_ARGOBOTS,     /* Argobots error */
    ALPHA_ERR_OP_UNSUPPORTED,    /* Unsupported operation */
    ALPHA_ERR_OP_FORBIDDEN,      /* Forbidden operation */
    /* ... TODO add more error codes here if needed */
    ALPHA_ERR_OTHER              /* Other error */
} alpha_return_t;

/**
 * The alpha_bulk_location_t structure encapsulates a bulk handle
 * with the address it originates from, and the range (offset, size)
 * that is relevant for the server to perform its operation on.
 *
 * It is defined using MERCURY_GEN_PROC so that it can be serialized
 * as an RPC argument.
 */
MERCURY_GEN_PROC(alpha_bulk_location_t,
    ((hg_bulk_t)(bulk))\
    ((hg_string_t)(address))\
    ((int64_t)(offset))\
    ((int64_t)(size)))

#ifdef __cplusplus
}
#endif

#endif
