#pragma once

#include <pigpio.h>
#include <unistd.h>

#include <cstdint>
#include <cstdio>
#include <iostream>

#define PCA9685_ADDR 0x40
#define PCA9685_MODE1 0x00
#define PCA9685_PRESCALE 0xFE
#define LED0_ON_L 0x06
#define LED0_ON_H 0x07
#define LED0_OFF_L 0x08
#define LED0_OFF_H 0x09

struct OpenById { int id; float freq; };
struct OpenByHandle { int handle; float freq; };

class PCA9685_RasPi
{
 private:
  int handle_;
  float freq_;
  bool handle_owner_;

 public:
  // id: 0から順のデバイスid(0~63), freq: pwmidで定めたデバイスの周波数(MAX: 1526)
  PCA9685_RasPi(const OpenById& args) : freq_(args.freq), handle_owner_(true)
  {
    handle_ = i2cOpen(1, PCA9685_ADDR + args.id, 0);
    if (handle_ < 0)
    {
      printf("I2Cオープン失敗\n");
      // 例外を投げて呼び出し元で処理できるようにする
      throw std::runtime_error("I2Cオープン失敗");
    }
    write8(PCA9685_MODE1, 0x00);  // 通常モード
    usleep(500);
    pca9685_set_pwm_freq();
  }

  // すでに開かれているI2Cハンドルを外部からもらう場合
  PCA9685_RasPi(const OpenByHandle& args) : handle_(args.handle), freq_(args.freq), handle_owner_(false)
  {
    if (handle_ < 0)
    {
      printf("有効でないI2Cハンドルが渡されました\n");
      throw std::runtime_error("I2Cハンドル無効");
    }
    write8(PCA9685_MODE1, 0x00);
    usleep(500);
    pca9685_set_pwm_freq();
  }

  // 8bitレジスタ書き込み
  void write8(uint8_t reg, uint8_t data)
  {
    i2cWriteByteData(handle_, reg, data);
  }

  // 8bitレジスタ読み込み
  uint8_t read8(uint8_t reg)
  {
    return i2cReadByteData(handle_, reg);
  }

  // 周波数設定
  void pca9685_set_pwm_freq(float freq = -1)
  {
    float use_freq = (freq < 0) ? freq_ : freq;
    float prescaleval = 25000000.0;
    prescaleval /= 4096.0;
    prescaleval /= use_freq;
    prescaleval -= 1.0;
    uint8_t prescale = (uint8_t)(prescaleval + 0.5);

    uint8_t oldmode = read8(PCA9685_MODE1);
    uint8_t sleep = (oldmode & 0x7F) | 0x10;  // sleepビットセット
    write8(PCA9685_MODE1, sleep);
    write8(PCA9685_PRESCALE, prescale);
    write8(PCA9685_MODE1, oldmode);
    usleep(500);
    write8(PCA9685_MODE1, oldmode | 0x80);  // Restart
  }

  // PWM出力設定
  void pca9685_set_pwm(uint8_t channel, uint16_t on, uint16_t off)
  {
    write8(LED0_ON_L + 4 * channel, on & 0xFF);
    write8(LED0_ON_H + 4 * channel, on >> 8);
    write8(LED0_OFF_L + 4 * channel, off & 0xFF);
    write8(LED0_OFF_H + 4 * channel, off >> 8);
  }

  void setPwm(uint8_t channel, float duty)
  {
    if (duty < 0) duty = 0;
    if (duty > 1) duty = 1;
    int off = static_cast<int>(4095 * duty);
    pca9685_set_pwm(channel, 0, off);
  }

  void setDigital(uint8_t channel, bool state)
  {
    uint16_t off = state ? 4095 : 0;
    write8(LED0_ON_L + 4 * channel, 0x00);
    write8(LED0_ON_H + 4 * channel, 0x00);
    write8(LED0_OFF_L + 4 * channel, off & 0xFF);
    write8(LED0_OFF_H + 4 * channel, off >> 8);
  }

  int handle()
  {
    return handle_;
  }
};