#include <chrono>
#include <cassert>
#include <algorithm>

#include "stm32xxx.hpp"


/* Bootloader command set */
#define GET_CMD_COMMAND        0x00U  /*!< Get CMD command               */
#define GET_VER_COMMAND        0x01U  /*!< Get Version command           */
#define GET_ID_COMMAND         0x02U  /*!< Get ID command                */
#define RMEM_COMMAND           0x11U  /*!< Read Memory command           */
#define GO_COMMAND             0x21U  /*!< Go command                    */
#define WMEM_COMMAND           0x31U  /*!< Write Memory command          */
#define EMEM_COMMAND           0x44U  /*!< Erase Memory command          */
#define WP_COMMAND             0x63U  /*!< Write Protect command         */
#define WU_COMMAND             0x73U  /*!< Write Unprotect command       */
#define RP_COMMAND             0x82U  /*!< Readout Protect command       */
#define RU_COMMAND             0x92U  /*!< Readout Unprotect command     */

#define BL_SPI_SOF              0x5AU
#define BL_ACK                  0x79U
#define BL_NAK                  0x1FU
#define BL_DUMMY                0xA5U
#define BL_IDLE                 0x00U
#define BL_PROCESSING_DELAY_MS  100


STM32XXX::STM32XXX(char* spi_name, uint32_t frequency_hz, SpiCpolnCphaMode_t mode,
                   char *logger_file_name, uint32_t max_file_size, uint32_t max_files, bool rotate_on_open
                   ): Programer(logger_file_name, max_file_size, max_files, rotate_on_open)
{
    this->logger->info("......... STM32XXX .......");
    this->spi = new SpiInterface(spi_name, this->logger);
    this->spi->ModuleEnable();
    this->spi->SetFrequency(frequency_hz);
    this->spi->SwitchSpiMode(mode);

    std::memset(this->buffer, BL_IDLE, sizeof(this->buffer));

}


STM32XXX::~STM32XXX()
{
    if (this->spi){
        delete this->spi;
    }
}


uint8_t complement(uint8_t data)
{
    return data ^ 0xff;
}


uint8_t xor_checksum(uint8_t data[], int length)
{
    uint8_t checksum = 0x00;

    for (int i=0; i<length; i++){
        checksum ^= data[i];
    }
    return checksum;
}


DebuggerExecutingState_t STM32XXX::spi_transmit(uint8_t anWriteList[], uint8_t anReadList[], uint32_t nTransmitLength)
{
    return this->spi->sTransmit(anWriteList, anReadList, nTransmitLength);
}


DebuggerExecutingState_t STM32XXX::is_ack_ok(uint32_t timeout_ms)
{
    uint32_t elapsed = 0;
    uint8_t resp = 0;

    std::memset(this->buffer, BL_IDLE, 4);

    this->buffer[0] = BL_IDLE;
    this->spi_transmit(this->buffer, this->buffer, 1);

    auto start = std::chrono::steady_clock::now();
    do{
        std::memset(this->buffer, BL_IDLE, 4);
        this->buffer[0] = BL_IDLE;
        this->spi_transmit(this->buffer, this->buffer, 1);
        resp = this->buffer[0];
        if (BL_ACK == resp){
            std::memset(this->buffer, BL_IDLE, 4);
            this->buffer[0] = BL_ACK;
            this->spi_transmit(this->buffer, NULL, 1);
            // auto end = std::chrono::steady_clock::now();
            // elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            // this->logger->debug("ACK cost {:d} ms", elapsed);
            return DebuggerExecutingNormal;
        }
        else if (BL_NAK == resp){
            std::memset(this->buffer, BL_IDLE, 4);
            this->buffer[0] = BL_ACK;
            this->spi_transmit(this->buffer, NULL, 1);
            // auto end = std::chrono::steady_clock::now();
            // elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            // this->logger->debug("NACK cost {:d} ms", elapsed);
            return DebuggerExecutingHardwareFault;
        }
        else {
        }
        auto end = std::chrono::steady_clock::now();
        elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        // std::cout<< "elapsed " << elapsed << " ms"<< std::endl;
    }while(elapsed < timeout_ms);
    // std::cout<< "ack timeout " << std::dec << timeout_ms << " ms" << std::endl;
    this->logger->error("ACK wait timeout {:d} ms", timeout_ms);
    return DebuggerExecutingHardwareBusy;

}

