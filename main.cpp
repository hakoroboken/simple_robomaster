#include "DigitalIn.h"
#include "DigitalOut.h"
#include "InterruptIn.h"
#include "PinNames.h"
#include "PinNamesTypes.h"
#include "PwmOut.h"
#include "mbed.h"
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cwchar>

#include "mbed_retarget.h"
#include "stm32f4xx.h"

#include "net_lib.hpp"

// User settings

#define CAN_RX PB_8
#define PWM_TX PB_9 

// User settings end

#define MAXIMUM_BUFFER_SIZE 128

// pin setup
CAN can(CAN_RX, PWM_TX, 1000 * 1000);
static BufferedSerial serial_port(USBTX, USBRX);


Ticker safeTimer;
bool safeFlag = false;
int16_t safeCounter = 0;

fn safeCheck(){
    if(safeFlag){
        safeFlag = false;
        safeCounter = 0;
        return;
    }else{
        safeCounter++;
    }

    if(safeCounter > 1000){
        safeCounter = 0;
        safeFlag = false;

        auto msg = CANMessage();
        msg.id = 0x1ff;
        msg.len = 8;
        can.write(msg);
    }
}


int main() {
    safeTimer.attach(&safeCheck, 1ms);

    // Serial
    serial_port.set_baud(115200);
    serial_port.set_format(8, BufferedSerial::None, 1);
    uint8_t buf[MAXIMUM_BUFFER_SIZE] = {0};

    vector<uint8_t> data;

    while (1) {
        if (const ssize_t num = serial_port.read(buf, sizeof(buf))) {
            for(auto i = 0; i < num ; i++){
                data.push_back(buf[i]);
            }

            if(data.size() > 128){
                data.clear();
            }

            if(std::find(data.begin() , data.end() , 's') == data.end()){data.clear();}
            if(std::find(data.begin() , data.end() , 't') == data.end()){data.clear();}
            if(std::find(data.begin() , data.end() , 'e') == data.end()){continue;}
            if(std::find(data.begin() , data.end() , 'n') == data.end()){continue;}


            data.pop_back();
            data.pop_back();
            data.erase(data.begin());
            data.erase(data.begin());
            
            auto mc_msg = deserialize<motor_control_msg>(data);
            auto msg = CANMessage();
            msg.id = 0x1ff;
            msg.len = 8;
            msg.data[0] = set_motor(mc_msg.motor_1) >> 8 & 0xff;
            msg.data[1] = set_motor(mc_msg.motor_1) & 0xff;
            msg.data[2] = set_motor(mc_msg.motor_2) >> 8 & 0xff;
            msg.data[3] = set_motor(mc_msg.motor_2) & 0xff;
            msg.data[4] = set_motor(mc_msg.motor_3) >> 8 & 0xff;
            msg.data[5] = set_motor(mc_msg.motor_3) & 0xff;
            msg.data[6] = set_motor(mc_msg.motor_4) >> 8 & 0xff;
            msg.data[7] = set_motor(mc_msg.motor_4) & 0xff;
            can.write(msg);

            data.clear();
            safeFlag = true;
        }
    }
}