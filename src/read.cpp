#include "lazygaspi_h.h"
#include "utils.h"
#include "gaspi_utils.h"

#include <cstring>

gaspi_return_t lazygaspi_read(lazygaspi_id_t row_id, lazygaspi_id_t table_id, lazygaspi_slack_t slack, void* row, 
                              LazyGaspiRowData* outData){
    LazyGaspiProcessInfo* info;
    auto r = lazygaspi_get_info(&info); ERROR_CHECK;

    auto data = (LazyGaspiRowData*)((char*)info->rows + get_row_offset(info, row_id, table_id));
    Notification notif;
    const auto min = get_min_age(info->age, slack, info->offset_slack);

    PRINT_DEBUG_INTERNAL("Reading row (" << row_id << ", " << table_id << ") with minimum age " << min);
    #if defined DEBUG || defined DEBUG_INTERNAL
    if(data->age < min) PRINT_DEBUG_INTERNAL("Age below minimum (" << data->age << "). Waiting for write...");
    #endif

    while(data->age < min);

    PRINT_DEBUG_INTERNAL("Age fresh enough (" << data->age << "). Outputting row...");

    memcpy(row, (char*)data + sizeof(LazyGaspiRowData), info->row_size);
    if(outData) *outData = *data;
    return GASPI_SUCCESS;
}