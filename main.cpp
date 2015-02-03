#include "mbed.h"
#include "ChainableLED.h"
 
// ChainableLED(clk, data, number_of_leds)
ChainableLED color_led(p14, p13, 1);
 
int main() {
    uint8_t value = 0;
    
    while(1) {
        value++;
        // ChainableLED.setColorRGB(index_of_led, red, green, blue)
        color_led.setColorRGB(0, value, 255 - value, value + 80);
        wait_ms(10);
    }
}