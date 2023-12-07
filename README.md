# DS18B20-Temperature-Sensor

Digital sensors are much more interesting to work when compared to analog devices because when working with the first ones there is only the intrinsic accuracy of the sensor to deal with. A digital sensor throws a binary code which is then interpreted and converted by the microcontroller. The algorithm for the interpretation and conversion depends on the sensor but it can always be found on its datasheet.

In some sensors, like the DS18B20 [datasheet](resources/DS18B20.pdf) which has a dedicated CRC hardware, when the binary code (temperature) is calculated it is also calculated an additional cyclic redundancy check (CRC) byte which is appended to the transmission. This CRC byte is extremely important to make sure that the received binary code is correct. Then, to confirm that the transmission has no errors the same CRC algorithm must be implemented on the receiver side.

On the other hand, an analog sensor provides an analog ouput (e.g. current, voltage) proportional to the absolute temperature. This means that any change on that current or voltage will change the temperature reading. Knowing that there are lots of external factors that can influence the current or voltage (e.g. noise, ambient temperature, length of cables) it can be hard to know if that reading is correct.

Also, analog sensors rely on some sort of current or voltage reference. Again, a variation on that reference will change the temperature reading.

For this tutorial I decided to use the interface code written by Dr_Ugi (member of the [Arduino Forum](https://forum.arduino.cc/u/dr_ugi/summary)) which allowed me to communicate with the sensor without any library. I had this decision for the reason that sometimes it is easier to use our own code (or one extremely well commented) because when a problem occurs and there is no idea why, an analysis to that library’s code must be performed which can be very painful. Another disadvantage of using libraries is that they can consume lots of resources even if all the available functionalities are not being used.

For this tutorial you need:

- 1x Arduino Nano 3.0
- 1x DS18B20 temperature sensor
- 1x 4.7kOhm resistor
- Breadboard and wires

Now you have to replicate the circuit below:<br/>
<br/>
![alt text](resources/DS18B20-Temperature-Sensor_bb.png?raw=true)

Upload the [code](DS18B20_Temperature_Sensor.ino) to the board.

For a simple project, like this one: print the temperature in the Serial Monitor, it will be very unlikely to get any trouble when using a library. However, in the Aquarium PWM LED project, when I decided to change the LM35 temperature sensor for this one, the LEDs were flickering when the temperature conversion was being performed. This problem occurs because the DS18B20 library I was using disables the interrupts (mandatory while performing the temperature conversion on the DS18B20 sensor) for too long.

So, to solve that problem, [Dr_Ugi](https://forum.arduino.cc/u/dr_ugi/summary) explained that I could disable the interrupts just for a few microseconds. Working with a higher refresh rate the LED flickering is now imperceptible to the human eye.

Serial Monitor:<br/>
<br/>
![alt text](resources/SerialMonitor.jpg?raw=true)