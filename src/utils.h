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
#include "lazygaspi.h"
#include "gaspi_utils.h"

#define NOTIF_ID_ROW_WRITTEN 0

#ifdef DEBUG
#define PRINT_DEBUG(msg) if(info->out) timestamp(*info->out) << " Rank " << info->id << " => " << msg << std::endl
#else
#define PRINT_DEBUG(msg)
#endif

#define ERROR_CHECK_STREAM(stream) if(r != GASPI_SUCCESS) { stream << "Error " << r << " at " << __FILE__ << ':' << __LINE__ \
                                                            << std::endl; return r; }
#define ERROR_CHECK if(*info->out){ ERROR_CHECK_STREAM(*info->out) } else { DIE_ON_ERROR_OUT("ERROR_CHECK", \
                                                                            "*info->out was null.", std::cout) }

static inline lazygaspi_age_t get_min_age(lazygaspi_age_t current, lazygaspi_age_t slack){
    return current < slack + 2 ? 1 : current - slack - 1;
}

static inline gaspi_offset_t get_row_offset(LazyGaspiProcessInfo* info, lazygaspi_id_t row_id, lazygaspi_id_t table_id){
    return (info->table_size * table_id + row_id) * (sizeof(LazyGaspiRowData) + info->row_size);
}

#endif