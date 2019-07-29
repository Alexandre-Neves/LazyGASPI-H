/** Utility header for the current LazyGASPI implementation. */

#ifndef __H_UTILS
#define __H_UTILS

#include <GASPI.h>
#include <cstdlib>
#include <cassert>
#include <iostream>
#include <thread>
#include <utility>

#include "typedefs.h"
#include "lazygaspi_h.h"
#include "gaspi_utils.h"

#define NOTIF_ID_ROW_WRITTEN 0

#if (defined (DEBUG) || defined (DEBUG_INTERNAL))
#define PRINT_DEBUG_INTERNAL_OUT(out, msg) if(out){ *out << msg << std::endl; }\
                                           else{ std::cout << msg << std::endl; }
#define PRINT_DEBUG_INTERNAL(msg) PRINT_DEBUG_INTERNAL_OUT(info->out, msg)
#else
#define PRINT_DEBUG_INTERNAL_OUT(out, msg)
#define PRINT_DEBUG_INTERNAL(msg)
#endif

#if (defined (DEBUG) || defined (DEBUG_TEST))
#define PRINT_DEBUG_TEST(msg) {if(info->out) {*info->out << msg << std::endl; }\
                                   else std::cout << msg << std::endl; }
#else
#define PRINT_DEBUG_TEST(msg)
#endif

#if (defined (DEBUG) || defined (DEBUG_PERF))
#define PRINT_DEBUG_PERF(msg) {if(info->out) {*info->out << msg << std::endl; }\
                                   else std::cout << msg << std::endl; }
#else
#define PRINT_DEBUG_PERF(msg)
#endif

#if defined(DEBUG) || defined(DEBUG_INTERNAL) || defined(DEBUG_TEST) || defined(DEBUG_PERF)
#define PRINT_DEBUG_OUT(out, msg) { if(out) {*out << msg << std::endl; }\
                                  else std::cout << msg << std::endl; }
#define PRINT_DEBUG(msg) PRINT_DEBUG_OUT(info->out, msg)
#define PRINT_TIMESTAMP { if(info->out){ timestamp(*info->out) << std::endl;}\
                          else timestamp(std::cout) << std::endl; }
#else
#define PRINT_DEBUG_OUT(out, msg)
#define PRINT_DEBUG(msg)
#define PRINT_TIMESTAMP
#endif

static inline lazygaspi_age_t get_min_age(lazygaspi_age_t current, lazygaspi_age_t slack, bool offset_slack){
    return (current < slack + 1 + offset_slack) ? 1 : (current - slack - offset_slack);
}

static inline gaspi_offset_t get_row_offset(LazyGaspiProcessInfo* info, lazygaspi_id_t row_id, lazygaspi_id_t table_id){
    return (info->table_size * table_id + row_id) * (sizeof(LazyGaspiRowData) + info->row_size);
}

#endif