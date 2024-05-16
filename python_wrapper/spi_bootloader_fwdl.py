# -*- coding: utf-8 -*-
import os
import ctypes
import hashlib


__author__ = 'StrayingCloud'
__version__ = '0.1'


class SPIBootLoaderFWDLDef:
    SPI_FREQUENCY = 8000000
    SPI_MODE = 0
    MAX_LOG_FILE_SIZE = 20 * 1024 * 1024
    MAX_LOG_FILES = 2


class SPIBootLoaderFWDLException(Exception):
    pass


class SPIBootLoaderFWDL(object):
    '''
    SPI BootLoader for FWDL.

    Args:
        spi_name:       instance(string),           SPI device name
        frequency:      instance(int),              spi clk frequency
        mode:           instance(int), [0~3]        spi mode.
                                                    +---------+---------+----------+
                                                    | Mode    |   CPOL  |   CPHA   |
                                                    +=========+=========+==========+
                                                    |    0    |    0    |     0    |
                                                    +---------+---------+----------+
                                                    |    1    |    0    |     1    |
                                                    +---------+---------+----------+
                                                    |    2    |    1    |     0    |
                                                    +---------+---------+----------+
                                                    |    3    |    1    |     1    |
                                                    +---------+---------+----------+
        libchip:        instance(string),           file path of libchip.so

    Examples:
        spi_name = "/dev/AXI4_QSPI_2"
        sbl_fwdl = SPIBootLoaderFWDL(spi_name, frequency=8000000, mode=0, libchip="/root/lib/libchip.so")

    '''
    rpc_public_api = ['init', 'mass_erase', 'program_only', 'verify', 'dump',
                      'get_protocol_version', 'get_chip_id', 'read', 'write',
                      'write_protect', 'write_unprotect', 'readout_protect', 'readout_unprotect'
                      ]

    def __init__(self, spi_name, frequency=SPIBootLoaderFWDLDef.SPI_FREQUENCY, mode=SPIBootLoaderFWDLDef.SPI_MODE,
                 libchip="/root/lib/libchip.so", log_file="/root/log/spi_bootloader.log"):
        self.libchip = libchip
        self.clib = ctypes.cdll.LoadLibrary(libchip)
        self.spi_name = bytes(spi_name)
        self.frequency = frequency
        self.mode = mode
        self.log_file = bytes(log_file)

        self.open()

    def __del__(self):
        if hasattr(self, "clib"):
            self.close()

    def open(self):
        self._stm32 = self.clib.new_STM32XXX(ctypes.c_char_p(
            self.spi_name), ctypes.c_uint32(self.frequency), ctypes.c_uint32(self.mode),
            ctypes.c_char_p(self.log_file), ctypes.c_uint32(SPIBootLoaderFWDLDef.MAX_LOG_FILE_SIZE),
            ctypes.c_uint32(SPIBootLoaderFWDLDef.MAX_LOG_FILES), ctypes.c_bool(False))
        if self._stm32 == 0:
            raise Exception("Open device failure.")

    def close(self):
        if hasattr(self, "_stm32"):
            self.clib.del_STM32XXX(self._stm32)

    def _get_state_message(self, state):
        reason = (128 * ctypes.c_char)()
        self.clib.debug_state_message(ctypes.c_int32(state), reason, 128)
        return ctypes.string_at(reason, -1).decode('utf-8')

    def init(self):
        '''
        Initializes and enters communication mode.
        '''
        state = self.clib.init(self._stm32)
        if (state == -8):
            raise SPIBootLoaderFWDLException(
                "Start SPI Bootloader error!!! Please Bootloader activation patterns then Power Up.")
        elif (state != -5) and (state < 0):
            raise SPIBootLoaderFWDLException(self._get_state_message(state))
        return 'done'

    def get_protocol_version(self):
        '''
        Get protocol version.

        Examples:
            init()
            get_protocol_version()

        '''
        version = ctypes.c_ubyte(0)
        state = self.clib.get_version_cmd(self._stm32, ctypes.byref(version))
        if state < 0:
            raise SPIBootLoaderFWDLException(self._get_state_message(state))
        return version.value

    def get_chip_id(self):
        '''
        Get chip id.

        Examples:
            init()
            get_chip_id()

        '''
        length = ctypes.c_ubyte(0)
        uid = (ctypes.c_ubyte * 10)()
        state = self.clib.get_id_cmd(self._stm32, uid, ctypes.byref(length))
        if state < 0:
            raise SPIBootLoaderFWDLException(self._get_state_message(state))
        return uid[:length.value]

    def mass_erase(self):
        '''
        mass erase for chip flash.

        Examples:
            init()
            mass_erase()

        '''
        state = self.clib.mass_erase(self._stm32)
        if state < 0:
            raise SPIBootLoaderFWDLException(self._get_state_message(state))

    def program_only(self, file, md5, addr, file_offset=0):
        '''
        Program file datas to chip.

        Args:
            file:   string,     file path.
            md5:    string,     file md5sum
            addr:   int,        chip memory addr
            file_offset:    int,    file data offset to program, default=0

        Examples:
            base_addr = 0x8000000
            file = r'/root/STM32CubeL4_Demo_STM32L452RE_Nucleo-V1.0.0.bin'
            md5 = "20eb868d5d3df4be0e3f1d68342d31a8"

            init()
            mass_erase()
            program_only(file, md5, base_addr)
            verify(file, md5, base_addr)

        '''
        if (not os.path.exists(file)):
            raise NameError('FW file not found')

        if (md5.lower().startswith("0x")):
            md5 = md5[2:]

        if (len(md5) != 32):
            raise NameError('Error in MD5 provided')

        file_md5 = "cccc"   # Just a random init value, in case hashlib.md5.hexdigest() return erronus value.
        with open(file, "rb") as f:
            file_md5 = hashlib.md5(f.read()).hexdigest()

        if (file_md5.lower() != md5.lower()):
            raise NameError('Error, MD5 mismatched.  Programming aborted')

        size = os.path.getsize(file)
        state = self.clib.program_only(self._stm32,
                                       ctypes.c_char_p(file), ctypes.c_uint32(addr),
                                       ctypes.c_uint32(size), ctypes.c_uint32(file_offset))
        if state < 0:
            raise SPIBootLoaderFWDLException(self._get_state_message(state))

        return 'done'

    def verify(self, file, md5, addr, file_offset=0):
        '''
        Verify file datas with chip.

        Args:
            file:   string,     file path.
            md5:    string,     file md5sum
            addr:   int,        chip memory addr
            file_offset:    int,    file data offset to program, default=0

        Examples:
            base_addr = 0x8000000
            file = r'/root/STM32CubeL4_Demo_STM32L452RE_Nucleo-V1.0.0.bin'
            md5 = "20eb868d5d3df4be0e3f1d68342d31a8"

            init()
            mass_erase()
            program_only(file, md5, base_addr)
            verify(file, md5, base_addr)

        '''
        if (not os.path.exists(file)):
            raise NameError('FW file not found')

        if (md5.lower().startswith("0x")):
            md5 = md5[2:]

        if (len(md5) != 32):
            raise NameError('Error in MD5 provided')

        file_md5 = "cccc"   # Just a random init value, in case hashlib.md5.hexdigest() return erronus value.
        with open(file, "rb") as f:
            file_md5 = hashlib.md5(f.read()).hexdigest()

        if (file_md5.lower() != md5.lower()):
            raise NameError('Error, MD5 mismatched.  Programming aborted')

        size = os.path.getsize(file)
        state = self.clib.verify(self._stm32,
                                 ctypes.c_char_p(file), ctypes.c_uint32(addr),
                                 ctypes.c_uint32(size), ctypes.c_uint32(file_offset))
        if state < 0:
            raise SPIBootLoaderFWDLException(self._get_state_message(state))

        return 'done'

    def dump(self, file, addr, size):
        '''
        Dump chip datas to file.

        Args:
            file:   string,     file path to save datas
            addr:   int,        chip memory addr
            size:   int,        data size to dump.

        Examples:
            base_addr = 0x8000000
            file = r'/root/STM32CubeL4_Demo_STM32L452RE_Nucleo-V1.0.0.bin'

            init()
            dump(file, base_addr, os.path.getsize(file))

        '''
        state = self.clib.dump(self._stm32, ctypes.c_char_p(file), ctypes.c_uint32(addr), ctypes.c_uint32(size))
        if state < 0:
            raise SPIBootLoaderFWDLException(self._get_state_message(state))
        return 'done'

    def read(self, addr, size):
        '''
        Read memory.

        Args:
            addr:   int,        chip memory addr
            size:   int,        data size to dump.

        Examples:
            init()
            read(0x8000000, 4)

        '''
        data = (ctypes.c_ubyte * size)()
        state = self.clib.read_memory(self._stm32, ctypes.c_uint32(addr), data, ctypes.c_uint32(size))
        if state < 0:
            raise SPIBootLoaderFWDLException(self._get_state_message(state))
        return data[:size]

    def write(self, addr, data):
        '''
        Write memory.

        Args:
            addr:   int,        chip memory addr
            data:   int,        data size to dump.

        Examples:
            init()
            write(0x8000000, [1,2,3,4])

        '''
        size = len(data)
        data_c = (ctypes.c_ubyte * size)(*data)
        state = self.clib.write_memory(self._stm32, ctypes.c_uint32(addr), data_c, ctypes.c_uint32(size))
        if state < 0:
            raise SPIBootLoaderFWDLException(self._get_state_message(state))
        return 'done'

    def write_protect(self, start_page, page_num):
        '''
        Write protect memory.

        Enable the write protection of the flash memory pages.
        System reset is called after ACK ok, only for some STM32 BL (STM32F4/F7) and some STM32L4.

        Args:
            start_page:   int,        chip memory page
            page_num:     int,        page number.

        Examples:
            init()
            write_protect(0, 10)

        '''
        assert 0 <= start_page < 256
        assert 0 < page_num <= 256

        state = self.clib.write_protect_cmd(self._stm32, ctypes.c_uint32(start_page), ctypes.c_uint32(page_num))
        if state < 0:
            raise SPIBootLoaderFWDLException(self._get_state_message(state))
        return 'done'

    def write_unprotect(self):
        '''
        Write unprotect memory.

        Disable the write protection of all the flash memory pages.
        System reset is called after ACK ok, only for some STM32 BL (STM32F4/F7) and some STM32L4

        Args:
            None
        Examples:
            init()
            write_unprotect()

        '''
        state = self.clib.write_unprotect_cmd(self._stm32)
        if state < 0:
            raise SPIBootLoaderFWDLException(self._get_state_message(state))
        return 'done'

    def readout_protect(self):
        '''
        Readout unprotect memory.

        Enable the flash memory read protection.
        System reset is called after ACK ok, only for some STM32 BL (STM32F4/F7) and some STM32L4

        Args:
            None
        Examples:
            init()
            readout_protect()

        '''
        state = self.clib.readout_protect_cmd(self._stm32)
        if state < 0:
            raise SPIBootLoaderFWDLException(self._get_state_message(state))
        return 'done'

    def readout_unprotect(self):
        '''
        Readout unprotect memory.

        Disable the flash memory read protection, and erase all flash memory.
        System reset is called after ACK ok, only for some STM32 BL (STM32F4/F7) and some STM32L4

        Args:
            None
        Examples:
            init()
            readout_unprotect()

        '''
        state = self.clib.readout_unprotect_cmd(self._stm32)
        if state < 0:
            raise SPIBootLoaderFWDLException(self._get_state_message(state))
        return 'done'

    def go(self, addr):
        '''
        Execute the downloaded code or any other code by branching to an address specified by the application.

        Args:
            addr:   int,        chip memory addr

        Examples:
            init()
            go(0x8000000)

        '''
        state = self.clib.go_cmd(self._stm32, ctypes.c_uint32(addr))
        if state < 0:
            raise SPIBootLoaderFWDLException(self._get_state_message(state))
        return 'done'


