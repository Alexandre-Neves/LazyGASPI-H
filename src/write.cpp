#include "lazygaspi_h.h"
#include "gaspi_utils.h"
#include "utils.h"

#include <cstring>

gaspi_return_t lazygaspi_write(lazygaspi_id_t row_id, lazygaspi_id_t table_id, void* row){
    LazyGaspiProcessInfo* info;
    auto r = lazygaspi_get_info(&info); ERROR_CHECK_COUT;
    
    const auto offset = get_row_offset(info, row_id, table_id);

    #ifdef LOCKED_OPERATIONS
    r = lock_row_for_write(info, LAZYGASPI_ID_ROWS, offset + ROW_LOCK_OFFSET, info->id); ERROR_CHECK;
    #endif

    auto data = (LazyGaspiRowData*)((char*)info->rows + offset + ROW_METADATA_OFFSET);
    data->age = info->age;

    PRINT_DEBUG_INTERNAL("Writing row " << row_id << " of table " << table_id << " of age " << data->age
                         << " at offset " << offset);

    memcpy((char*)(info->rows) + offset + ROW_DATA_OFFSET, row, info->row_size);

    for(gaspi_rank_t rank = 0; rank < info->n; rank++) if(rank != info->id){
        #ifdef LOCKED_OPERATIONS
        r = lock_row_for_write(info, LAZYGASPI_ID_ROWS, offset + ROW_LOCK_OFFSET, rank);
        #endif
        //Read lock for local segment is never used because only the current process will read from its own rows, so write lock 
        //can stay instead.
        r = writecopy(LAZYGASPI_ID_ROWS, offset + ROW_METADATA_OFFSET, ROW_ENTRY_SIZE, rank);
        ERROR_CHECK;

        #ifdef LOCKED_OPERATIONS
        r = unlock_row_from_write(info, LAZYGASPI_ID_ROWS, offset + ROW_LOCK_OFFSET, rank);
        #endif
        PRINT_DEBUG_INTERNAL(" | Wrote to rank " << rank);
    }

    #ifdef LOCKED_OPERATIONS
    r = unlock_row_from_write(info, LAZYGASPI_ID_ROWS, offset + ROW_LOCK_OFFSET, info->id);
    #endif
    //Ensures data is sent before it is overwritten by another call to this function
    r = gaspi_wait(0, GASPI_BLOCK); ERROR_CHECK;

    return GASPI_SUCCESS;
}