DebuggerExecutingState_t STM32XXX::init()
{
    /* Initializes and enters communication mode.
        Note: BOOT0=1 and chip power up, then send init command */

    uint8_t resp=0;

    std::memset(this->buffer, BL_IDLE, 4);
    this->buffer[0] = BL_SPI_SOF;
    this->spi_transmit(this->buffer, this->buffer, 1);
    resp = this->buffer[0];
    if (BL_DUMMY != resp){
        this->logger->error("ACK=0x{:x}, Start SPI Bootloader error!!! Please Check Bootloader activation patterns then Power Up.", resp);
        return DebuggerExecutingDutIllegalOperation;
    }
    return this->is_ack_ok();
}


DebuggerExecutingState_t STM32XXX::send_command_frame(uint8_t command)
{
    /* 
        Command Frame:
                    Write command            ACK
        MOSI: |0x5A|Cmd Code|Cmd XOR|  |0x00|0x00|0x79|
        MISO: | xx | xx     | xx    |  | xx |0x79| xx |
    */
    std::memset(this->buffer, BL_IDLE, 4);
    this->buffer[0] = BL_SPI_SOF;
    this->buffer[1] = command;
    this->buffer[2] = complement(command);

    this->spi_transmit(this->buffer, NULL, 3);
    return this->is_ack_ok();
}


DebuggerExecutingState_t STM32XXX::send_data_frame(uint8_t data[], int length)
{
    /*
    Command Frame:
                    Write DATA            ACK
        MOSI: | D1 | D2 | D3 |  |0x00|0x00|0x79|
        MISO: | xx | xx | xx |  | xx |0x79| xx |
    */
    assert(length <= BL_MAX_FRAME_SIZE_B);
    assert(length > 0);
    assert(data);

    this->spi_transmit(data, NULL, length);
    return this->is_ack_ok();
}


DebuggerExecutingState_t STM32XXX::receive_data_frame(int length)
{
    /*
    Command Frame:
                  Read Data
        MOSI: | xx | xx | xx | ...
        MISO: | xx | D1 | D2 | ...
    */
    std::memset(this->buffer, BL_DUMMY, length);
    return this->spi_transmit(NULL, this->buffer, length);

}


DebuggerExecutingState_t STM32XXX::get_command_cmd(uint8_t data[], int& length)
{
    /*
    Get the version of the protocol and the supported commands.
    Returns as bellow:
        • Byte 1: protocol version (0 < version < 255), example: 0x10 = version 1.0.
        • Byte 2: 0x00 (Get command)
        • Byte 3: 0x01 (Get Version command)
        • Byte 4: 0x02 (Get ID command)
        • Byte 5: 0x11 (Read Memory command)
        • Byte 6: 0x21 (Go command)
        • Byte 7: 0x31 (Write Memory command)
        • Byte 8: 0x44 (Erase command)
        • Byte 9: 0x63 (Write Protect command)
        • Byte 10: 0x73 (Write Unprotect command)
        • Byte 11: 0x82 (Readout Protect command)
        • Byte 12: 0x92 (Readout Unprotect command)
        • Byte 13: 0xA1 (Get Checksum command), available only for version 1.3
    */
    assert(data);

    DebuggerExecutingState_t stat = DebuggerExecutingNormal;

    stat = this->send_command_frame(GET_CMD_COMMAND);
    if (DebuggerExecutingNormal != stat){
        this->logger->error("Send GET_CMD_COMMAND ACK error.");
        return stat;
    }
    this->receive_data_frame(2);
    uint8_t ack = this->buffer[0];
    uint8_t n = this->buffer[1];  // the number of bytes to follow – 1, except current and ACKs
    if (BL_DUMMY != ack){
        this->logger->error("ACK=0x{:x}, GET CMD data ACK error.", ack);
        return DebuggerExecutingHardwareFault;
    }
    if (n >= BL_DUMMY){
        this->logger->warn("Get CMD number 0x{:x} >= 0x{:x} fail!!!, and retry", n, BL_DUMMY);
        stat = this->is_ack_ok();
        if (DebuggerExecutingNormal != stat){
            this->logger->error("ACK error, line {:d}", __LINE__);
            return stat;
        }
        this->receive_data_frame(2);
        n = this->buffer[1];
        if (n >= BL_DUMMY){
            this->logger->error("retry Get CMD number 0x{:x} >= 0x{:x} fail!!!", n, BL_DUMMY);
            return DebuggerExecutingHardwareFault;
        }
    }
    this->receive_data_frame(n + 1);
    length = n + 1;
    std::copy(this->buffer, this->buffer + length, data);
    stat = this->is_ack_ok();
    return stat;
}


