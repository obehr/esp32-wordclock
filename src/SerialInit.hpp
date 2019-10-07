/*
 * SerialInit.hpp
 *
 *  Created on: 07.10.2019
 *      Author: oliver
 */

#ifndef SRC_SERIALINIT_HPP_
#define SRC_SERIALINIT_HPP_

#include <HardwareSerial.h>

/**
 * Class for initialization of serial interface.
 *
 * Will init the serial interface in its constructor. This way, it is available
 * for other objects in their constructors and not only after initialization in
 * the main setup() function.
 */
class SerialInit
{
public:
    /**
     * Constructor
     */
    SerialInit(unsigned long ui32Baud)
    {
        // Init serial console
        Serial.begin (ui32Baud);
        Serial.println ("Serial interface initialized.");
    }
};



#endif /* SRC_SERIALINIT_HPP_ */
