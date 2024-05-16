#ifndef __Programer_HPP__
#define __Programer_HPP__

#include "DbgrCommon.h"
#include <fstream>
#include <iostream>


#define ALIGN_SIZE_B    (4 * 1024)
#define MAX_FILE_SIZE   (20 * 1024 * 1024)
#define MAX_FILES       (2)

class Programer
{
public:
    Programer(std::shared_ptr<spdlog::logger> logger=spdlog::default_logger());
    Programer(char *logger_file_name=NULL, uint32_t max_file_size=MAX_FILE_SIZE, uint32_t max_files=MAX_FILES, bool rotate_on_open=false);
    ~Programer();

    DebuggerExecutingState_t program_only(char* file_name, uint32_t addr, uint32_t size, uint32_t file_offset=0);
    DebuggerExecutingState_t dump(char* file_name, uint32_t addr, uint32_t size);
    DebuggerExecutingState_t verify(char* file_name, uint32_t addr, uint32_t size, uint32_t file_offset=0);
    virtual DebuggerExecutingState_t mass_erase() = 0;

    virtual DebuggerExecutingState_t erase4k(uint32_t page) = 0;
    virtual DebuggerExecutingState_t read(uint32_t addr, uint8_t* data, uint32_t size) = 0;
    virtual DebuggerExecutingState_t write(uint32_t addr, const uint8_t* data, uint32_t size) = 0;

    std::shared_ptr<spdlog::logger> logger=NULL;
private:
    uint8_t buff[ALIGN_SIZE_B]={0xff};
    uint8_t verify_buff[ALIGN_SIZE_B]={0xff};
    
};

#endif