DebuggerExecutingState_t STM32XXX::get_version_cmd(uint8_t& version)
{
    /* Get the version of the SPI protocol. */

    DebuggerExecutingState_t stat = DebuggerExecutingNormal;

    stat = this->send_command_frame(GET_VER_COMMAND);
    if (DebuggerExecutingNormal != stat){
        this->logger->error("Send GET_VER_COMMAND ACK error.");
        return stat;
    }
    this->receive_data_frame(2);
    version = this->buffer[1];
    stat = this->is_ack_ok();
    return stat;
}


DebuggerExecutingState_t STM32XXX::get_id_cmd(uint8_t id[], int& length)
{
    /* get the version of the chip ID (identification) */

    assert(id);

    DebuggerExecutingState_t stat;
    stat = this->send_command_frame(GET_ID_COMMAND);
    if (DebuggerExecutingNormal != stat){
        this->logger->error("Send GET_ID_COMMAND ACK error.");
        return stat;
    }

    this->receive_data_frame(2);
    uint8_t ack = this->buffer[0];
    uint8_t n = this->buffer[1];  // the number of bytes to follow – 1, except current and ACKs
    if (BL_DUMMY != ack){
        this->logger->error("ACK=0x{:x}, GET ID data ACK error.", ack);
        return DebuggerExecutingHardwareFault;
    }
    if (n >= BL_DUMMY){
        this->logger->warn("Get ID number 0x{:x} >= 0x{:x} fail!!!, and retry", n, BL_DUMMY);
        stat = this->is_ack_ok();
        if (DebuggerExecutingNormal != stat){
            this->logger->error("ACK error, line {:d}", __LINE__);
            return stat;
        }
        this->receive_data_frame(2);
        n = this->buffer[1];
        if (n >= BL_DUMMY){
            this->logger->error("retry Get ID number 0x{:x} >= 0x{:x} fail!!!", n, BL_DUMMY);
            return DebuggerExecutingHardwareFault;
        }
    }
    this->receive_data_frame(n + 1);
    length = n + 1;
    std::copy(this->buffer, this->buffer + length, id);

    stat = this->is_ack_ok();
    if (DebuggerExecutingNormal != stat){
        this->logger->error("ACK error, line {:d}", __LINE__);
        return stat;
    }
    return DebuggerExecutingNormal;
}


DebuggerExecutingState_t STM32XXX::send_addr_frame(uint32_t addr)
{
    // address 4 bytes, byte 1 being the MSB, and byte 4 being the LSB
    std::memset(this->buffer, BL_IDLE, 8);
    this->buffer[0] = ((addr >> 24) & 0xff);
    this->buffer[1] = ((addr >> 16) & 0xff);
    this->buffer[2] = ((addr >> 8) & 0xff);
    this->buffer[3] = ((addr >> 0) & 0xff);
    this->buffer[4] = xor_checksum(this->buffer, 4);
    return this->send_data_frame(this->buffer, 5);
}


DebuggerExecutingState_t STM32XXX::send_size_frame(int size)
{
    assert(size > 0);
    assert(size <= BL_MAX_FRAME_SIZE_B);

    uint8_t _size = (uint8_t)(size - 1);

    std::memset(this->buffer, BL_IDLE, 4);
    this->buffer[0] = _size;
    this->buffer[1] = complement(_size);
    return this->send_data_frame(this->buffer, 2);
}


