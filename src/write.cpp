#include "lazygaspi.h"
#include "gaspi_utils.h"
#include "utils.h"

#include <cstring>

gaspi_return_t lazygaspi_write(lazygaspi_id_t row_id, lazygaspi_id_t table_id, void* row){
    LazyGaspiProcessInfo* info;
    auto r = lazygaspi_get_info(&info); ERROR_CHECK;
    
    const auto offset = get_row_offset(info, row_id, table_id);
    auto data = (LazyGaspiRowData*)((char*)info->rows + offset);
    data->age = info->age;

    PRINT_DEBUG("Writing row of age " << data->age);

    memcpy((char*)(info->rows) + offset + sizeof(LazyGaspiRowData), row, info->row_size);

    for(gaspi_rank_t rank = 0; rank < info->n; rank++) if(rank != info->id){
        r = writenotify(SEGMENT_ID_ROWS, SEGMENT_ID_ROWS, offset, offset, sizeof(LazyGaspiRowData) + info->row_size, rank,
                    NOTIF_ID_ROW_WRITTEN);
        ERROR_CHECK;
        PRINT_DEBUG("Wrote to rank " << rank);
    }

    return GASPI_SUCCESS;
}