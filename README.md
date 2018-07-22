# Internet_Radio_ESP8266_VS1053
This is an Internet Radio reciever made with ESP8266 and VS1053 on Arduino enviroment

I use:<br> 
ESP8266 D1 mini like this <br>
https://www.ebay.com/sch/i.html?_from=R40&_trksid=m570.l1313&_nkw=d1+mini+esp8266&_sacat=0

VS1053 like this <br>
https://www.ebay.com/itm/VS1053-MP3-Module-Development-Board-w-On-Board-Recording-Function-SPI-Interface/282624198793?hash=item41cdb70889%3Ag%3ARvAAAOSweRlauwP3&_sacat=0&_nkw=vs1053&_from=R40&rt=nc&LH_TitleDesc=0

Serial I2C LCD 20x4 like this <br>
https://www.ebay.com/itm/IIC-I2C-TWI-SP-I-Serial-Interface2004-20X4-Character-LCD-Module-Display-Blue-CK/191903982397?hash=item2cae5e633d%3Ag%3ADzMAAOSwMNxXa31J&_sacat=0&_nkw=LCD+20x4&_from=R40&rt=nc&_trksid=m570.l1313


Wiring of VS1053 board (SPI connected in a standard way) to Wemos D1 Mini : <br>
VS1053_CS   (D1 mini pin D8)  (Arduino pin 15)<br>
VS1053_DCS  (D1 mini pin D0)  (Arduino pin 16)<br>
VS1053_DREQ (D1 mini pin D3)  (Arduino pin 0)<br>
MOSI        (D1 mini pin D7)  (Arduino pin 13)<br>
MISO        (D1 mini pin D6)  (Arduino pin 12)<br>
SCK         (D1 mini pin D5)  (Arduino pin 14)<br>
XRST        (D1 mini pin RST)<br>

Wiring of Serial I2C 20x4 LCD Display : <br>
SDA  (D1 mini pin D2) (Arduino pin 4)<br>
CLS  (D1 mini pin D1) (Arduino pin 5)<br>


Librarys: <br>
VS 1053 : https://github.com/baldram/ESP_VS1053_Library The library use also a third-party logging framework ArduinoLog (https://platformio.org/lib/show/1532/ArduinoLog) for debugging purposes.<br>
Serial LCD Display : https://github.com/fdebrabander/Arduino-LiquidCrystal-I2C-library<br>


Tools:<br>
ESP8266 sketch Data Uploader : https://github.com/esp8266/arduino-esp8266fs-plugin
The above tool is very usefull to upload files in ESP8266 file system

Tips:<br>
1. With tool Data Uploader, upload first the files in /data directory<br>
2. In lines 18 and 19 add your AP SSID and Password. Then compile and upload the sketch to ESP8266<br>
3. When upload the sketch to ESP8266 open your mobile phone browser and look at http://192.168.1.249 This address could change in line 85<br>
4. The web interface is focused on mobile phones.<br>
5. Use Alarm option for a melodic awakening


