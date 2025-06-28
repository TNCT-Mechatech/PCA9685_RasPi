#pragma once

#include <cstdint>
#include <cstdio>
#include <iostream>
#include <pigpio.h>
#include <unistd.h>

#define PCA9685_ADDR 0x40
#define PCA9685_MODE1 0x00
#define PCA9685_PRESCALE 0xFE
#define LED0_ON_L 0x06
#define LED0_ON_H 0x07
#define LED0_OFF_L 0x08
#define LED0_OFF_H 0x09

class PCA9685_RasPi {
private:
  int handle;
  float freq;

public:
  // id: 0から順のデバイスid(0~63), freq: pwmidで定めたデバイスの周波数
  PCA9685_RasPi(int id, float freq_) : freq(freq_) {

    handle = i2cOpen(1, PCA9685_ADDR + id, 0);
    if (handle < 0) {
      printf("I2Cオープン失敗\n");
      // 例外を投げて呼び出し元で処理できるようにする
      throw std::runtime_error("I2Cオープン失敗");
    }
    write8(PCA9685_MODE1, 0x00); // 通常モード
    usleep(500);
    pca9685_set_pwm_freq();
  }
  // 8bitレジスタ書き込み
  void write8(uint8_t reg, uint8_t data) {
    i2cWriteByteData(handle, reg, data);
  }

  // 8bitレジスタ読み込み
  uint8_t read8(uint8_t reg) { return i2cReadByteData(handle, reg); }

  // 周波数設定
  void pca9685_set_pwm_freq(float freq_ = -1) {
    float freq = (freq_ < 0) ? freq : freq_;
    float prescaleval = 25000000.0;
    prescaleval /= 4096.0;
    prescaleval /= freq;
    prescaleval -= 1.0;
    uint8_t prescale = (uint8_t)(prescaleval + 0.5);

    uint8_t oldmode = read8(PCA9685_MODE1);
    uint8_t sleep = (oldmode & 0x7F) | 0x10; // sleepビットセット
    write8(PCA9685_MODE1, sleep);
    write8(PCA9685_PRESCALE, prescale);
    write8(PCA9685_MODE1, oldmode);
    usleep(500);
    write8(PCA9685_MODE1, oldmode | 0x80); // Restart
  }

  // PWM出力設定
  void pca9685_set_pwm(uint8_t channel, uint16_t on, uint16_t off) {
    write8(LED0_ON_L + 4 * channel, on & 0xFF);
    write8(LED0_ON_H + 4 * channel, on >> 8);
    write8(LED0_OFF_L + 4 * channel, off & 0xFF);
    write8(LED0_OFF_H + 4 * channel, off >> 8);
  }

  void setPwm(uint8_t channel, float duty) {
    if (duty < 0)
      duty = 0;
    if (duty > 1)
      duty = 1;
    int off = static_cast<int>(4096 * duty);
    pca9685_set_pwm(channel, 0, off);
  }

  void setDigital(uint8_t channel, bool state) {
    uint16_t off = state ? 4095 : 0;
    write8(LED0_ON_L + 4 * channel, 0x00);
    write8(LED0_ON_H + 4 * channel, 0x00);
    write8(LED0_OFF_L + 4 * channel, off & 0xFF);
    write8(LED0_OFF_H + 4 * channel, off >> 8);
  }

  int handle() { return handle; }
};