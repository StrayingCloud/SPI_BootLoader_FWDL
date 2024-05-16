#ifndef __STM32XXX_HPP__
#define __STM32XXX_HPP__

#include "interface/SpiInterface.h"
#include "programer.hpp"


#define SPI_FREQENCY_HZ         8000000
#define RESET_DELAY_MS          300
#define WAIT_ACK_TIMEOUT_MS     500
#define ERASE_ACK_TIMEOUT_MS    10000  // 256 pages need 5.7s
#define BL_MAX_FRAME_SIZE_B     256
#define BUFFER_SIZE_B           (BL_MAX_FRAME_SIZE_B + 4)  // 4 Align, write max: 258 (Number + 256*Data + checksum), read max: 257(DUMMY + 256*Data)
#define WRITE_MEM_RETRY_COUNT   10


class STM32XXX: public Programer
{
public:
    STM32XXX(char* spi_name, uint32_t frequency_hz=SPI_FREQENCY_HZ, SpiCpolnCphaMode_t mode=SpiCpol0nCpha0Mode,
             char *logger_file_name=NULL, uint32_t max_file_size=MAX_FILE_SIZE, uint32_t max_files=MAX_FILES, bool rotate_on_open=false);
    ~STM32XXX();

    DebuggerExecutingState_t init();
    DebuggerExecutingState_t get_command_cmd(uint8_t data[], int& length);
    DebuggerExecutingState_t get_version_cmd(uint8_t& version);
    DebuggerExecutingState_t get_id_cmd(uint8_t id[], int& length);
    DebuggerExecutingState_t read_memory_cmd(uint32_t addr, uint8_t data[], int size);
    DebuggerExecutingState_t write_memory_cmd(uint32_t addr, const uint8_t* data, int length);
    DebuggerExecutingState_t erase_memory_cmd(uint32_t start_page, uint32_t page_num);
    DebuggerExecutingState_t go_cmd(uint32_t addr);
    DebuggerExecutingState_t write_protect_cmd(uint32_t start_page, uint32_t page_num);
    DebuggerExecutingState_t write_unprotect_cmd();
    DebuggerExecutingState_t readout_protect_cmd();
    DebuggerExecutingState_t readout_unprotect_cmd();

    DebuggerExecutingState_t mass_erase();
    DebuggerExecutingState_t erase4k(uint32_t page);
    DebuggerExecutingState_t read(uint32_t addr, uint8_t* data, uint32_t size);
    DebuggerExecutingState_t write(uint32_t addr, const uint8_t* data, uint32_t size);

protected:
    DebuggerExecutingState_t is_ack_ok(uint32_t timeout_ms=WAIT_ACK_TIMEOUT_MS);

private:
    DebuggerExecutingState_t spi_transmit(uint8_t anWriteList[], uint8_t anReadList[], uint32_t nTransmitLength);
    DebuggerExecutingState_t send_command_frame(uint8_t command);
    DebuggerExecutingState_t send_data_frame(uint8_t data[], int length);
    DebuggerExecutingState_t receive_data_frame(int length);
    DebuggerExecutingState_t send_addr_frame(uint32_t addr);
    DebuggerExecutingState_t send_size_frame(int size);

private:
    SpiInterface* spi;
    uint8_t buffer[BUFFER_SIZE_B];
};

#endif