DebuggerExecutingState_t STM32XXX::read_memory_cmd(uint32_t addr, uint8_t data[], int size)
{
    /*
    Read data from any valid memory address in the RAM, flash memory,
    and information block (system memory or option byte areas)\
    */
    assert(data);
    // assert(size > 0);
    // assert(size <= BL_MAX_FRAME_SIZE_B);


    if (! ((size > 0) && (size <= BL_MAX_FRAME_SIZE_B))){
        this->logger->error("param error.");
        return DebuggerExecutingParamFault;
    }

    DebuggerExecutingState_t stat;
    stat = this->send_command_frame(RMEM_COMMAND);
    if 
        (DebuggerExecutingNormal != stat){
        this->logger->error("Send RMEM_COMMAND ACK error or Protection active.");
        return stat;
    }
    stat = this->send_addr_frame(addr);
    if (DebuggerExecutingNormal != stat){
        this->logger->error("Send addr ACK error, line {:d}.", __LINE__);
        return stat;
    }
    stat = this->send_size_frame(size);
    if (DebuggerExecutingNormal != stat){
        this->logger->error("Send size ACK error, line {:d}.", __LINE__);
        return stat;
    }

    this->receive_data_frame(size + 1);  // the first one is BL_DUMMY byte.
    std::copy(this->buffer + 1, this->buffer + size + 1, data);
    return DebuggerExecutingNormal;
}


DebuggerExecutingState_t STM32XXX::write_memory_cmd(uint32_t addr, const uint8_t* data, int size)
{
    /*
    write data to any valid address of the RAM, flash memory, or option byte area.

        Write operations to the flash memory must be word (16-bit) aligned and data must be in
        multiples of two bytes. If less data are written, the remaining bytes have to be filled by 0xFF.

        If the Write Memory command is issued to the Option byte area, all options are erased
        before writing the new values, and at the end of the command the bootloader generates a
        system reset to take into account the new configuration of the option byte.

        If the write destination is the flash memory, the master must wait enough time for the sent
        buffer to be written (refer to product datasheet for timing values) before polling for a slave
        response.

        No error is returned when performing write operations in write-protected sectors.
    */
    assert(data);
    // assert(size > 0);
    // assert(size <= BL_MAX_FRAME_SIZE_B);

    DebuggerExecutingState_t stat;


    if (! ((size > 0) && (size <= BL_MAX_FRAME_SIZE_B))){
        this->logger->error("param error.");
        return DebuggerExecutingParamFault;
    }

    stat = this->send_command_frame(WMEM_COMMAND);
    if (DebuggerExecutingNormal != stat){
        this->logger->error("Send WMEM_COMMAND ACK error or Protection active.");
        return stat;
    }

    stat = this->send_addr_frame(addr);
    if (DebuggerExecutingNormal != stat){
        this->logger->error("Send addr ACK error, line {:d}.", __LINE__);
        return stat;
    }
    // send data
    std::memset(this->buffer, BL_IDLE, sizeof(this->buffer));
    this->buffer[0] = size - 1;
    std::copy(data, data + size, this->buffer + 1);

    // word (16-bit) aligned
    if (size % 2){
        this->buffer[0] = size;
        this->buffer[size + 1] = 0xff;
        size += 1;
    }

    this->buffer[size + 1] = xor_checksum(this->buffer, size + 1);
    stat = this->send_data_frame(this->buffer, size + 2);  // Number + size + checksum
    if (DebuggerExecutingNormal != stat){
        this->logger->error("Send data ACK error, line {:d}.", __LINE__);
        return stat;
    }
    return DebuggerExecutingNormal;

}
DebuggerExecutingState_t STM32XXX::erase_memory_cmd(uint32_t start_page, uint32_t page_num)
{
    /*
    For page_num-1 = 0xFFFY (where Y is from 0 to F) a special erase is performed.
        – 0xFFFF for a global mass erase.
        – 0xFFFE for bank 1 mass erase (only for products supporting this feature).
        – 0xFFFD for bank 2 mass erase (only for products supporting this feature).
        – Values from 0xFFFC to 0xFFF0 are reserved

    No error is returned when performing erase operations on write-protected sectors. The
    maximum number of pages or sectors is product-related, and must be respected.
    */
    // assert(start_page >= 0);
    // assert(start_page <= 0xff00);
    // assert(page_num > 0);
    // assert(page_num <= 0x10000);

    if (! ((start_page >= 0) && (start_page <= 0xff00))){
        this->logger->error("param start_page error.");
        return DebuggerExecutingParamFault;
    }

    if (! ((page_num > 0) && (page_num <= 0x10000))){
        this->logger->error("param page_num error.");
        return DebuggerExecutingParamFault;
    }

    uint32_t num = page_num - 1;
    if (num + start_page > 0xffff){
        this->logger->error("param error max page offset {:d} out of range 0x10000, line {:d}.", page_num + start_page,  __LINE__);
        return DebuggerExecutingParamFault;
    }

    DebuggerExecutingState_t stat;
    stat = this->send_command_frame(EMEM_COMMAND);
    if (DebuggerExecutingNormal != stat){
        this->logger->error("Send EMEM_COMMAND ACK error or Protection active.");
        return stat;
    }

    // Send data frame: start sector(2 Bytes), the number of pages or sectors to be erased + checksum (1 Byte)
    std::memset(this->buffer, BL_IDLE, 4);
    this->buffer[0] = (num >> 8) & 0xff;
    this->buffer[1] = num & 0xff;
    this->buffer[2] = xor_checksum(this->buffer, 2);
    stat = this->send_data_frame(this->buffer, 3);
    if (DebuggerExecutingNormal != stat){
        this->logger->error("Send data ACK error, line {:d}.", __LINE__);
        return stat;
    }

    // If non-special erase, send sector size (2 Bytes) + checksum (1 Byte)
    if ((num & 0xfff0) != 0xfff0){
        uint8_t checksum = 0x00;
        int buff_index = 0;
        std::memset(this->buffer, BL_IDLE, sizeof(this->buffer));
        for (int i=start_page; i < (start_page + num); ++i)
        {
            if (buff_index < (BL_MAX_FRAME_SIZE_B)){
                this->buffer[buff_index] = (i >> 8) & 0xff;
                this->buffer[buff_index + 1] = i & 0xff;
                buff_index += 2;
            }
            else {
                this->spi_transmit(this->buffer, NULL, buff_index);
                checksum ^= xor_checksum(this->buffer, buff_index);
                buff_index = 0;
                std::memset(this->buffer, BL_IDLE, sizeof(this->buffer));
            }
        }
        if (buff_index > 0){
            this->spi_transmit(this->buffer, NULL, buff_index);
            checksum ^= xor_checksum(this->buffer, buff_index);
        }
        std::memset(this->buffer, BL_IDLE, 4);
        this->buffer[0] = checksum;
        this->spi_transmit(this->buffer, NULL, 1);
        stat = this->is_ack_ok(ERASE_ACK_TIMEOUT_MS);  // 256 pages need 5.7s
        if (DebuggerExecutingNormal != stat){
            this->logger->error("Erase Send data ACK error {:d}, line {:d}.", int(stat), __LINE__);
            return stat;
        }
    }
    return DebuggerExecutingNormal;
}