if __name__ == "__main__":
    import time

    sbl_fwdl = SPIBootLoaderFWDL("/dev/AXI4_QSPI_2", libchip="/root/lib/libchip.so")

    # init and get id
    ret = sbl_fwdl.init()
    print(ret)
    ret = sbl_fwdl.get_protocol_version()
    print(ret)
    ret = sbl_fwdl.get_chip_id()
    print(ret)

    # read write
    sbl_fwdl.mass_erase()
    ret = sbl_fwdl.write(0x8000000, [1, 2, 3, 4, 5, 6, 7])
    print(ret)
    ret = sbl_fwdl.read(0x8000000, 10)
    print(ret)

    # unprotect
    ret = sbl_fwdl.readout_unprotect()
    print(ret)
    ret = sbl_fwdl.write_unprotect()
    print(ret)

    # program verify
    base_addr = 0x8000000
    file = r'/root/STM32CubeL4_Demo_STM32L452RE_Nucleo-V1.0.0.bin'
    md5 = "20eb868d5d3df4be0e3f1d68342d31a8"

    t_start = time.time()
    ret = sbl_fwdl.mass_erase()
    print(ret)
    print("erase cost {} s".format(time.time() - t_start))

    t_start = time.time()
    ret = sbl_fwdl.program_only(file, md5, base_addr)
    print(ret)
    print("program_only cost {} s".format(time.time() - t_start))

    t_start = time.time()
    ret = sbl_fwdl.verify(file, md5, base_addr)
    print(ret)
    print("verify cost {} s".format(time.time() - t_start))

    t_start = time.time()
    ret = sbl_fwdl.dump(file, base_addr, os.path.getsize(file))
    print(ret)
    print("dump cost {} s".format(time.time() - t_start))

    # ret = sbl_fwdl.go(base_addr)
    # print(ret)
    # print("go to addr 0x{:0>8x}".format(base_addr))
