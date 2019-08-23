#include "lazygaspi_h.h"
#include "gaspi_utils.h"
#include "utils.h"

#include <cstring>

#ifdef LOCKED_OPERATIONS

gaspi_return_t lock_row_for_read(const LazyGaspiProcessInfo* info, const gaspi_segment_id_t seg, const gaspi_offset_t offset, 
                       const gaspi_rank_t rank){
    gaspi_atomic_value_t oldval;
    gaspi_return_t r;
    
    PRINT_DEBUG_INTERNAL(" | : Locking row from segment " << (int)seg << " at offset " << offset << " of rank " << rank << " for READ.");

    wait_for_write:
    do {
        r = gaspi_atomic_compare_swap(seg, offset, rank, 0, 1, &oldval, GASPI_BLOCK); ERROR_CHECK;
        PRINT_DEBUG_INTERNAL(" | : > Compare and swap saw " << oldval);
    } while((oldval & LOCK_MASK_WRITE) != 0);   //While row is being written, keep trying to lock

    //Lock was unlocked by writer, but another process became the "first reader" faster. Use fetch&add
    if(oldval != 0) { 
        PRINT_DEBUG_INTERNAL(" | : > Row was locked for reading before. Increasing lock count to " << oldval + 1 << "...");
        r = gaspi_atomic_fetch_add(seg, offset, rank, 1, &oldval, GASPI_BLOCK); ERROR_CHECK;
        //This conditional can only be true if the following happens:
        //Lock is free for writes (0 at the write bit); Read lock is set (`n` at the read bits); Before fetch&add of this proc, 
        //all `n` readers unlock the lock (becomes 0 again); Another writer process locks (sets write bit to 1)
        if((oldval & LOCK_MASK_WRITE) != 0){
            PRINT_DEBUG_INTERNAL(" | : > Write lock was placed before read lock could have. Retrying...");
            goto wait_for_write; 
        }
    }
    return GASPI_SUCCESS;
}

gaspi_return_t unlock_row_from_read(const LazyGaspiProcessInfo* info, const gaspi_segment_id_t seg, const gaspi_offset_t offset,
                                    const gaspi_rank_t rank){
    gaspi_atomic_value_t val;

    PRINT_DEBUG_INTERNAL(" | : Unlocking row from segment " << (int)seg << " at offset " << offset << " of rank " << rank << " from READ.");

    auto r = gaspi_atomic_fetch_add(seg, offset, rank, (gaspi_atomic_value_t)-1, &val, GASPI_BLOCK); 
    PRINT_DEBUG_INTERNAL(" | : > Old value was " << val);
    if(r != GASPI_SUCCESS) PRINT_ON_ERROR(r);
    return r;
}

#endif

gaspi_return_t lazygaspi_read(lazygaspi_id_t row_id, lazygaspi_id_t table_id, lazygaspi_slack_t slack, void* row, 
                              LazyGaspiRowData* outData){
    LazyGaspiProcessInfo* info;
    auto r = lazygaspi_get_info(&info); ERROR_CHECK_COUT;

    const auto offset = get_row_offset(info, row_id, table_id);

    #ifdef LOCKED_OPERATIONS
    r = lock_row_for_read(info, LAZYGASPI_ID_ROWS, offset + ROW_LOCK_OFFSET, info->id); ERROR_CHECK;
    #endif

    auto data = (LazyGaspiRowData*)((char*)info->rows + offset + ROW_METADATA_OFFSET);
    Notification notif;
    const auto min = get_min_age(info->age, slack, info->offset_slack);

    PRINT_DEBUG_INTERNAL("Reading row (" << row_id << ", " << table_id << ") with minimum age " << min);
    #if defined DEBUG || defined DEBUG_INTERNAL 
    if(data->age < min) PRINT_DEBUG_INTERNAL("Age below minimum (" << data->age << "). Waiting for write...");
    #endif

    while(data->age < min)
    #ifdef LOCKED_OPERATIONS
    {   r = unlock_row_from_read(info, LAZYGASPI_ID_ROWS, offset + ROW_LOCK_OFFSET, info->id); ERROR_CHECK;
        r = lock_row_for_read(info, LAZYGASPI_ID_ROWS, offset + ROW_LOCK_OFFSET, info->id);    ERROR_CHECK;    }
    #else
    ;
    #endif

    PRINT_DEBUG_INTERNAL("Age fresh enough (" << data->age << "). Outputting row...");

    memcpy(row, (char*)data + sizeof(LazyGaspiRowData), info->row_size);
    if(outData) *outData = *data;

    #ifdef LOCKED_OPERATIONS
    r = unlock_row_from_read(info, LAZYGASPI_ID_ROWS, offset + ROW_LOCK_OFFSET, info->id); ERROR_CHECK;
    #endif
    
    return GASPI_SUCCESS;
}