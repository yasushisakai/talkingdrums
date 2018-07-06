# the talking drums

[project page](https://yasushisakai.github.io/talkingdrums/)



# auto start processing sketch

## 
change following file
```
~/.config/lxsession/LXDE-pi/autostart
```

and add

```
/usr/local/bin/processing-java --sketch=/home/pi/sketchbook/sketchname --run
```

the path to processing-java and sketchbook path should be changed accordingly


Flash ATMega comands

```
cd ~/Library/Arduino15/packages/arduino/hardware/avr/1.6.12/bootloaders/optiboot

avrdude -b 19200 -c usbtiny -p m328p -v -e -U efuse:w:0x05:m -U hfuse:w:0xDE:m -U lfuse:w:0xFF:m

avrdude -b19200 -c usbtiny -p m328p -v -e -U flash:w:optiboot_atmega328.hex -U lock:w:0x0F:m


```
