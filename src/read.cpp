#include "lazygaspi_h.h"
#include "gaspi_utils.h"
#include "utils.h"

#include <cstring>

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