DebuggerExecutingState_t STM32XXX::go_cmd(uint32_t addr)
{
    /*
    Execute the downloaded code or any other code by branching to an address specified by the application.
    */
    DebuggerExecutingState_t stat;

    stat = this->send_command_frame(GO_COMMAND);
    if (DebuggerExecutingNormal != stat){
        this->logger->error("Send GO_COMMAND ACK error or Protection active.");
        return stat;
    }
    stat = this->send_addr_frame(addr);
    if (DebuggerExecutingNormal != stat){
        this->logger->error("Send addr ACK error, line {:d}.", __LINE__);
        return stat;
    }
    return DebuggerExecutingNormal; 
}


DebuggerExecutingState_t STM32XXX::write_protect_cmd(uint32_t start_page, uint32_t page_num)
{
    /*
    Enable the write protection of the flash memory pages.
    System reset is called after ACK ok, only for some STM32 BL (STM32F4/F7) and some STM32L4.
    */
    if (! ((start_page >= 0) && (start_page < 256))){
        this->logger->error("param start_page error.");
        return DebuggerExecutingParamFault;
    }

    if (! ((page_num > 0) && (page_num <= 256))){
        this->logger->error("param page_num error.");
        return DebuggerExecutingParamFault;
    }

    uint32_t num = page_num - 1;
    DebuggerExecutingState_t stat;

    stat = this->send_command_frame(WP_COMMAND);
    if (DebuggerExecutingNormal != stat){
        this->logger->error("Send WP_COMMAND ACK error or Protection active.");
        return stat;
    }

    std::memset(this->buffer, BL_IDLE, 4);
    this->buffer[0] = num;
    this->buffer[1] = complement(num);
    this->buffer[2] = xor_checksum(this->buffer, 2);
    stat = this->send_data_frame(this->buffer, 3);
    if (DebuggerExecutingNormal != stat){
        this->logger->error("Write protect num checksum fail!!!, line {:d}.", __LINE__);
        return stat;
    }

    std::memset(this->buffer, BL_IDLE, 4);
    this->buffer[0] = start_page;
    this->buffer[1] = start_page + num;
    this->buffer[2] = xor_checksum(this->buffer, 2);
    stat = this->send_data_frame(this->buffer, 3);
    if (DebuggerExecutingNormal != stat){
        this->logger->error("Write protect data checksum fail!!!, line {:d}.", __LINE__);
        return stat;
    }
    DbgrDelayMs(RESET_DELAY_MS);
    this->init();
    return DebuggerExecutingNormal;
}


