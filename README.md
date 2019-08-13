# teensy_batdetector
Batdetector on Teensy3.6 (Based on original-code from Frank (DD4WH)
https://github.com/DD4WH/Teensy-Bat-Detector )

latest changes<br>
<b> v0.94 important changes:</b>
- EEprom saving was not always when asked to restart in the user mode<br>
- when no SD card was mounted the Left Encodermenu was not cyclic (jumped back to volume)<br>
- when no sD card was mounted user could still set the LEFT button to RECORD or PLAY<br>

<b>minor changes</b>:
- store the detectormode when starting a recording and restore after stopping a recording<br>

 <b> bug repair:</b>
  in previous versions the recording system was not checking if a filename allready existed but only checked the number of files on the SD. If a file was removed (for instance from a PC) this would lead to overwriting existing files. <br>
  In the new setup the new filenumber for a recording will be checked against highest filenumber in the
          allready stored files.

<b>v0.93</b>
minor changes:<br>
 - changing sample_rate was not stored properly in previous versions<br>
 - added system-voltage in startup-display (for future battery managment)<br>
 

<b>v0.92</b>
 - further work on the setup to record/play files <b>NEEDS TESTING</b> <br>
 - filename display shortened so no overlap with other menu <br>
 - filename selection (when playing) is now cyclic <br>
 To go to the latest recording just turn counterclockwise instead of stepping through all previous  <br>
 - rightside menu allows setting of samplerate (in all modes) <br>
 - during playing of files microphones will be switched off <br>
 - maximum number of files to be stored 999 (BXXX_RRR.RAW) XXX=0..999 RRR=samplerate in Khz <br>


<b>0.91</b><br> intermediate test version !! 
-this hopefully solves the issues with replay of recorded files that were present since v0.84 

<b>IMPORTANT: updates since version 0.84 have problems with replaying files from the SD-card. Recording works fine. From V0.91 working again</b> 

<b>0.89</b><br> 
- eeprom version 0001
- indicator for low-high detectionrange in graph for AUTO_TE <br>
- menu updated and more uniform (all choosen values shown in white next to menusetting also for TE_LOW/TE_SPD) <br>
- added counter to keep track of EEprom saving (still reset to 0 for each EEprom version)<br>
- added text to display after pressing PRESET USER to save to EEprom<br>


<b>0.88</b><br>(source and hex)<br>
-EEprom saving added (initial test)<br>
-Menu expanded with <br>
- Preset: Default (as programmed) or User (saved at that moment)
-  TE_speed : replay speed when using Auto_TimeExpansion (20 means 1/20th of original speed)<br>
-  TE_low : lowest frequency to check for possible signalpeaks that will trigger the TimeExpansion<br>
  


<b>0.86</b><br>
-changes by WMXZ to use latest uSDFS library and not be depending on ff_utils and SD.h<br>


<b>v0.85</b> (23 06 2019) <br>
  -changed the usage of seconds2tm from the ff_utils library,<br> 
   created ff_utils_copy library that is independent of other uSDFS libraries.


This is a work in progress project, the code and setup is still changing. If you have a specific request or otherwise questions please share them. 

Several specific libraries have been added to this repository, in de directory <b>lib</b> you can find them.

I am using a non-default programming setup for Teensy, that means that the code might not work in other environments.

Setup @2019-06-22

Operating system Linux Mint 19.1

<b>Visual Code 1.35.1</b><br>
-PlatformIO Home 2.2.0·Core 3.6.7<br>
-platformio.ini<br>
  [env:teensy36]<br>
  platform = teensy<br>
  board = teensy36<br>
  framework = arduino<br>

