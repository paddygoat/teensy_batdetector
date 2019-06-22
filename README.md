# teensy_batdetector
Batdetector on Teensy3.6 (Based on original-code from Frank (DD4WH)
https://github.com/DD4WH/Teensy-Bat-Detector )

This is a work in progress project, the code and setup is still changing. If you have a specific request or otherwise questions please share them. 

Several specific libraries have been added to this repository, in de directory <b>lib</b> you can find them.

I am using a non-default programming setup for Teensy, that means that the code might not work in other environments.

Setup @2019-06-22

Operating system Linux Mint 19.1

Visual Code 1.35.1

-PlatformIO Home 2.2.0·Core 3.6.7

-platformio.ini

  [env:teensy36]<br>
  
  platform = teensy<br>
  board = teensy36<br>
  framework = arduino<br>

