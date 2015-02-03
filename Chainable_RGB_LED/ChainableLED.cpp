/*
 * Copyright (C) 2013 Seeed Technology Inc.
 * Copyright (C) 2012 Paulo Marques (pjp.marques@gmail.com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of 
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all 
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN 
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/* Information about the P9813 protocol obtained from:
 * http://www.seeedstudio.com/wiki/index.php?title=Twig_-_Chainable_RGB_LED
 *
 * HSB to RGB routine adapted from:
 * http://mjijackson.com/2008/02/rgb-to-hsl-and-rgb-to-hsv-color-model-conversion-algorithms-in-javascript
 *
 * This library is ported from Arduino to mbed
 */


// --------------------------------------------------------------------------------------

#include "ChainableLED.h"

// Forward declaration
float hue2rgb(float p, float q, float t);

// --------------------------------------------------------------------------------------

ChainableLED::ChainableLED(PinName clk_pin, PinName data_pin, unsigned int number_of_leds) :
    _clk_pin(clk_pin), _data_pin(data_pin)
{
    _num_leds = number_of_leds;
    
    for (uint8_t i=0; i<_num_leds; i++)
        setColorRGB(i, 0, 0, 0);
}

ChainableLED::~ChainableLED()
{
}

// --------------------------------------------------------------------------------------

void ChainableLED::clk(void)
{
    _clk_pin = 0;
    wait_us(_CLK_PULSE_DELAY);
    _clk_pin = 1;
    wait_us(_CLK_PULSE_DELAY);
}

void ChainableLED::sendByte(uint8_t b)
{
    // Send one bit at a time, starting with the MSB
    for (uint8_t i=0; i<8; i++)
    {
        // If MSB is 1, write one and clock it, else write 0 and clock
        if ((b & 0x80) != 0)
            _data_pin = 1;
        else
            _data_pin = 0;
            
        clk();

        // Advance to the next bit to send
        b <<= 1;
    }
}
 
void ChainableLED::sendColor(uint8_t red, uint8_t green, uint8_t blue)
{
    // Start by sending a uint8_t with the format "1 1 /B7 /B6 /G7 /G6 /R7 /R6"
    uint8_t prefix = 0xC0;
    if ((blue & 0x80) == 0)     prefix|= 0x20;
    if ((blue & 0x40) == 0)     prefix|= 0x10; 
    if ((green & 0x80) == 0)    prefix|= 0x08;
    if ((green & 0x40) == 0)    prefix|= 0x04;
    if ((red & 0x80) == 0)      prefix|= 0x02;
    if ((red & 0x40) == 0)      prefix|= 0x01;
    sendByte(prefix);
        
    // Now must send the 3 colors
    sendByte(blue);
    sendByte(green);
    sendByte(red);
}

void ChainableLED::setColorRGB(unsigned int led, uint8_t red, uint8_t green, uint8_t blue)
{
    // Send data frame prefix (32x "0")
    sendByte(0x00);
    sendByte(0x00);
    sendByte(0x00);
    sendByte(0x00);
    
    // Send color data for each one of the leds
    for (uint8_t i=0; i<_num_leds; i++)
    {
        if (i == led)
        {
            _led_state[i*3 + _CL_RED] = red;
            _led_state[i*3 + _CL_GREEN] = green;
            _led_state[i*3 + _CL_BLUE] = blue;
        }
                    
        sendColor(_led_state[i*3 + _CL_RED], 
                  _led_state[i*3 + _CL_GREEN], 
                  _led_state[i*3 + _CL_BLUE]);
    }

    // Terminate data frame (32x "0")
    sendByte(0x00);
    sendByte(0x00);
    sendByte(0x00);
    sendByte(0x00);
}

void ChainableLED::setColorHSB(unsigned int led, float hue, float saturation, float brightness)
{
    float r, g, b;
    
//     constrain(hue, 0.0, 1.0);
//     constrain(saturation, 0.0, 1.0);
//     constrain(brightness, 0.0, 1.0);

    if(saturation == 0.0)
    {
        r = g = b = brightness;
    }
    else
    {
        float q = brightness < 0.5 ? 
            brightness * (1.0 + saturation) : brightness + saturation - brightness * saturation;
        float p = 2.0 * brightness - q;
        r = hue2rgb(p, q, hue + 1.0/3.0);
        g = hue2rgb(p, q, hue);
        b = hue2rgb(p, q, hue - 1.0/3.0);
    }

    setColorRGB(led, (uint8_t)(255.0*r), (uint8_t)(255.0*g), (uint8_t)(255.0*b));
}

// --------------------------------------------------------------------------------------

float hue2rgb(float p, float q, float t)
{
    if (t < 0.0) 
        t += 1.0;
    if(t > 1.0) 
        t -= 1.0;
    if(t < 1.0/6.0) 
        return p + (q - p) * 6.0 * t;
    if(t < 1.0/2.0) 
        return q;
    if(t < 2.0/3.0) 
        return p + (q - p) * (2.0/3.0 - t) * 6.0;

    return p;
}
