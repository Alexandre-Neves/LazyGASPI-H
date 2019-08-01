#include "lazygaspi_h.h"
#include "gaspi_utils.h"
#include "utils.h"

#include <cstring>

gaspi_return_t lazygaspi_write(lazygaspi_id_t row_id, lazygaspi_id_t table_id, void* row){
    LazyGaspiProcessInfo* info;
    auto r = lazygaspi_get_info(&info); ERROR_CHECK_COUT;
    
    const auto offset = get_row_offset(info, row_id, table_id);
    auto data = (LazyGaspiRowData*)((char*)info->rows + offset);
    data->age = info->age;

    PRINT_DEBUG_INTERNAL("Writing row " << row_id << " of table " << table_id << " of age " << data->age
                         << " at offset " << offset);

    memcpy((char*)(info->rows) + offset + sizeof(LazyGaspiRowData), row, info->row_size);

    for(gaspi_rank_t rank = 0; rank < info->n; rank++) if(rank != info->id){
        r = writecopy(SEGMENT_ID_ROWS, offset, sizeof(LazyGaspiRowData) + info->row_size, rank);
        ERROR_CHECK;
        PRINT_DEBUG_INTERNAL("Wrote to rank " << rank);
    }
    //Ensures data is sent before it is overwritten by another call to this function
    r = gaspi_wait(0, GASPI_BLOCK); ERROR_CHECK;

    return GASPI_SUCCESS;
}