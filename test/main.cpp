/*
 * main.cpp
 *
 *  Created on: 2024-5-15
 *      Author: StrayingCloud
 */
#include <iostream>
#include <fstream>
#include <cstdio>
#include <string.h>
#include <stdlib.h>
#include <typeinfo>
#include <chrono>

#include "DbgrCommon.h"
#include "stm32xxx.hpp"
#include "stm32xxx_c.cpp"



using namespace std;


int main(int argc ,char **argv){

    // FILE *stream;
    // stream = freopen("log.txt", "w", stdout);
    // printf("This is a log message.\n");

    cout << "main." << endl;

    char* bus_name = (char*)"/dev/AXI4_QSPI_2";
    char* targe_file = (char*)"/root/STM32CubeL4_Demo_STM32L452RE_Nucleo-V1.0.0.bin";

    STM32XXX* s = new_STM32XXX(bus_name, 8000000, 0);
    del_STM32XXX(s);
    printf("------\n");

    STM32XXX stm32 = STM32XXX(bus_name, 8000000, SpiCpol0nCpha0Mode, (char*)"/root/log/spi_bootloader.log");

    int stat;
    uint8_t buff[256];
    uint8_t new_buff[256];
    int length = 0;
    uint32_t flash_base_addr = 0x8000000;
    uint32_t elapsed = 0;
    uint32_t file_size = 37831;
    auto start = std::chrono::steady_clock::now();
    auto end = std::chrono::steady_clock::now();


    printf("start test....\n");
    stat = stm32.init();
    printf("stat %d, line %d\n", stat, __LINE__);
    if (stat == -6){
        return -1;
    }

    stat = stm32.get_command_cmd(buff, length);
    printf("stat %d, line %d\n", stat, __LINE__);
    if (stat < 0){return -1;}
    printf("get cmd:\n");
    cout << dec << length << endl;
    for (int i = 0; i < length; ++i)
    {
        printf("0x%x ", buff[i]);
    }
    printf("\n");

    uint8_t version;
    stat = stm32.get_version_cmd(version);
    printf("stat %d, line %d\n", stat, __LINE__);
    printf("version: 0x%x\n", version);
    if (stat < 0){return -1;}


    stat = stm32.get_id_cmd(buff, length);
    printf("stat %d, line %d\n", stat, __LINE__);
    if (stat < 0){return -1;}
    printf("get id\n");
    cout << dec << length << endl;
    for (int i = 0; i < length; ++i)
    {
        printf("0x%x ", buff[i]);
    }
    printf("\n");

    // stat = stm32.read_memory_cmd(flash_base_addr, buff, 256);
    // printf("stat %d, line %d\n", stat, __LINE__);
    // printf("[\n");
    // for (int i = 0; i < 256; ++i)
    // {
    //     printf("0x%02x ", buff[i]);
    //     if  ( !((i+1) % 16) ){
    //         printf("\n");
    //     }
    //     new_buff[i] = buff[i] + 1;
    // }
    // printf("\n]\n");

    // stm32.readout_unprotect_cmd();
    // printf("stat %d, line %d\n", stat, __LINE__);
    // printf("readout_unprotect_cmd\n");

    // printf("write_unprotect_cmd\n");
    // stat = stm32.write_unprotect_cmd();
    // printf("stat %d, line %d\n", stat, __LINE__);
    // if (stat < 0){return -1;}


    printf("--- loop ---\n");
    uint32_t e = 0;
    uint32_t p = 0;
    uint32_t v = 0;
    uint32_t d = 0;
    uint32_t total = 100000;
    for (int i=1; i<=total; ++i)
    {
        start = std::chrono::steady_clock::now();
        stat = stm32.mass_erase();
        end = std::chrono::steady_clock::now();
        elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        printf("erase cost %d ms\n", elapsed);
        if (stat < 0){return -1;}

        if (stat < 0){
            e += 1;
        }

        start = std::chrono::steady_clock::now();
        stat = stm32.program_only(targe_file, flash_base_addr, file_size);
        end = std::chrono::steady_clock::now();
        elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        printf("program cost %d ms\n", elapsed);
        if (stat < 0){return -1;}

        if (stat < 0){
            p += 1;
        }

        start = std::chrono::steady_clock::now();
        stat = stm32.verify(targe_file, flash_base_addr, file_size);
        end = std::chrono::steady_clock::now();
        elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        printf("verify cost %d ms\n", elapsed);
        if (stat < 0){return -1;}

        if (stat < 0){
            v += 1;
        }

        start = std::chrono::steady_clock::now();
        stat = stm32.dump(targe_file, flash_base_addr, file_size);
        end = std::chrono::steady_clock::now();
        elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        printf("dump cost %d ms\n", elapsed);
        if (stat < 0){return -1;}

        if (stat < 0){
            d += 1;
        }


        printf("----------------- e %d, p %d, v %d, d %d, total %d----------------------- \n", e, p, v, d, i);

    }

    // stm32.write_protect_cmd(0, 46);
    // printf("stat %d, line %d\n", stat, __LINE__);
    // printf("write_protect_cmd\n");

    // stm32.readout_protect_cmd();
    // printf("stat %d, line %d\n", stat, __LINE__);
    // printf("readout_protect_cmd\n");

    // stat = stm32.go_cmd(flash_base_addr);
    // printf("go addr %p\n", flash_base_addr);
    // printf("stat %d, line %d\n", stat, __LINE__);
    // if (stat < 0){return -1;}

    // fclose(stdout);

    return 1;
}





