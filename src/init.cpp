#include "lazygaspi_h.h"
#include "gaspi_utils.h"
#include "utils.h"


gaspi_return_t lazygaspi_init(lazygaspi_id_t table_amount, lazygaspi_id_t table_size, gaspi_size_t row_size, 
                              OutputCreator outputCreator,
                              SizeDeterminer det_amount, void* data_amount, 
                              SizeDeterminer det_tablesize, void* data_tablesize, 
                              SizeDeterminer det_rowsize, void* data_rowsize){

    #ifdef WITH_MPI
    int mpi_rank, mpi_rank_amount;
    {
        int provided;
        auto ret = MPI_Init_thread(0, 0, MPI_THREAD_SERIALIZED, &provided); ERROR_MPI_CHECK_COUT("Failed to init");
        if(provided != MPI_THREAD_SERIALIZED) ERROR_MPI_CHECK_COUT("Tried to initialize with " << (int)MPI_THREAD_SERIALIZED 
                                                                  << " but " << provided << " was provided.");
        ret = MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank); ERROR_MPI_CHECK_COUT("Failed to get rank");
        ret = MPI_Comm_size(MPI_COMM_WORLD, &mpi_rank_amount); ERROR_MPI_CHECK_COUT("Failed to get rank amount");
        PRINT_DEBUG_INTERNAL_COUT("MPI determined this to be rank " << mpi_rank << ", with " << mpi_rank_amount << " total.");
        ret = MPI_Comm_set_errhandler(MPI_COMM_WORLD, MPI_ERRORS_RETURN); ERROR_MPI_CHECK_COUT("Failed to set error handler.");
    }
    #endif

    PRINT_DEBUG_INTERNAL_COUT("Initializing LazyGASPI...");

    auto r = gaspi_proc_init(GASPI_BLOCK); ERROR_CHECK_COUT;

    LazyGaspiProcessInfo* info;
    r = gaspi_malloc_noblock(SEGMENT_ID_INFO, sizeof(LazyGaspiProcessInfo), &info, GASPI_MEM_INITIALIZED);
    ERROR_CHECK_COUT;

    r = gaspi_proc_rank(&(info->id)); ERROR_CHECK_COUT;
    r = gaspi_proc_num(&(info->n)); ERROR_CHECK_COUT;

    #ifdef WITH_MPI
    if(mpi_rank != info->id) {
        PRINT_ON_ERROR_COUT("MPI and GASPI ranks did not match!");
        return GASPI_ERR_INV_RANK;
    }
    if(mpi_rank_amount != info->n){
        PRINT_ON_ERROR_COUT("MPI and GASPI rank amounts did not match!");
        return GASPI_ERR_INV_RANK;
    }
    #endif

    if(info->n == 0) return GASPI_ERR_INV_NUM;
    if(outputCreator){
        outputCreator(info);
        if(info->out == nullptr){
            PRINT_ON_ERROR_COUT("Output stream was nullptr (not created).");
            return GASPI_ERR_NULLPTR;
        }
    }
    else info->out = &std::cout;

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
    info->offset_slack = true;

    r = gaspi_malloc_noblock(SEGMENT_ID_ROWS, table_amount * table_size * (row_size + sizeof(LazyGaspiRowData)), &(info->rows), 
                             GASPI_MEM_INITIALIZED);
    ERROR_CHECK_COUT;

    return GASPI_BARRIER;
}