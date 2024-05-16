#include "programer.hpp"
#include <cassert>
#include <algorithm>
#include "spdlog/sinks/rotating_file_sink.h"
// #include "spdlog/sinks/stdout_color_sinks.h"


using namespace std;

Programer::Programer(std::shared_ptr<spdlog::logger> logger)
{
    this->logger = logger;
    this->logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e][%l] %v");
    this->logger->set_level(spdlog::level::info);
}


Programer::Programer(char *logger_file_name, uint32_t max_file_size, uint32_t max_files, bool rotate_on_open)
{
    if (logger_file_name){
        auto logger = spdlog::get(logger_file_name);
        if (! logger){
            logger = spdlog::rotating_logger_mt(logger_file_name, logger_file_name, max_file_size, max_files, rotate_on_open);
        }
        this->logger = logger;
    }
    else{
        // this->logger = spdlog::stdout_color_mt("console");
        this->logger = spdlog::default_logger();
    }
    this->logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e][%l] %v");
    this->logger->set_level(spdlog::level::debug);
}


Programer::~Programer(){}

DebuggerExecutingState_t Programer::program_only(char* file_name, uint32_t addr, uint32_t size, uint32_t file_offset)
{
    assert(file_name);

    DebuggerExecutingState_t stat = DebuggerExecutingNormal;

    ifstream file;
    file.open(file_name, ios::binary|ios::in);
    if (file.fail() || (!(file.is_open()))){
        this->logger->error("file {} open fail, line {:d}.", file_name,  __LINE__);
        return DebuggerExecutingFileOperationError;
    }
    // check file size
    file.seekg(0, ios::end);
    ios::pos_type local = file.tellg();
    uint32_t file_size = (uint32_t) local;
    this->logger->info("file {} size {:d} Bytes", file_name, file_size);
    if ((file_offset > file_size) || (file_size - file_offset < size)){
        file.close();
        this->logger->error("file offset out of range, file size {:d}, max offset 0x{:x}, line {:d}.", file_size, file_offset + size, __LINE__);
        return DebuggerExecutingFirmwareFileNotSuitable;
    }

    // write data
    uint32_t finish_size = 0;
    uint32_t tmp_size = 0;
    file.seekg(file_offset, ios::beg);
    while (finish_size < size)
    {
        tmp_size = (size - finish_size < ALIGN_SIZE_B)? (size - finish_size): ALIGN_SIZE_B;
        file.read((char*)this->buff, tmp_size);
        stat = this->write(addr + finish_size, this->buff, tmp_size);
        if (DebuggerExecutingNormal != stat){
            file.close();
            this->logger->error("Programer addr 0x{:08x} size {:d} Bytes fail!!! line {:d}", addr + finish_size, tmp_size,  __LINE__);
            return stat;
        }
        finish_size += tmp_size;
        this->logger->info("Programer addr 0x{:08x} size {:d} Bytes, finish size {:d}. line {:d}", addr + finish_size, tmp_size, finish_size,  __LINE__);

    }

    file.close();
    return stat;
}


DebuggerExecutingState_t Programer::dump(char* file_name, uint32_t addr, uint32_t size)
{
    assert(file_name);

    // string file_name(file_name);

    DebuggerExecutingState_t stat;

    ofstream file;
    file.open(string(file_name) + ".readback", ios::binary|ios::out);
    if (file.fail()){
        this->logger->error("file {} open fail, line {:d}.", file_name,  __LINE__);
        return DebuggerExecutingFileOperationError;
    }

    uint32_t finish_size = 0;
    uint32_t tmp_size;
    while (finish_size < size)
    {
        tmp_size = (size - finish_size < ALIGN_SIZE_B)? (size - finish_size): ALIGN_SIZE_B;
        stat = this->read(addr + finish_size, this->buff, tmp_size);
        if (DebuggerExecutingNormal != stat){
            file.close();
            this->logger->error("Dump addr 0x{:08x} size {:d} Bytes fail!!! line {:d}", addr + finish_size, tmp_size,  __LINE__);
            return stat;
        }
        file.write((char*)this->buff, tmp_size);
        finish_size += tmp_size;
        this->logger->info("Dump addr 0x{:08x} size {:d} Bytes, finish size {:d}. line {:d}", addr + finish_size, tmp_size, finish_size,  __LINE__);

    }
    file.close();
    return DebuggerExecutingNormal;
}


DebuggerExecutingState_t Programer::verify(char* file_name, uint32_t addr, uint32_t size, uint32_t file_offset)
{
    assert(file_name);

    DebuggerExecutingState_t stat;

    ifstream file;
    file.open(file_name, ios::binary|ios::in);
    if (file.fail()){
        this->logger->error("file {} open fail, line {:d}.", file_name,  __LINE__);
        return DebuggerExecutingFileOperationError;
    }
    // check file size
    file.seekg(0, ios::end);
    ios::pos_type local = file.tellg();
    uint32_t file_size = (uint32_t) local;
    this->logger->info("file {} size {:d} Bytes", file_name, file_size);
    if ((file_offset > file_size) || (file_size - file_offset < size)){
        this->logger->error("file offset out of range, file size {:d}, max offset {:d}, line {:d}.", file_size, file_offset + size, __LINE__);
        return DebuggerExecutingFirmwareFileNotSuitable;
    }

    // verify data
    uint32_t finish_size = 0;
    uint32_t tmp_size = 0;
    file.seekg(file_offset, ios::beg);
    while (finish_size < size)
    {
        tmp_size = (size - finish_size < ALIGN_SIZE_B)? (size - finish_size): ALIGN_SIZE_B;
        file.read((char*)this->buff, tmp_size);
        stat = this->read(addr + finish_size, this->verify_buff, tmp_size);
        if (DebuggerExecutingNormal != stat){
            file.close();
            this->logger->error("Verify read addr 0x{:08x} size {:d} Bytes fail!!! line {:d}", addr + finish_size, tmp_size,  __LINE__);
            return stat;
        }
        if (! equal(this->buff, this->buff + tmp_size, this->verify_buff))
        {
            file.close();
            this->logger->error("Verify addr 0x{:08x} size {:d} Bytes equal fail!!! line {:d}", addr + finish_size, tmp_size,  __LINE__);
            return DebuggerExecutingProgrammingCheckoutError;
        }
        finish_size += tmp_size;
        this->logger->info("Verify addr 0x{:08x} size {:d} Bytes, finish size {:d}. line {:d}", addr + finish_size, tmp_size, finish_size,  __LINE__);

    }
    file.close();
    return DebuggerExecutingNormal;
}

