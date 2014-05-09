/*
PCA9685 driver code is placed under the BSD license.
Copyright (c) 2014, Emlid Limited, www.emlid.com
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the Emlid Limited nor the names of its contributors 
      may be used to endorse or promote products derived from this software
      without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL EMLID LIMITED BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "PCA9685.h"

/** Default constructor, uses default I2C address.
 * @see PCA9685_DEFAULT_ADDRESS
 */
PCA9685::PCA9685() {
    devAddr = PCA9685_DEFAULT_ADDRESS;
}

/** Specific address constructor.
 * @param address I2C address
 * @see AK8975_DEFAULT_ADDRESS
 */
PCA9685::PCA9685(uint8_t address) {
    devAddr = address;
}

/** Power on and prepare for general usage.
 * This method reads prescale value stored in PCA9685 and calculate frequency based on it.
 * Then it enables auto-increment of register address to allow for faster writes.
 * And finally the restart is performed to enable clocking.
 */
void PCA9685::initialize() {
    frequency = getFrequency();
    I2Cdev::writeBit(devAddr, PCA9685_RA_MODE1, PCA9685_MODE1_AI_BIT, 1);
    restart();   
}

/** Verify the I2C connection.
 * @return True if connection is valid, false otherwise
 */
bool PCA9685::testConnection() {
    uint8_t data;
    return I2Cdev::readByte(devAddr, PCA9685_RA_PRE_SCALE, &data);
}

/** Put PCA9685 to sleep mode thus turning off the outputs.
 * @see PCA9685_MODE1_SLEEP_BIT
 */
void PCA9685::sleep() {
    I2Cdev::writeBit(devAddr, PCA9685_RA_MODE1, PCA9685_MODE1_SLEEP_BIT, 1);
}

/** Disable sleep mode and start the outputs.
 * @see PCA9685_MODE1_SLEEP_BIT
 * @see PCA9685_MODE1_RESTART_BIT
 */
void PCA9685::restart() {
    I2Cdev::writeBit(devAddr, PCA9685_RA_MODE1, PCA9685_MODE1_SLEEP_BIT, 0);  
    I2Cdev::writeBit(devAddr, PCA9685_RA_MODE1, PCA9685_MODE1_RESTART_BIT, 1);   
}

/** Calculate prescale value based on the specified frequency and write it to the device.
 * @return Frequency in Hz
 * @see PCA9685_RA_PRE_SCALE
 */
float PCA9685::getFrequency() {
    uint8_t data;
    I2Cdev::readByte(devAddr, PCA9685_RA_PRE_SCALE, &data);
    return 25000000.f / 4096.f / (data + 1);
}


/** Calculate prescale value based on the specified frequency and write it to the device.
 * @param Frequency in Hz
 * @see PCA9685_RA_PRE_SCALE
 */
void PCA9685::setFrequency(float frequency) {
    sleep();
    usleep(10000);
    uint8_t prescale = roundf(25000000.f / 4096.f / frequency)  - 1;
    I2Cdev::writeByte(devAddr, PCA9685_RA_PRE_SCALE, prescale);
    this->frequency = getFrequency();
    restart();                
}

/** Set channel start offset of the pulse and it's length
 * @param Channel number (0-15)
 * @param Offset (0-4095)
 * @param Length (0-4095)
 * @see PCA9685_RA_LED0_ON_L
 */
void PCA9685::setPWM(uint8_t channel, uint16_t offset, uint16_t length) {
    uint8_t data[4] = {offset & 0xFF, offset >> 8, length & 0xFF, length >> 8};
    I2Cdev::writeBytes(devAddr, PCA9685_RA_LED0_ON_L + 4 * channel, 4, data);
}

/** Set channel's pulse length
 * @param Channel number (0-15)
 * @param Length (0-4095)
 * @see PCA9685_RA_LED0_ON_L
 */
void PCA9685::setPWM(uint8_t channel, uint16_t length) {
    setPWM(channel, 0, length);
}

/** Set channel's pulse length in microseconds
 * @param Channel number (0-15)
 * @param Length in microseconds
 * @see PCA9685_RA_LED0_ON_L
 */
void PCA9685::setPWMmS(uint8_t channel, float length_mS) {	
    setPWM(channel, round((length_mS * 4096.f) / (1000.f / frequency) - 1));
}

/** Set start offset of the pulse and it's length for all channels
 * @param Offset (0-4095)
 * @param Length (0-4095)
 * @see PCA9685_RA_ALL_LED_ON_L
 */
void PCA9685::setAllPWM(uint16_t offset, uint16_t length) {
    uint8_t data[4] = {offset & 0xFF, offset >> 8, length & 0xFF, length >> 8};
    I2Cdev::writeBytes(devAddr, PCA9685_RA_ALL_LED_ON_L, 4, data);
}

/** Set pulse length for all channels
 * @param Length (0-4095)
 * @see PCA9685_RA_ALL_LED_ON_L
 */
void PCA9685::setAllPWM(uint16_t length) {
    setAllPWM(0, length);
}

/** Set pulse length in microseconds for all channels
 * @param Length in microseconds
 * @see PCA9685_RA_ALL_LED_ON_L
 */
void PCA9685::setAllPWMmS(float length_mS) {	
    setAllPWM(round((length_mS * 4096.f) / (1000.f / frequency) - 1));
}