DebuggerExecutingState_t STM32XXX::write_unprotect_cmd()
{
    /*
    Disable the write protection of all the flash memory pages.
    System reset is called after ACK ok, only for some STM32 BL (STM32F4/F7) and some STM32L4
    */
    DebuggerExecutingState_t stat;

    stat = this->send_command_frame(WU_COMMAND);
    if (DebuggerExecutingNormal != stat){
        this->logger->error("Send WU_COMMAND ACK error or Protection active.");
        return stat;
    }
    DbgrDelayMs(BL_PROCESSING_DELAY_MS);

    std::memset(this->buffer, BL_IDLE, 4);
    this->buffer[0] = BL_IDLE;
    stat = this->send_data_frame(this->buffer, 1);
    if (DebuggerExecutingNormal != stat){
        this->logger->error("Remove write protection for the whole flash memory fail!!!, line {:d}.", __LINE__);
        return stat;
    }

    DbgrDelayMs(RESET_DELAY_MS);
    this->init();
    return DebuggerExecutingNormal;
}


DebuggerExecutingState_t STM32XXX::readout_protect_cmd()
{
    /*
    Enable the flash memory read protection.
    System reset is called after ACK ok, only for some STM32 BL (STM32F4/F7) and some STM32L4
    */
    DebuggerExecutingState_t stat;

    stat = this->send_command_frame(RP_COMMAND);
    if (DebuggerExecutingNormal != stat){
        this->logger->error("Send RP_COMMAND ACK error or RDP active.");
        return stat;
    }
    DbgrDelayMs(BL_PROCESSING_DELAY_MS);

    stat = this->is_ack_ok();
    if (DebuggerExecutingNormal != stat){
        this->logger->error("Activate read protection for the whole flash memory fail!!! {:d}, line {:d}.", int(stat), __LINE__);
        return stat;
    }
    DbgrDelayMs(RESET_DELAY_MS);
    this->init();
    return DebuggerExecutingNormal;
}


DebuggerExecutingState_t STM32XXX::readout_unprotect_cmd()
{
    /*
    Disable the flash memory read protection, and erase all flash memory.
    System reset is called after ACK ok, only for some STM32 BL (STM32F4/F7) and some STM32L4
    */
    DebuggerExecutingState_t stat;

    stat = this->send_command_frame(RU_COMMAND);
    if (DebuggerExecutingNormal != stat){
        this->logger->error("Send RU_COMMAND ACK error.");
        return stat;
    }
    DbgrDelayMs(BL_PROCESSING_DELAY_MS);

    std::memset(this->buffer, BL_IDLE, 4);
    this->buffer[0] = BL_IDLE;
    stat = this->send_data_frame(this->buffer, 1);
    if (DebuggerExecutingNormal != stat){
        this->logger->error("Activate read unprotection for the whole flash memory fail!!!, line {:d}.", __LINE__);
        return stat;
    }

    DbgrDelayMs(RESET_DELAY_MS);
    this->init();
    return DebuggerExecutingNormal;
}


