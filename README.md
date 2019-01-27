# esp8266Car
DIY Wifi RC car




Setup:
  Connect to:
   network:    RCDIYCar
   password:   123456789


Usage:
Open web-browser address: http://192.168.4.1  

Advanced config
  Connect it to router:
		Open web-browser address: http://192.168.4.1/config 
		    <enter data..>

		The connected network address could be found using the USB/serial interface


Dependencies:
	ESP8266 board 
	SPIFFS
	IotWebConf




Install ESP8266 board in the arduino IDE:  http://esp8266.github.io/Arduino/versions/2.0.0/doc/installing.html
info found at: 
In Arduino IDE select:
  File | Preferences
     Tab [Settings]
     Additional Boards Manager : http://arduino.esp8266.com/stable/package_esp8266com_index.json

  Tools | Board: | Boards Manager..  select: esp8266 by ESP8266Comunity


SPIFFS:  more info at: https://arduino-esp8266.readthedocs.io/en/latest/filesystem.html

  Uploading files to file system:
Uploading files to file system
ESP8266FS is a tool which integrates into the Arduino IDE. It adds a menu item to Tools menu for uploading the contents of sketch data directory into ESP8266 flash file system.

Download the tool: https://github.com/esp8266/arduino-esp8266fs-plugin/releases/download/0.3.0/ESP8266FS-0.3.0.zip.
In your Arduino sketchbook directory, create tools directory if it doesn’t exist yet
Unpack the tool into tools directory (the path will look like <home_dir>/Arduino/tools/ESP8266FS/tool/esp8266fs.jar)
Restart Arduino IDE
Open a sketch (or create a new one and save it)
Go to sketch directory (choose Sketch > Show Sketch Folder)
Create a directory named data and any files you want in the file system there
Make sure you have selected a board, port, and closed Serial Monitor
Select Tools > ESP8266 Sketch Data Upload. This should start uploading the files into ESP8266 flash file system. When done, IDE status bar will display SPIFFS Image Uploaded message.


If you have the Windows 10 app: 
 - Close the Arduino IDE (v1.8.5)
 - Unzip the ESP8266FS-0.3.0.zip file. 
 - Then copy the newly created ESP8266FS folder into:

 C:\Users\#UserName#\Documents\Arduino\tools\

 - Create the tools folder inside of the ...\Documents\Arduino\ folder if it is not already there.
 - The full location of the .jar file is:
 
 C:\Users\#UserName#\Documents\Arduino\tools\ESP8266FS\tool\esp8266fs.jar

- Start the Arduino IDE
- You should now see the "ESP Sketch Data Upload" option in the Tools menu


Install IotWebConf
Arduino IDE:
  Open the menu :
      Tools | Manage Libraries...
      In inputfield enter IotWebConf and select IotWebConf by Balazs Kelemen and click install
  