/** Utility header for the current LazyGASPI implementation. */

#ifndef __H_UTILS
#define __H_UTILS

#include <GASPI.h>
#include <cstdlib>
#include <cassert>
#include <iostream>
#include <thread>
#include <utility>

#include "lazygaspi_h.h"
#include "gaspi_utils.h"

#define NOTIF_ID_ROW_WRITTEN 0

typedef unsigned int uint;
typedef unsigned char byte;
typedef unsigned long ulong;

#define subsizeof(STRUCT, MEMBER_BEGIN, MEMBER_END) (offsetof(STRUCT, MEMBER_END) + sizeof(decltype(std::declval<STRUCT>().MEMBER_END)) - offsetof(STRUCT, MEMBER_BEGIN))
#define sizeofmember(STRUCT, MEMBER) sizeof((( STRUCT *)0)-> MEMBER)

#if (defined (DEBUG) || defined (DEBUG_INTERNAL))
#define PRINT_DEBUG_INTERNAL(msg) *info->out << msg << std::endl
#define PRINT_DEBUG_INTERNAL_COUT(msg) std::cout << msg << std::endl
#else
#define PRINT_DEBUG_INTERNAL(msg)
#define PRINT_DEBUG_INTERNAL_COUT(msg)
#endif

#if (defined (DEBUG) || defined (DEBUG_TEST))
#define PRINT_DEBUG_TEST(msg) *info->out << msg << std::endl
#else
#define PRINT_DEBUG_TEST(msg)
#endif

#if (defined (DEBUG) || defined (DEBUG_PERF))
#define PRINT_DEBUG_PERF(msg) *info->out << msg << std::endl
#else
#define PRINT_DEBUG_PERF(msg)
#endif

#if defined DEBUG || defined DEBUG_PERF || defined DEBUG_TEST || defined DEBUG_INTERNAL
#define PRINT_DEBUG(msg) *info->out << msg << std::endl
#define PRINT_DEBUG_COUT(msg) std::cout << msg << std::endl
#else
#define PRINT_DEBUG(msg)
#define PRINT_DEBUG_COUT(msg)
#endif

#ifdef WITH_MPI
#include <mpi.h>
#define ERROR_MPI_CHECK_COUT(msg) {if(ret != MPI_SUCCESS){ std::cout << "Error " << ret << " at [" << __FILE__ << ':' << __LINE__ << "] \
                              from MPI: " << msg << std::endl; return GASPI_ERROR; }}
#endif

#ifdef LOCKED_OPERATIONS
    #define LOCK_MASK_WRITE (((gaspi_atomic_value_t)1) << (sizeof(gaspi_atomic_value_t) * 8 - 1))

    struct Lock{ gaspi_atomic_value_t val; };

    gaspi_return_t lock_row_for_read(const LazyGaspiProcessInfo* info, const gaspi_segment_id_t seg, const gaspi_offset_t offset, 
                       const gaspi_rank_t rank);
    gaspi_return_t unlock_row_from_read(const LazyGaspiProcessInfo* info, const gaspi_segment_id_t seg, const gaspi_offset_t offset,
                          const gaspi_rank_t rank);
    gaspi_return_t lock_row_for_write(const LazyGaspiProcessInfo* info, const gaspi_segment_id_t seg, const gaspi_offset_t offset, 
                        const gaspi_rank_t rank);
    gaspi_return_t unlock_row_from_write(LazyGaspiProcessInfo* info, const gaspi_segment_id_t seg, const gaspi_offset_t offset,
                           const gaspi_rank_t rank, const gaspi_queue_id_t q = 0, bool wait_on_q = true);

    #define ROW_ENTRY_SIZE (sizeof(Lock) + sizeof(LazyGaspiRowData) + info->row_size)
    #define ROW_ENTRY_SIZE_WITH_LOCK (ROW_ENTRY_SIZE + sizeof(Lock))
    #define ROW_LOCK_OFFSET 0
    #define ROW_METADATA_OFFSET (ROW_LOCK_OFFSET + sizeof(Lock))
#else
    #define ROW_ENTRY_SIZE (sizeof(LazyGaspiRowData) + info->row_size)
    #define ROW_ENTRY_SIZE_WITH_LOCK ROW_ENTRY_SIZE
    #define ROW_METADATA_OFFSET 0
#endif

#define ROW_DATA_OFFSET (ROW_METADATA_OFFSET + sizeof(LazyGaspiRowData))

static inline lazygaspi_age_t get_min_age(lazygaspi_age_t current, lazygaspi_age_t slack, bool offset_slack){
    return (current < slack + 1 + offset_slack) ? 1 : (current - slack - offset_slack);
}

static inline gaspi_offset_t get_row_offset(LazyGaspiProcessInfo* info, lazygaspi_id_t row_id, lazygaspi_id_t table_id){
    return (info->table_size * table_id + row_id) * ROW_ENTRY_SIZE_WITH_LOCK;
}

#endif