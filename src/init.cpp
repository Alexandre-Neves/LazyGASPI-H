#include "lazygaspi_h.h"
#include "gaspi_utils.h"
#include "utils.h"


gaspi_return_t lazygaspi_init(lazygaspi_id_t table_size, lazygaspi_id_t table_amount, gaspi_size_t row_size, bool output){

    auto r = gaspi_proc_init(GASPI_BLOCK); ERROR_CHECK_STREAM(std::cout);

    LazyGaspiProcessInfo* info;
    r = gaspi_malloc_noblock(SEGMENT_ID_INFO, sizeof(LazyGaspiProcessInfo), &info);
    ERROR_CHECK_STREAM(std::cout);

    r = gaspi_proc_rank(&(info->id)); ERROR_CHECK_STREAM(std::cout);
    r = gaspi_proc_num(&(info->n)); ERROR_CHECK_STREAM(std::cout);

    info->age = 0;
    info->table_size = table_size;
    info->table_amount = table_amount;
    info->row_size = row_size;

    if(output){
        r = gaspi_setup_output("lazygaspi_h", info->id, &info->out); 
        ERROR_CHECK_STREAM(std::cout); 
    }
    else info->out = nullptr;

    return gaspi_malloc_noblock(SEGMENT_ID_ROWS, table_amount * table_size * (row_size + sizeof(LazyGaspiRowData)), &(info->rows), 
                             GASPI_MEM_INITIALIZED);
}