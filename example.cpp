#include "PCA9685_Raspi.hpp"

int main()
{
    if (gpioInitialise() < 0){

        printf("pigpio初期化失敗\n");
    return 1;
    }

    PCA9685_RasPi pin(0, 2000);  // device id, frequency
    // -> freqの最大の1526に丸められる

    pin.setPwm(0, 0.5);
    pin.setPwm(1, 0.5);
    pin.setPwm(2, 0.5);
    pin.setPwm(3, 0.5);

    pin.setDigital(4, false);
    pin.setDigital(5, true);
    pin.setDigital(6, false);
    pin.setDigital(7, true);

    usleep(5 * 1000 * 1000);  // 5s待つ

    pin.setPwm(0, 0.0);
    pin.setPwm(1, 0.0);
    pin.setPwm(2, 0.0);
    pin.setPwm(3, 0.0);

    pin.setDigital(4, false);
    pin.setDigital(5, false);
    pin.setDigital(6, false);
    pin.setDigital(7, false);

    i2cClose(pin.handle());
    gpioTerminate();
    return 0;
}