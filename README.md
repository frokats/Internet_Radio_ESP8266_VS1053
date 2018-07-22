# Internet_Radio_ESP8266_VS1053
This is an Internet Radio reciever made with ESP8266 on Arduino enviroment

I use:<br> 
ESP8266 D1 mini like this <br>
https://www.ebay.com/sch/i.html?_from=R40&_trksid=m570.l1313&_nkw=d1+mini+esp8266&_sacat=0

VS1053 like this <br>
https://www.ebay.com/itm/VS1053-MP3-Module-Development-Board-w-On-Board-Recording-Function-SPI-Interface/282624198793?hash=item41cdb70889%3Ag%3ARvAAAOSweRlauwP3&_sacat=0&_nkw=vs1053&_from=R40&rt=nc&LH_TitleDesc=0

Serial LCD 20x4 like this <br>
https://www.ebay.com/itm/IIC-I2C-TWI-SP-I-Serial-Interface2004-20X4-Character-LCD-Module-Display-Blue-CK/191903982397?hash=item2cae5e633d%3Ag%3ADzMAAOSwMNxXa31J&_sacat=0&_nkw=LCD+20x4&_from=R40&rt=nc&_trksid=m570.l1313


Wiring of VS1053 board (SPI connected in a standard way) to Wemos D1 Mini : <br>
            D1 mini pin     Arduino pin<br>
VS1053_CS       D8              15<br>
VS1053_DCS      D0              16<br>
VS1053_DREQ D3  (Arduino pin 0)<br>
MOSI        D7  (Arduino pin 13)<br>
MISO        D6  (Arduino pin 12)<br>
SCK         D5  (Arduino pin 14)<br>
XRST        RST (Arduino pin RST)<br>

