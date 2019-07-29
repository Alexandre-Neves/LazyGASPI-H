#include "lazygaspi_h.h"
#include "gaspi_utils.h"
#include "utils.h"


gaspi_return_t lazygaspi_init(lazygaspi_id_t table_amount, lazygaspi_id_t table_size, gaspi_size_t row_size, 
                              OutputCreator outputCreator,
                              SizeDeterminer det_amount, void* data_amount, 
                              SizeDeterminer det_tablesize, void* data_tablesize, 
                              SizeDeterminer det_rowsize, void* data_rowsize){

    PRINT_DEBUG_INTERNAL_OUT(&std::cout, "Initializing LazyGASPI...");

    auto r = gaspi_proc_init(GASPI_BLOCK); ERROR_CHECK_OUT(&std::cout);

    LazyGaspiProcessInfo* info;
    r = gaspi_malloc_noblock(SEGMENT_ID_INFO, sizeof(LazyGaspiProcessInfo), &info);
    ERROR_CHECK_OUT(&std::cout);

    r = gaspi_proc_rank(&(info->id)); ERROR_CHECK_OUT(&std::cout);
    r = gaspi_proc_num(&(info->n)); ERROR_CHECK_OUT(&std::cout);

    if(info->n == 0) return GASPI_ERR_INV_NUM;
    if(outputCreator) outputCreator(info);
    else info->out = nullptr;

    PRINT_DEBUG_INTERNAL("Allocated info segment for rank " << info->id << ". Total amount of ranks: " << info->n);

    if(table_amount == 0) { 
        if(!det_amount) return GASPI_ERR_INV_NUM;
        if(!(table_amount = det_amount(info->id, info->n, data_amount))) return GASPI_ERR_INV_NUM;
    }
    if(table_size == 0) { 
        if(!det_tablesize) return GASPI_ERR_INV_NUM;
        if(!(table_size = det_tablesize(info->id, info->n, data_tablesize))) return GASPI_ERR_INV_NUM;
    }
    if(row_size == 0) { 
        if(!det_rowsize) return GASPI_ERR_INV_NUM;
        if(!(row_size = det_rowsize(info->id, info->n, data_rowsize))) return GASPI_ERR_INV_NUM;
    }

    PRINT_DEBUG_INTERNAL("Table amount: " << table_amount << " | Table size: " << table_size << " | Row size: " << row_size);

    info->age = 0;
    info->table_size = table_size;
    info->table_amount = table_amount;
    info->row_size = row_size;

    r = gaspi_malloc_noblock(SEGMENT_ID_ROWS, table_amount * table_size * (row_size + sizeof(LazyGaspiRowData)), &(info->rows), 
                             GASPI_MEM_INITIALIZED);
    return GASPI_BARRIER;
}