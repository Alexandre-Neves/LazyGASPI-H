#include "lazygaspi_h.h"
#include "utils.h"
#include "gaspi_utils.h"

#include <cstring>

#define WAIT_TIMEOUT 100

gaspi_return_t lazygaspi_read(lazygaspi_id_t row_id, lazygaspi_id_t table_id, lazygaspi_slack_t slack, void* row, 
                              LazyGaspiRowData* outData){
    LazyGaspiProcessInfo* info;
    auto r = lazygaspi_get_info(&info); ERROR_CHECK;

    auto data = (LazyGaspiRowData*)((char*)info->rows + get_row_offset(info, row_id, table_id));
    Notification notif;
    const auto min = get_min_age(info->age, slack);

    PRINT_DEBUG("Reading row (" << row_id << ", " << table_id << ") with minimum age " << min);

    while(data->age < min){
        PRINT_DEBUG("Age below minimum. Waiting for notification...");
        
        r = get_notification(SEGMENT_ID_ROWS, NOTIF_ID_ROW_WRITTEN, 1, &notif, WAIT_TIMEOUT); 
        if(r == GASPI_TIMEOUT) continue;
        ERROR_CHECK;
    }

    PRINT_DEBUG("Age fresh enough. Outputting row...");

    memcpy(row, (char*)data + sizeof(LazyGaspiRowData), info->row_size);
    if(outData) *outData = *data;
    return GASPI_SUCCESS;
}