DebuggerExecutingState_t STM32XXX::write(uint32_t addr, const uint8_t* data, uint32_t size)
{
    assert(data);

    DebuggerExecutingState_t stat;

    uint32_t finish_size = 0;
    uint32_t tmp_size = 0;

    while (finish_size < size)
    {
        tmp_size = (size - finish_size < BL_MAX_FRAME_SIZE_B)? (size - finish_size):BL_MAX_FRAME_SIZE_B;
        stat = this->write_memory_cmd(addr + finish_size, data + finish_size, tmp_size);
        if (DebuggerExecutingNormal != stat){
            this->logger->error(">> write addr 0x{:08x} size {:d} Bytes fail!!! line {:d}", addr + finish_size, tmp_size,  __LINE__);
            return stat;
        }
        // ----- read and verify -----
        /*
        Reference AN2606 for the W2 solution to fix write operation fail limitation. 

        AN2606-stm32-microcontroller-system-memory-boot-mode-stmicroelectronics

        STM32L45xxx/46xxx bootloader versions
        V9.2 Version Known limitations:

            SPI write operation fail limitation:
            a. During Bootloader SPI write flash memory operation, some random 64-bits (2 double-words) may be left blank at 0xFF.
            Root cause:
                a. Bootloader uses 64-bits cast write operation which is interrupted by SPI DMA and it leads to double access on same flash memory address and the 64-bits are not written
            Workarounds:
                a. WA1: add a delay between sending write command and its ACK request. Its duration must be the duration of the 256-Bytes flash memory write time.
                b. WA2: read back after write and in case of error start write again.
                c. WA3: Patch in RAM to write in flash memory that implements write memory without 64-bits

        */
        for (int j=0; j < WRITE_MEM_RETRY_COUNT; ++j)
        {
            stat = this->read_memory_cmd(addr + finish_size, this->buffer, tmp_size);
            if (DebuggerExecutingNormal != stat){
                this->logger->error(">> read addr 0x{:08x} size {:d} Bytes fail!!! line {:d}", addr + finish_size, tmp_size,  __LINE__);
                return stat;
            }
            if (! std::equal(data + finish_size, data + finish_size + tmp_size, this->buffer))
            {
                this->logger->debug(">> verify addr 0x{:08x} size {:d} Bytes equal fail!!! and retry {:d} line {:d}", addr + finish_size, tmp_size, j, __LINE__);
                stat = this->write_memory_cmd(addr + finish_size, data + finish_size, tmp_size);
                if (DebuggerExecutingNormal != stat){
                    this->logger->error(">> retry write addr 0x{:08x} size {:d} Bytes fail!!! line {:d}", addr + finish_size, tmp_size,  __LINE__);
                    return stat;
        }
            }
            else {
                break;
            }
        }
        // ----- read and verify -----
        finish_size += tmp_size;
        this->logger->debug(">> write addr 0x{:08x} size {:d} Bytes, finish size {:d}. line {:d}", addr + finish_size, tmp_size, finish_size,  __LINE__);
    }
    return DebuggerExecutingNormal;
}


DebuggerExecutingState_t STM32XXX::read(uint32_t addr, uint8_t* data, uint32_t size)
{
    assert(data);

    DebuggerExecutingState_t stat;

    uint32_t finish_size = 0;
    uint32_t tmp_size = 0;
    while (finish_size < size)
    {
        tmp_size = (size - finish_size < BL_MAX_FRAME_SIZE_B)? (size - finish_size):BL_MAX_FRAME_SIZE_B;
        stat = this->read_memory_cmd(addr + finish_size, data + finish_size, tmp_size);
        if (DebuggerExecutingNormal != stat){
            this->logger->error(">> read addr 0x{:08x} size {:d} Bytes fail!!! line {:d}", addr + finish_size, tmp_size,  __LINE__);
            return stat;
        }
        finish_size += tmp_size;
        this->logger->debug(">> read addr 0x{:08x} size {:d} Bytes, finish size {:d}. line {:d}", addr + finish_size, tmp_size, finish_size,  __LINE__);
    }
    return DebuggerExecutingNormal;
}


DebuggerExecutingState_t STM32XXX::mass_erase()
{
    DebuggerExecutingState_t stat;

    stat = this->erase_memory_cmd(0, 0x10000);
    if (DebuggerExecutingNormal != stat){
        this->logger->error("mass erase fail!!! line {:d}.", __LINE__);
        return stat;
    }
    return DebuggerExecutingNormal;
}

DebuggerExecutingState_t STM32XXX::erase4k(uint32_t page)
{
    return this->erase_memory_cmd(page, 2);
}