#ifndef __STM32XXX_H__
#define __STM32XXX_H__

#include <cstring>
#include "stm32xxx.hpp"
#include "DbgrCommon.h"


extern "C" {

STM32XXX* new_STM32XXX(char* spi_name, uint32_t frequency_hz, uint32_t mode,
                       char *logger_file_name=NULL, uint32_t max_file_size=MAX_FILE_SIZE, uint32_t max_files=MAX_FILES, bool rotate_on_open=false)
{
    return new STM32XXX(spi_name, frequency_hz, (SpiCpolnCphaMode_t)mode,
                        logger_file_name, max_file_size, max_files, rotate_on_open);
}


STM32XXX* del_STM32XXX(STM32XXX* stm32)
{
    if (stm32){
        delete stm32;
    }
    return 0;
}


int init(STM32XXX* stm32)
{
    if (! stm32){printf("param is NULL pointer!!!\n"); return DebuggerExecutingNullPointerParamFault;}
    return stm32->init();
}


int get_command_cmd(STM32XXX* stm32, uint8_t data[], int& length)
{
    if (! stm32){printf("param is NULL pointer!!!\n"); return DebuggerExecutingNullPointerParamFault;}
    return stm32->get_command_cmd(data, length);
}


int get_version_cmd(STM32XXX* stm32, uint8_t& version)
{
    if (! stm32){printf("param is NULL pointer!!!\n"); return DebuggerExecutingNullPointerParamFault;}
    return stm32->get_version_cmd(version);
}


int get_id_cmd(STM32XXX* stm32, uint8_t id[], int& length)
{
    if (! stm32){printf("param is NULL pointer!!!\n"); return DebuggerExecutingNullPointerParamFault;}
    return stm32->get_id_cmd(id, length);
}


int read_memory_cmd(STM32XXX* stm32, uint32_t addr, uint8_t data[], int size)
{
    if (! stm32){printf("param is NULL pointer!!!\n"); return DebuggerExecutingNullPointerParamFault;}
    return stm32->read_memory_cmd(addr, data, size);
}


int write_memory_cmd(STM32XXX* stm32, uint32_t addr, const uint8_t* data, int length)
{
    if (! stm32){printf("param is NULL pointer!!!\n"); return DebuggerExecutingNullPointerParamFault;}
    return stm32->write_memory_cmd(addr, data, length);
}


int erase_memory_cmd(STM32XXX* stm32, uint32_t start_page, uint32_t page_num)
{
    if (! stm32){printf("param is NULL pointer!!!\n"); return DebuggerExecutingNullPointerParamFault;}
    return stm32->erase_memory_cmd(start_page, page_num);
}


int go_cmd(STM32XXX* stm32, uint32_t addr)
{
    if (! stm32){printf("param is NULL pointer!!!\n"); return DebuggerExecutingNullPointerParamFault;}
    return stm32->go_cmd(addr);
}


int write_protect_cmd(STM32XXX* stm32, uint32_t start_page, uint32_t page_num)
{
    if (! stm32){printf("param is NULL pointer!!!\n"); return DebuggerExecutingNullPointerParamFault;}
    return stm32->write_protect_cmd(start_page, page_num);
}


int write_unprotect_cmd(STM32XXX* stm32)
{
    if (! stm32){printf("param is NULL pointer!!!\n"); return DebuggerExecutingNullPointerParamFault;}
    return stm32->write_unprotect_cmd();
}


int readout_protect_cmd(STM32XXX* stm32)
{
    if (! stm32){printf("param is NULL pointer!!!\n"); return DebuggerExecutingNullPointerParamFault;}
    return stm32->readout_protect_cmd();
}


int readout_unprotect_cmd(STM32XXX* stm32)
{
    if (! stm32){printf("param is NULL pointer!!!\n"); return DebuggerExecutingNullPointerParamFault;}
    return stm32->readout_unprotect_cmd();
}


int mass_erase(STM32XXX* stm32)
{
    if (! stm32){printf("param is NULL pointer!!!\n"); return DebuggerExecutingNullPointerParamFault;}
    return stm32->mass_erase();
}


int program_only(STM32XXX* stm32, char* file_name, uint32_t addr, uint32_t size, uint32_t file_offset)
{
    if (! stm32){printf("param is NULL pointer!!!\n"); return DebuggerExecutingNullPointerParamFault;}
    return stm32->program_only(file_name, addr, size, file_offset);
}


int dump(STM32XXX* stm32, char* file_name, uint32_t addr, uint32_t size)
{
    if (! stm32){printf("param is NULL pointer!!!\n"); return DebuggerExecutingNullPointerParamFault;}
    return stm32->dump(file_name, addr, size);
}


int verify(STM32XXX* stm32, char* file_name, uint32_t addr, uint32_t size, uint32_t file_offset)
{
    if (! stm32){printf("param is NULL pointer!!!\n"); return DebuggerExecutingNullPointerParamFault;}
    return stm32->verify(file_name, addr, size, file_offset);
}


int erase4k(STM32XXX* stm32, uint32_t page)
{
    if (! stm32){printf("param is NULL pointer!!!\n"); return DebuggerExecutingNullPointerParamFault;}
    return stm32->erase4k(page);
}


int read_memory(STM32XXX* stm32, uint32_t addr, uint8_t* data, uint32_t size)
{
    if (! stm32){printf("param is NULL pointer!!!\n"); return DebuggerExecutingNullPointerParamFault;}
    return stm32->read(addr, data, size);
}


int write_memory(STM32XXX* stm32, uint32_t addr, const uint8_t* data, uint32_t size)
{
    if (! stm32){printf("param is NULL pointer!!!\n"); return DebuggerExecutingNullPointerParamFault;}
    return stm32->write(addr, data, size);
}


void debug_state_message(int tDebuggerState, char* reason, uint32_t length){
    std::string mesg = DbgrErrorState((DebuggerExecutingState_t)tDebuggerState);
    memset(reason, '\0', length);
    strcpy(reason, mesg.c_str());
}

}

#endif