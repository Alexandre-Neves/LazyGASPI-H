#include "lazygaspi_h.h"
#include "utils.h"

gaspi_return_t lazygaspi_get_info(LazyGaspiProcessInfo** info){
    gaspi_pointer_t ptr;
    auto r = gaspi_segment_ptr(SEGMENT_ID_INFO, &ptr);
    *info = (LazyGaspiProcessInfo*)ptr;
    return r;
}

gaspi_return_t lazygaspi_clock(){
    LazyGaspiProcessInfo* info;
    auto r = lazygaspi_get_info(&info); ERROR_CHECK;
    info->age++;

    PRINT_DEBUG("Incremented age to " << info->age);

    return GASPI_SUCCESS;
}

gaspi_return_t lazygaspi_term(){
    LazyGaspiProcessInfo* info;
    auto r = lazygaspi_get_info(&info); ERROR_CHECK;

    r = gaspi_wait(0, GASPI_BLOCK); ERROR_CHECK;

    r = GASPI_BARRIER; ERROR_CHECK;

    return gaspi_proc_term(GASPI_BLOCK);
}