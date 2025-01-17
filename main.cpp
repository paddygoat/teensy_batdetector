//REMINDER FOR CORBEE
//PROCEDURE FOR LINUX DIRECT UPLOAD when using PLATFORMIO
//Dont forget Install Step 2: (Linux only) Install udev Rules
//The udev rule file gives non-root users permission to use the Teensy devices (serial, HID, etc). More Linux tips below.
//sudo cp 49-teensy.rules /etc/udev/rules.d/
//CALL TEENSY_LOADER and SELECT HEX (located in subdir .pioenvs/teensy36)
//CALL TEENSY_REBOOT ... this will then directly upload the HEX
// ...change the source ... and recompile
//CALL TEENSY_REBOOT ... this will directly upload the changed HEX

/* CORBEE */
/* TEENSY 3.6 PINSETUP (20180901)
                 GND *                * Vin  - PREAMP V+  ----------------------R
                   0 .                *  Analog GN D      --------C-----R       R1
                   1 .                *  3.3V - MEMS MIC          C1    R2      R1
                   2 .                *  23 AUDIO -LRCLK          C1    R2      R1
       TFT-PWM     3 .                *  22 AUDIO -TX             C1    R2      R1  voltage dividor R2/(R1+R2) 
                   4 .                *  21 TFT CS                C1    R2      R1  
                   5 .                *  20 TFT DC                C1    R2      R1  
       AUDIO MEMCS 6 *                *  19 AUDIO - SCL           C1    R2      R1  ex. R1=15k, R2=2.2k C1=100nf(https://forum.pjrc.com/threads/25111-Decreasing-noise-on-Teensy-ADC) 
       AUDIO MOSI  7 *                *  18 AUDIO - SDA           C1    R2      R1
                   8 .                .  17 A3 - ADC -    --------C-----R-------R  
       AUDIO BCLK  9 *                .  16                                
       AUDIO SDCS 10 *                *  15 AUDIO -VOL                      
       AUDIO MCLK 11 *                *  14 AUDIO -SCLK                     
       AUDIO MISO 12 *                *  13 AUDIO -RX                       
                3.3V *                *  GND                 
                  24 .                .  A22
                  25 .                .  A21
                  26 .                *  39  TFT MISO
        TFT SCLK  27 *                *  38  MICROPUSH_L
        TFT MOSI  28 *                *  37  MICROPUSH_R
     ENC_L-BUTTON 29 *                *  36  ENC_R-BUTTON
     ENC_L A      30 *                *  35  ENC_R A
     ENC_L B      31 *                *  34  ENC_R B
                  32 .                .  33

*/

#define batversion "v0.98 20190901"
#define versionno 97 // used in EEProm storage
//PIO (platformio) or ARDUINO environment 
#define COMPILER PIO

#define USE_VIN_ADC
//#define USE_PWMTFT
//#define DEBUG_SERIAL

/* changes 
* v0.97/0.98 fixed bug in the replay mode: when choosing a file the replay-sample_rate changed also
        added routines to monitor VIN (A3 ADC on pin17 using a voltage divider and capacitor) see #define USE_VIN_ADC
        added routine to control TFT PWM from pin3, see #define USE_PWMTFT
        added compiler option PIO (platformiobased project) or ARDUINO (teensyduino/arduino IDE) 

* v0.96 MAJOR CHANGES !!!
        - compacted the main menu to have all less often changed settings in a separate SETTINGS page
        - setting of both time and date can be done from the new settings page
        - updated startup from EEprom stored settings (more settings stored (see settings page))
        - improved spectrum-graph functionality (peak-hold now visibible, was present for many versions but invisible)
        
        - made as many as possible user-changeable options cyclic (if the user tries to rotate the encoder above the highest value automatically go to the lowest value)
        - time was updating still on screen every minute during recording, now has been removed
        - added a routine to monitor VIN (or anything else) voltage on A3 (pin17)
        - read unique SN (allows identifying different detectors, one for testing with additional functions)
         

* v0.94 important changes:
        - Lower menu format on screen now follows the alignment of the encoders/buttons on Edwins PCB. 
        So the encoders control the lower line on bottom part of the screen(SETTINGS) and the buttons control the upper line on the bottom 
        of the screen (MODES).
        -EEprom saving was not always done when asked to restart in the user mode
        -when no SD card was mounted the Left Encodermenu was not cyclic (jumped back to volume)
        -when no sD card was mounted user could still set the LEFT button to RECORD or PLAY

        minor changes:
        -store the detectormode when starting a recording and restore after stopping a recording
        - bug repair:
          in previous versions the recording system was not checking if a filename allready existed but only 
          checked the number of files on the SD. If a file was removed (for instance from a PC) this would lead to 
          overwriting existing files. 
          In the new setup the new filenumber for a recording will be checked against highest filenumber in the
          allready stored files.

* v0.93 minor changes:
         changing SR was not stored properly in previous versions
        added system-voltage in startup-display (for future battery managment)

* v0.92 -further work on the setup to record/play files NEEDS TESTING 
        -filename display shortened so no overlap with other menu
        -filename selection (when playing) is now cyclic. 
         To go to the latest recording just turn counterclockwise instead of stepping through all previous 
        -rightside menu allows setting of samplerate (in all modes)
        -during playing of files microphones will be switched off
        -maximum number of files to be stored 999 (BXXX_RRR.RAW) XXX=0..999 RRR=samplerate in Khz
                
* v0.91 IMPORTANT :solved the issues with replay that were present since v0.86

* v0.90 added define to allow encoder reversal (some encoders work counter-clockwise) 
        menu cleaned (removed spectrum, was not active. changed sequence of menu)
        maximum filecount 99 allowed to save from now on

* v0.89 indicator for low-high detectionrange in graph for AUTO_TE
        menu updated and more uniform (all choosen values shown in white next to menusetting also for TE_LOW/TE_SPD)
        added counter to keep track of EEprom saving (still reset to 0 for each EEprom version)
        added text to display after pressing PRESET USER to save to EEprom
        
* v0.88 EEprom setup added
        

* v0.87 allow to set low-detection frequency for auto_te via MENU TE_LOW
        allow the auto_te divider to be set via MENU TE_SPEED
        changed detection routine for auto_te, removed the denoising section
* v0.86 20190623  
  WMXZ added changes to remove issues when both using SD.h and uSDFS.h, no need for ff_utils anymore
* v0.85 20190623 
  added ff_utils.h and ff_utils.c (ff_utils_copy) to the main structure to remove depences from uSDFS 
  changed seconds2time function into seconds2time
*/

/***********************************************************************
 *  TEENSY 3.6 BAT DETECTOR 
 *  Copyright (c) 2019, Cor Berrevoets, registax@gmail.com
 *
 *  HARDWARE USED:
 *     TEENSY 3.6
 *     TEENSY audio board
 *     Ultrasonic microphone with seperate preamplifier connected to mic/gnd on audioboard
 *       eg. Knowles MEMS SPU0410LR5H-QB
 *           ICS - 40730 
 *     TFT based on ILI9341
 *     2 rotary encoders with pushbutton
 *     2 pushbuttons
 *     SDCard
 * 
 *  when using a GX16-4 connector for the microphone: pinout 1=signal, 2=GND, 3=+V, 4=GND 
 *
*   IMPORTANT: uses the SD card slot of the Teensy, NOT the SD card slot of the audio board
 *
 *  4 operational modes: Heterodyne.
 *                       Frequency divider
 *                       Automatic heterodyne (1/10 implemented)
 *                       Automatic TimeExpansion (live)
 *                       Passive (no processing)
 *
 *  Sample rates up to 352k
 *
 *  
 *  Record raw data
 *  Play raw data (user selectable) on the SDcard using time_expansion 
 *
 *  
 * **********************************************************************
 *   Based on code by DD4WH
 *
 *   https://forum.pjrc.com/threads/38988-Bat-detector
 *
 *   https://github.com/DD4WH/Teensy-Bat-Detector
 *
 *   made possible by the samplerate code by Frank Boesing, thanks Frank!
 *   Audio sample rate code - function setI2SFreq
 *   Copyright (c) 2016, Frank Bösing, f.boesing@gmx.de
 *
 * 
 * **********************************************************************
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice, development funding notice, and this permission
 * notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 **********************************************************************/

#include <Arduino.h>
#ifndef __MKL26Z64__
	// only SIM_UIDML will probably change !!
  unsigned long chipNum[4] = { SIM_UIDH, SIM_UIDMH, SIM_UIDML, SIM_UIDL };
#else
	unsigned long chipNum[4] = { 0L, SIM_UIDMH, SIM_UIDML, SIM_UIDL };
#endif


//USE a TFT (this code will not function properly without !!! Currently only setup for ILI9341 )
#define USETFT

#define USESD
#define USESD1

#define USERECORD //allows to remove recording section from software (used for testing )
#define USEEEPROM //allows to remove EEprom section (used for testing)

// *************************** LIBRARIES **************************
#if COMPILER==PIO
  #include <TimeLib.h>
#endif
#if COMPILER==ARDUINO
  #include <TimeAltLib.h>
#endif

#include "Audio.h"
#include <SPI.h>
#include <Bounce.h>

#include "sec2time.h" // separate time routine library
// *************************** VARS  **************************

String versionStr=batversion;

boolean SD_ACTIVE=false;
boolean continousPlay=false;
boolean recorderActive=false;
boolean playActive=false;
boolean Ultrasound_detected=false;//triggers when an ultrasonic signalpeak is found during FFT
boolean TE_ready=true; //when a TEcall is played this signals the end of the call

// ****************************************************  FILE SYSTEMS

int filecounter=0;
int filemax=0;
char filename[80];
char shortfilename[13];

// ************************************  TIME *****************************
time_t getTeensy3Time()
{  return Teensy3Clock.get();
}

int helpmin; // definitions for time and date adjust - Menu
int helphour;
int helpday;
int helpmonth;
int helpyear;
int helpsec;
uint8_t old_time_min=0;
uint8_t old_time_sec=0;


uint8_t hour10_old;
uint8_t hour1_old;
uint8_t minute10_old;
uint8_t minute1_old;
uint8_t second10_old;
uint8_t second1_old;
bool timeflag = 0;
uint32_t lastmillis;

// **************************** TIMER VARS ********************************************

//continous timers
elapsedMillis started_detection; //start timing directly after FFT detects an ultrasound
elapsedMillis end_detection; //start timing directly after FFT detects the end of the ultrasound
elapsedMillis since_heterodyne=1000; //timing interval for auto_heterodyne frequency adjustments
uint16_t callLength=0;
//uint16_t clicker=0;

// ************************************  TFT *****************************
// TFT definitions currently only setup for ILI9341

#ifdef USETFT
 #define ILI9341
 #ifdef ILI9341
  #include "ILI9341_t3.h"
  #include "font_Arial.h"
  
  
 //see https://www.pjrc.com/store/display_ili9341_touch.html 

  #define BACKLIGHT_PIN 255 //backlight is controlled by hardware 
  #define TFT_DC      20
  #define TFT_CS      21
  #define TFT_RST     255  // 255 = unused. connect to 3.3V
  #define TFT_MOSI    28
  #define TFT_SCLK    27
  #define TFT_MISO    39

  ILI9341_t3 tft = ILI9341_t3(TFT_CS, TFT_DC, TFT_RST, TFT_MOSI, TFT_SCLK, TFT_MISO);

  //#define Touch_CS    8
  //XPT2046_Touchscreen ts = XPT2046_Touchscreen(Touch_CS);
  //predefine menu background etc colors

//setup of the SCREEN BLOCKS for a 240wide and 320high TFT
  //**********************
  // SETTINGS AND TIME   *
  //                     *
  // POWERGRAPH          * TOPOFFSET 
  //                     *
  //       GRAPH         * 
  //                     *
  // PUSHBUTTON SETTING  * BOTTOMOFFSET
  // ENCODER SETTING     *
 //***********************

  #define TOP_OFFSET 90
  #define POWERGRAPH 45
  #define SPECTRUMSCALE 5

  #define BOTTOM_OFFSET 40      // split the BOTTOMOFFSET into 2 sections, this is the highest 
  #define BOTTOM_OFFSET_PART 20 // this is the lowest

  //default menu colors

  #define ENC_MENU_COLOR COLOR_YELLOW
  #define ENC_VALUE_COLOR COLOR_LIGHTGREY
  #define MENU_BCK_COLOR COLOR_DARKRED
  #define MENU_BCK_COLOR_PART COLOR_DARKGREEN

 #endif
#endif


// ********************************************* SD declarations 
#ifdef USESD
  #define MAX_FILES    999
  #define MAX_FILE_LENGTH  13   // 8 chars plus 4 for.RAW plus NULL
  char filelist[ MAX_FILES ][ MAX_FILE_LENGTH];
  int fileselect=0;
  int referencefile=0;
#endif

// ******** LEFT AND RIGHT ENCODER CONNECTIONS/BUTTONS

/************** LButton Mode SETTINGS *****************/
const int8_t    MODE_DISPLAY =0;  //default left
const int8_t    MODE_REC = 1; //
const int8_t    MODE_PLAY = 2; //
const int8_t    MODE_MAX=3; // limiter for leftbutton

const int8_t    MODE_DETECT = 5; // default right

#include <Encoder.h>
//try to avoid interrupts as they can (possibly ?) interfere during recording

#define ENCODER_DO_NOT_USE_INTERRUPTS

#define MICROPUSH_RIGHT  37
Bounce micropushButton_R = Bounce(MICROPUSH_RIGHT, 50);
#define encoderButton_RIGHT      36
Bounce encoderButton_R = Bounce(encoderButton_RIGHT, 50);
Encoder EncRight(34,35);
int EncRightPos=0;
int EncRightchange=0;

#define MICROPUSH_LEFT  38
Bounce micropushButton_L = Bounce(MICROPUSH_LEFT, 50);
#define encoderButton_LEFT       29
Bounce encoderButton_L = Bounce(encoderButton_LEFT, 50);
Encoder EncLeft(30,31);
int EncLeftPos=0;
int EncLeftchange=0;

//************************* ENCODER variables/constants
const int8_t enc_menu=0; //changing encoder sets menuchoice
const int8_t enc_value=1; //changing encoder sets value for a menuchoice

const int8_t enc_leftside=0; //encoder
const int8_t enc_rightside=1; //encoder
// ENCODER TURN 1 (clockwise). Use ENCODER_TURN -1 for counter-clockwise
static int8_t ENCODER_TURN= 1;

static int8_t enc_up=1*ENCODER_TURN; //encoder goes up
static int8_t enc_nc=0;
static int8_t enc_dn=-1*ENCODER_TURN; //encoder goes down

int EncLeft_menu_idx=0;
int EncRight_menu_idx=0;

int EncLeft_function=0;
int EncRight_function=0;

uint8_t LeftButton_Mode=MODE_DISPLAY;
uint8_t LeftButton_Next=MODE_DISPLAY;
uint8_t RightButton_Mode=MODE_DETECT;

// **END************ LEFT AND RIGHT ENCODER DEFINITIONS

// *********************************** AUDIO SGTL5000 SETUP *******************************
// this audio comes from the codec by I2S2
AudioInputI2S                    i2s_in; // MIC input
AudioRecordQueue                 recorder;
AudioSynthWaveformSineHires      sine1; // local oscillator
//AudioFilterBiquad                BiQuad1;
AudioEffectMultiply              heterodyne_multiplier; // multiply = mix
AudioAnalyzeFFT256               myFFT; // for spectrum display
AudioPlaySdRaw                   player;
AudioEffectGranular              granular1;
//mAudioEffectGranular             granular1; change by WMXZ
AudioMixer4                      mixFFT;
AudioMixer4                      outputMixer; //selective output
AudioMixer4                      inputMixer; //selective input
AudioOutputI2S                   i2s_out; // headphone output

//AudioConnection mic_BiQuad          (i2s_in, 0, BiQuad1, 0); //microphone signal
AudioConnection mic_toinput         (i2s_in, 0, inputMixer, 0); //microphone signal
//AudioConnection BiQuad_toinput      (BiQuad1, 0, inputMixer, 2); //microphone signal

AudioConnection mic_torecorder      (i2s_in, 0, recorder, 0); //microphone signal
AudioConnection player_toinput      (player, 0, inputMixer, 1); //player signal
AudioConnection input_toswitch      (inputMixer,0,  mixFFT,0);
AudioConnection input_todelay       (inputMixer,0, granular1, 0);
AudioConnection switch_toFFT        (mixFFT,0, myFFT,0 ); //raw recording channel
AudioConnection input_toheterodyne1 (inputMixer, 0, heterodyne_multiplier, 0); //heterodyne 1 signal
AudioConnection sineheterodyne1     (sine1, 0, heterodyne_multiplier, 1);//heterodyne 1 mixerfreq
AudioConnection granular_toout      (granular1,0, outputMixer,1);
AudioConnection heterodyne1_toout   (heterodyne_multiplier, 0, outputMixer, 0);  //heterodyne 1 output to outputmixer
AudioConnection player_toout        (inputMixer,0, outputMixer, 2);    //direct signal (use with player) to outputmixer
AudioConnection output_toheadphoneleft      (outputMixer, 0, i2s_out, 0); // output to headphone
AudioConnection output_toheadphoneright     (outputMixer, 0, i2s_out, 1);

AudioControlSGTL5000        sgtl5000;

//const int myInput = AUDIO_INPUT_LINEIN;
const int myInput = AUDIO_INPUT_MIC;

// ****************** TEENSY MODEL BASED BUFFERS ***********************************
// WARNING: this setup uses a lot of memory in buffers (2x 96K !!)
// TODO: check if memory is available !!

#if defined(__MK20DX256__)
  #define BUFFSIZE (8*1024) // size of buffer  to be written
#elif defined(__MK66FX1M0__) // Teensy 3.6
  #define BUFF 96
  #define BUFFSIZE (BUFF*1024) // size of buffer to be written
#endif

// buffer to store audiosamples during recording
uint8_t sample_buffer[BUFFSIZE] __attribute__( ( aligned ( 16 ) ) );
//uint8_t sample_buffer2[BUFFSIZE] __attribute__( ( aligned ( 16 ) ) );
uint wr;
uint32_t nj = 0;

// ******************** BUFFER FOR TIMEEXPANSION ROUTINE ********************
#define GRANULAR_MEMORY_SIZE 16*1024  // enough for 50 ms at 281kHz sampling
int16_t granularMemory[GRANULAR_MEMORY_SIZE];

// ******************** DISPLAY MODES ********************

#define debug_page 4 //debug page (TODO) will show active debug messages 
#define settings_page 3 //special settings page
#define waterfallgraph 2
#define spectrumgraph 1
#define no_graph 0

const char* DP [5] =
  {
    "no graph",
    "spectrum",
    "waterfall",
    "settings",
    "debug"
  };


// *********************************SAMPLING ****************************

#define SR_MIN               0
#define SR_8K                0
#define SR_11K               1
#define SR_16K               2
#define SR_22K               3
#define SR_32K               4
#define SR_44K               5
#define SR_48K               6
#define SR_88K               7
#define SR_96K               8
#define SR_176K              9
#define SR_192K              10
#define SR_234K              11
#define SR_281K              12
#define SR_352K              13
#define SR_MAX               13

// ***************** SAMPLE RATE DESCRIPTION
typedef struct SR_Descriptor
{   const int SR_n;
    const char* txt; //text for the display
    const u_int osc_frequency;
} SR_Desc;

// SRtext and position for the FFT spectrum display scale
const SR_Descriptor SR [SR_MAX + 1] =
{
    {  SR_8K,  "8" ,     8000},
    {  SR_11K,  "11",   11025},
    {  SR_16K,  "16",   16000},
    {  SR_22K,  "22",   22050},
    {  SR_32K,  "32",   32000},
    {  SR_44K,  "44",   44100},
    {  SR_48K,  "48",   48000},
    {  SR_88K,  "88",   88200},
    {  SR_96K,  "96",   96000},
    {  SR_176K,  "176", 176400},
    {  SR_192K,  "192", 192000},
    {  SR_234K,  "234", 234000},
    {  SR_281K,  "281", 281000},
    {  SR_352K,  "352", 352800}
};

// default samplerates 

#define MAX_PLY_SR SR_44K

int oper_SR = SR_281K;
int rec_SR = SR_281K;
int ply_SR = SR_22K;

int SR_real = SR[oper_SR].osc_frequency;
//int last_SR=oper_SR;
const char * SRtext=SR[oper_SR].txt;

// ****************************** defaults SETTINGS at startup 
int startup_display=spectrumgraph;
int display_mode=spectrumgraph; //default display
int last_display_mode=spectrumgraph;
int8_t mic_gain = 30; // start detecting with this MIC_GAIN in dB
int8_t volume=50;
//int8_t TE_speed=20; // moved to EEprom section
//int8_t TE_low=15 ; //move to EEprom section

int osc_frequency = 45000; // start heterodyne detecting at this frequency
int last_osc_frequency=osc_frequency; //used to return to proper listening setting after using the play_function
float freq_Oscillator =50000;

int16_t EEsaved_count=0;
int8_t TE_speed=20;
int8_t TE_low=15 ;
int8_t param=10;

int preset_idx=1; //0= default values; 1=user values;
#ifdef USE_VIN_ADC
  int VIN=0; //VIN from ADC
  #include <ADC.h>
  ADC *adc = new ADC();
#endif

// *********************************************  SPECIAL FUNCTIONS
//adapted constrain function to allow cycling values initial functioncopied from wiring.h
#define cyclic_constrain(amt, low, high) ({ \
  typeof(amt) _amt = (amt); \
  typeof(low) _low = (low); \
  typeof(high) _high = (high); \
  (_amt < _low) ? _high : ((_amt > _high) ? _low : _amt); \
})

/******************* MAIN MENU ********************************/
/*********************************************************/

//TODO constants should be part of the menuentry, a single structure to hold the info#
#define  MENU_VOL   0 //volume
#define  MENU_MIC   1 //mic_gain
#define  MENU_FRQ   2 //frequency
#define  MENU_SR    3 //sample rate
#define  MENU_REC   4 //record
#define  MENU_PLAY  5 //play
#define  MENU_PLD   6 //play at original rate
#define  MENU_SETTINGS 7 // SETTINGS menu

#define  MENU_MAX MENU_SETTINGS

const int Leftchoices=MENU_MAX+1; //can have any value
const int Rightchoices=MENU_SR+1; //allow up to SR
const char* MenuEntry [Leftchoices] =
  {
    "Vol.",  //L and R
    "Gain",
    "Freq.",
    "SRate", 
    "Record", //only L
    "Play",
    "PlayD",
    "Settings"
  };

// ************************************************    settings menu
// THIS MENU STARTS at TOP_OFFSET-45 (currently at 45) and uses 20 pixels/line. The bottom should be above BOTTOM_OFFSET (320-40=280) so
// there is space for 280-45=245/20=12 lines with Arial_16
// goto a menuline on screen and clear it if its selected
int setmenu_pos=0;
#define SETMENU_DISPLAY 0
#define SETMENU_DETECTOR 1
#define SETMENU_STARTUPMODE 2
#define SETMENU_ENC   3 //encoder CW CCW
#define SETMENU_TE_LOW   4 //TE_LOW
#define SETMENU_TE_SPD   5 //TE_SPD
#define SETMENU_SR 6 //Sample Rate
#define SETMENU_SR_REC 7 //Sample Rate
#define SETMENU_SR_PLY 8 //Sample Rate
#define SETMENU_TIME  9
#define SETMENU_DATE 10
#define SETMENU_PARAM 11

#define SETMENU_MIN 0
#define SETMENU_MAX 11 // MAXIMIZE AT 10 !!!! with ARIAL 16

const char* SetMenuEntry [SETMENU_MAX+1] =
  {
    "Display ",  //which displaymode to start with
    "Detector ", //which detector
    "Settings ", //read the settings from EEprom or use the defaults
    "Encoder ",
    "TExp Lowest Frq ",
    "TExp Replay Spd ",
    "SampleRate ",
    "SampleRate rec ",
    "SampleRate play ",
    "Time ",
    "yyyyymmdd ",
    "param"

  };


//***************************************** available DETECTOR modes
#define detector_heterodyne 0
#define detector_Auto_heterodyne 1
#define detector_Auto_TE 2 // time expansion
#define detector_divider 3
#define detector_passive 4 // no processing at all

const char* DT [5] =
  {
    "Heterodyne",
    "A-heterodyne",
    "A-T.Expansion",
    "Freq. divider",
    "Passive"
    
  };

//default 
int startup_detector=detector_heterodyne;
int detector_mode=detector_heterodyne;
int last_detector_mode=detector_heterodyne;


// ***************************** FFT SETUP *******************************

long FFT_count=0;
int idx_t = 0;
int idx = 0;
int64_t sum;
float32_t mean;
int16_t FFT_bin [128];
//TEST
int16_t FFT_max1 = 0;
uint32_t FFT_max_bin1 = 0;
int16_t FFT_mean1 = 0;
int16_t FFT_max2 = 0;
uint32_t FFT_max_bin2 = 0;
int16_t FFT_mean2 = 0;
//int16_t FFT_threshold = 0;
int16_t FFT_bat [3]; // max of 3 frequencies are being displayed
int16_t index_FFT;
int l_limit;
int u_limit;
int index_l_limit;
int index_u_limit;
//const uint16_t FFT_points = 1024;
const uint16_t FFT_points = 256;
int barm [512];

uint16_t powerspectrumCounter=0;

float FFTavg[128];

float FFTpowerspectrum[128];
float powerspectrum_Max=0;


// ******************************************** DISPLAY

// ************** FUNCTIONS  //
int calc_menu_dxoffset(const char* str)  // to position the menu on the right screenedge
{
#ifdef USETFT
  String s=String(str); 
  char charBuf[50];
  s.toCharArray(charBuf, 50) ;
  uint16_t dx=tft.strPixelLen(charBuf);
  return dx; 
#endif
}

//SETTINGS MENU STARTS at TOP_OFFSET-45 (currently at 45) and uses 20 pixels/line. The bottom should be above BOTTOM_OFFSET (320-40=280) so
// there is space for 280-45=245/20=12 lines with Arial_14
// goto a menuline on screen and clear it if its selected


void SETMenu_line(int line) // position the cursor, clear the line if this is an active menuline or otherwise just display the text
                            // TODO: optimize to only update the necessary lines instead of rewriting all
        { 
          int h = Arial_14.cap_height+3;
          tft.setCursor(0,TOP_OFFSET-45+line*h);
          if (line==setmenu_pos)
            {tft.setTextColor(COLOR_WHITE);
             tft.fillRect(0,TOP_OFFSET-45+line*h,240,h,COLOR_BLACK);
            }
          else
          { tft.setTextColor(COLOR_YELLOW);
          }
          tft.print(SetMenuEntry[line]);
         
        }

void showMenu_value(const char *str)
        {
          uint16_t dx=calc_menu_dxoffset(str);
          int y=tft.getCursorY();
          tft.setCursor(ILI9341_TFTWIDTH-dx,y);
          tft.print(str);
        }
//**************************************  MAIN DISPLAY UPDATE 
void update_display() {
  #ifdef USETFT
  //default updates

    tft.setTextColor(ENC_MENU_COLOR);
    tft.setFont(Arial_16);
    //clear the top
    tft.fillRect(0,0,240,TOP_OFFSET-50,MENU_BCK_COLOR);
    tft.drawLine(0,TOP_OFFSET-50, 240, TOP_OFFSET-50, COLOR_WHITE);

    //clear the grapharea above the live graphs
    if (display_mode==spectrumgraph)
      { tft.fillRect(0,TOP_OFFSET-30,240,30,COLOR_BLACK);
      }
    if (display_mode==waterfallgraph)  
    { tft.fillRect(0,TOP_OFFSET-6,240,6,COLOR_BLACK);
      }
    //clear the bottompart  
    tft.fillRect(0,ILI9341_TFTHEIGHT-BOTTOM_OFFSET,240,BOTTOM_OFFSET_PART,MENU_BCK_COLOR);
    tft.fillRect(0,ILI9341_TFTHEIGHT-BOTTOM_OFFSET_PART,240,BOTTOM_OFFSET,MENU_BCK_COLOR_PART);

    //show the current settings
    tft.setCursor(0,0);
    tft.print("v"); tft.print(volume);
    tft.print(" g"); tft.print(mic_gain);
    if (display_mode==waterfallgraph)
       {tft.print(" f"); tft.print(int(osc_frequency/1000));
       }
    tft.print(" s"); tft.print(SRtext);

    //show the detectormode and time
    tft.setCursor(0,20);

    switch (detector_mode) {
       case detector_heterodyne:
         tft.print("HT"); //
       break;
       case detector_divider:
         tft.print("FrqDiv");
       break;
       case detector_Auto_heterodyne:
         tft.print("Auto_HT");
       break;
       case detector_Auto_TE:
        tft.print("Auto_TE");
       break;
       case detector_passive:
        tft.print("PASS");
       break;
       default:
        tft.print("error");
     }
      
      char tstr[9];
      
      
      #ifdef USE_VIN_ADC
      tft.setCursor(180,0);
      if (VIN<4500) 
            { snprintf(tstr,9, "L%4d", VIN-3700);
        tft.print(tstr);
      }
      else
      {
         tft.print("USB");
      }
      
      #endif

      tft.setCursor(180,20);
      struct tm tx = seconds2time(RTC_TSR);
      
      tft.setTextColor(ENC_MENU_COLOR);
      //if the user is editing the time show seconds also
      snprintf(tstr,9, "%02d:%02d", tx.tm_hour, tx.tm_min);
      tft.print(tstr);

    

    // *********************** SETTINGS PAGE ****************************
    if (display_mode==settings_page) // display user settings
      { 
        //position just below the top  

        tft.setCursor(0,TOP_OFFSET-45);
        tft.setTextColor(COLOR_YELLOW); //default color
        tft.setFont(Arial_14); //default font allows 12 lines of settings
        
        setmenu_pos=cyclic_constrain(setmenu_pos,SETMENU_MIN,SETMENU_MAX);
        char tstr[9];

        SETMenu_line(SETMENU_ENC);
        if (ENCODER_TURN==1)
          { snprintf(tstr,9, "Clockw");
          }
        else
           {snprintf(tstr,9, "C-Clockw");
           }
        showMenu_value(tstr);
         


        SETMenu_line(SETMENU_TE_LOW);
        snprintf(tstr,9, "%02d", TE_low);
        showMenu_value(tstr);
      
        SETMenu_line(SETMENU_TE_SPD);
        snprintf(tstr,9, "1/%02d", TE_speed);
        showMenu_value(tstr);

        SETMenu_line(SETMENU_SR);
        showMenu_value(SR[oper_SR].txt);
        //tft.println(SR[oper_SR].txt);

        SETMenu_line(SETMENU_SR_REC);
        showMenu_value(SR[rec_SR].txt);

        SETMenu_line(SETMENU_SR_PLY);
        showMenu_value(SR[ply_SR].txt);

        SETMenu_line(SETMENU_DISPLAY);
        showMenu_value(DP[startup_display]);

        SETMenu_line(SETMENU_DETECTOR);
        showMenu_value(DT[startup_detector]);

        SETMenu_line(SETMENU_TIME);
        
        struct tm tx = seconds2time(RTC_TSR);
        snprintf(tstr,9, "%02d:%02d:%02d", tx.tm_hour, tx.tm_min, tx.tm_sec);
        showMenu_value(tstr);
        //tft.print(tstr);
        
        SETMenu_line(SETMENU_DATE);
        tx = seconds2time(RTC_TSR);
        snprintf(tstr,9, "%04d%02d%02d", tx.tm_year+1970, tx.tm_mon, tx.tm_mday);
        showMenu_value(tstr);
        //tft.print(tstr);
        
        SETMenu_line(SETMENU_STARTUPMODE);
        if (preset_idx==0)
             {snprintf(tstr,9, "Default");
              }
             else
             {snprintf(tstr,9, "EEprom");
                };
        showMenu_value(tstr);

        SETMenu_line(SETMENU_PARAM);
        snprintf(tstr,9, "%04d", param);
        showMenu_value(tstr);

        
      }  
    
     //BOTTOM PART OF SCREEN      
     
     /****************** SHOW ENCODER/BUTTON SETTING ***********************/
     int LINE2=BOTTOM_OFFSET;  // allows switch of menuline positions
     int LINE1=BOTTOM_OFFSET_PART;

     // **************************** DISPLAY ENCODER FUNCTIONS/VALUE
     tft.setCursor(0,ILI9341_TFTHEIGHT-LINE1); //display of encoder functions
       
     // set the colors according to the function of the encoders
     //if (RightButton_Mode==MODE_DETECT )
     // show menu selection as menu-active of value-active
     if (display_mode!=settings_page)
      {
       if (EncLeft_function==enc_value)
        { tft.setTextColor(ENC_VALUE_COLOR);
         }
        else
        { tft.setTextColor(ENC_MENU_COLOR);
         }
      
       if ((EncLeft_menu_idx==MENU_PLAY) and (EncLeft_function==enc_value)) // when play selected only show filename
           { 
             #ifdef USESD
             char* ch = strchr(filelist[fileselect], '.');
             int cout = ch - filelist[fileselect] + 1; 
             
             strlcpy(shortfilename, filelist[fileselect], cout);
             tft.print(shortfilename); 
             
             #endif
           }
       else 
       {  tft.print(MenuEntry[EncLeft_menu_idx]);
          tft.print(" ");
       }
       
       tft.setTextColor(COLOR_WHITE);

       if (EncLeft_menu_idx==MENU_MIC)
          { tft.print(mic_gain);
          } 

       if (EncLeft_menu_idx==MENU_FRQ)
          { tft.print(int(osc_frequency/1000));
          } 
       if (EncLeft_menu_idx==MENU_VOL)
          { tft.print(volume);
          }

      //  if (EncLeft_menu_idx==MENU_TEL)
      //     { tft.print(TE_low);
      //     } 

      //  if (EncLeft_menu_idx==MENU_TES)
      //     { tft.print(TE_speed);
      //     } 

       if (EncLeft_menu_idx==MENU_SR)
          { tft.print(SR[oper_SR].txt);
          }    
       
       if (EncRight_function==enc_value) 
         { tft.setTextColor(ENC_VALUE_COLOR);} //value is active
       else
         { tft.setTextColor(ENC_MENU_COLOR);} //menu is active

       uint16_t dx=calc_menu_dxoffset((MenuEntry[EncRight_menu_idx]));
       tft.setCursor(ILI9341_TFTWIDTH-dx,ILI9341_TFTHEIGHT-LINE1);
       tft.print(MenuEntry[EncRight_menu_idx]);
       
    } // END of regular SETTINGS

    if (display_mode==settings_page) // special setup for settings
    { tft.setTextColor(ENC_VALUE_COLOR);
      tft.print("Select");
      uint16_t dx=calc_menu_dxoffset("Change/Save"); 
      tft.setCursor(ILI9341_TFTWIDTH-dx,ILI9341_TFTHEIGHT-LINE1);
      tft.print("Change/Save");
    }
    
    // ****************************** DISPLAY PUSHBUTTON FUNCTIONS 
    tft.setCursor(0,ILI9341_TFTHEIGHT-LINE2); //position of button functions
    tft.setTextColor(COLOR_YELLOW);
    
    if (LeftButton_Mode==MODE_DISPLAY)
        tft.print("DISPLAY");

    if (LeftButton_Mode==MODE_REC)
        { if (recorderActive)  
            {   tft.print("REC ON");
                }
            else
            {   tft.print("REC OFF");
            }
                
        }
    if (LeftButton_Mode==MODE_PLAY)
        { if (playActive==false)
            { tft.print("PLAY");
            }
            else
            { tft.print("STOP");
            }
            }
    

    tft.setTextColor(COLOR_YELLOW);

    char* s=(char*)"MODE";
    //currently v0.81 only DETECT 
    if (RightButton_Mode==MODE_DETECT) 
       s=(char*)"DETECT";

    if (playActive==false) //dont show rightside menu choice when playing a file to allow the display of a filename
     {uint16_t sx=tft.strPixelLen(s);
      tft.setCursor(ILI9341_TFTWIDTH-sx-5,ILI9341_TFTHEIGHT-LINE2);
      tft.print(s);
     }  

    // show a scale with ticks for every 10kHz except for no_graph or settings_page
    if (display_mode!=settings_page)
     {if (display_mode>0)
      { float x_factor=10000/(0.5*(SR_real / FFT_points));
        int curF=2*int(osc_frequency/(SR_real / FFT_points));
        
        int maxScale=int(SR_real/20000);
        for (int i=1; i<maxScale; i++)
        { tft.drawFastVLine(i*x_factor, TOP_OFFSET-SPECTRUMSCALE, SPECTRUMSCALE, ENC_MENU_COLOR);
        }
        // detection range
        int curLo=int(TE_low*1000/(SR_real / FFT_points));
        int curHi=int(80000/(SR_real / FFT_points));
        if (detector_mode==detector_Auto_TE) //show detection band
          { tft.drawFastHLine(2*curLo,TOP_OFFSET-SPECTRUMSCALE,2*(curHi-curLo),COLOR_WHITE);
            tft.drawFastHLine(2*curLo,TOP_OFFSET-1,2*(curHi-curLo),COLOR_WHITE);
          }

        if (curF<ILI9341_TFTWIDTH-40)
          {
          
        tft.fillCircle(curF,TOP_OFFSET-3,3,ENC_MENU_COLOR);
                
        if (display_mode==spectrumgraph)
          { //tft.drawFastVLine(curF,TOP_OFFSET-20,20,ENC_MENU_COLOR);
            char tstr[9];
            snprintf(tstr,9, "%02d", int(osc_frequency/1000));
            tft.setCursor(curF-10,TOP_OFFSET-30);  
            tft.print(tstr);
          }
          }
      }
      else 
        { tft.setCursor(0,TOP_OFFSET-SPECTRUMSCALE);
          tft.print("NO GRAPHS");
        }
     }

   #endif
}


// *************************************************** FUNCTIONS **************************************
void       set_mic_gain(int8_t gain) {

    AudioNoInterrupts();
    sgtl5000.micGain (gain);
    AudioInterrupts();
    update_display();
    powerspectrum_Max=0; // change the powerspectrum_Max for the FFTpowerspectrum
} // end function set_mic_gain

void       set_volume(int8_t vol) {
    AudioNoInterrupts();
    sgtl5000.volume (vol*0.01);
    AudioInterrupts();
       
} // end function set_mic_gain


// ***************************************************** OSCILLATOR
void       set_freq_Oscillator(int freq) {
    // audio lib thinks we are still in 44118sps sample rate
    // therefore we have to scale the frequency of the local oscillator
    // in accordance with the REAL sample rate

    freq_Oscillator = (freq) * (AUDIO_SAMPLE_RATE_EXACT / SR_real);
    //float F_LO2= (freq+5000) * (AUDIO_SR_EXACT / SR_real);
    // if we switch to LOWER samples rates, make sure the running LO
    // frequency is allowed ( < 22k) ! If not, adjust consequently, so that
    // LO freq never goes up 22k, also adjust the variable osc_frequency
    if(freq_Oscillator > 22000) {
      freq_Oscillator = 22000;
      osc_frequency = freq_Oscillator * (SR_real / AUDIO_SAMPLE_RATE_EXACT) + 9;
    }
    AudioNoInterrupts();
    sine1.frequency(freq_Oscillator);
    AudioInterrupts();
    update_display();
} // END of function set_freq_Oscillator
// ***************************************************** I2S
// set samplerate code by Frank Boesing
void setI2SFreq(int freq) {
  typedef struct {
    uint8_t mult;
    uint16_t div;
  } tmclk;
//MCLD Divide sets the MCLK divide ratio such that: MCLK output = MCLK input * ( (FRACT + 1) / (DIVIDE + 1) ).
// FRACT must be set equal or less than the value in the DIVIDE field.
//(double)F_PLL * (double)clkArr[iFreq].mult / (256.0 * (double)clkArr[iFreq].div);
//ex 180000000* 1 /(256* 3 )=234375Hz  setting   {1,3} at 180Mhz

  const int numfreqs = 17;
  const int samplefreqs[numfreqs] = {  8000,      11025,      16000,      22050,       32000,       44100, (int)44117.64706 , 48000,      88200, (int)44117.64706 * 2,   96000, 176400, (int)44117.64706 * 4, 192000,  234000, 281000, 352800};

#if (F_PLL==16000000)
  const tmclk clkArr[numfreqs] = {{16, 125}, {148, 839}, {32, 125}, {145, 411}, {64, 125}, {151, 214}, {12, 17}, {96, 125}, {151, 107}, {24, 17}, {192, 125}, {127, 45}, {48, 17}, {255, 83} };
#elif (F_PLL==72000000)
  const tmclk clkArr[numfreqs] = {{32, 1125}, {49, 1250}, {64, 1125}, {49, 625}, {128, 1125}, {98, 625}, {8, 51}, {64, 375}, {196, 625}, {16, 51}, {128, 375}, {249, 397}, {32, 51}, {185, 271} };
#elif (F_PLL==96000000)
  const tmclk clkArr[numfreqs] = {{8, 375}, {73, 2483}, {16, 375}, {147, 2500}, {32, 375}, {147, 1250}, {2, 17}, {16, 125}, {147, 625}, {4, 17}, {32, 125}, {151, 321}, {8, 17}, {64, 125} };
#elif (F_PLL==120000000)
  const tmclk clkArr[numfreqs] = {{32, 1875}, {89, 3784}, {64, 1875}, {147, 3125}, {128, 1875}, {205, 2179}, {8, 85}, {64, 625}, {89, 473}, {16, 85}, {128, 625}, {178, 473}, {32, 85}, {145, 354} };
#elif (F_PLL==144000000)
  const tmclk clkArr[numfreqs] = {{16, 1125}, {49, 2500}, {32, 1125}, {49, 1250}, {64, 1125}, {49, 625}, {4, 51}, {32, 375}, {98, 625}, {8, 51}, {64, 375}, {196, 625}, {16, 51}, {128, 375} };
#elif (F_PLL==168000000)
  const tmclk clkArr[numfreqs] = {{32, 2625}, {21, 1250}, {64, 2625}, {21, 625}, {128, 2625}, {42, 625}, {8, 119}, {64, 875}, {84, 625}, {16, 119}, {128, 875}, {168, 625}, {32, 119}, {189, 646} };
#elif (F_PLL==180000000)
  const tmclk clkArr[numfreqs] = {{46, 4043}, {49, 3125}, {73, 3208}, {98, 3125}, {183, 4021}, {196, 3125}, {16, 255},   {128, 1875}, {107, 853},     {32, 255},   {219, 1604}, {1, 4},      {64, 255},     {219,802}, { 1,3 },  {2,5} , {1,2} };  //last value 219 802
#elif (F_PLL==192000000)
  const tmclk clkArr[numfreqs] = {{4, 375}, {37, 2517}, {8, 375}, {73, 2483}, {16, 375}, {147, 2500}, {1, 17}, {8, 125}, {147, 1250}, {2, 17}, {16, 125}, {147, 625}, {4, 17}, {32, 125} };
#elif (F_PLL==216000000)
  const tmclk clkArr[numfreqs] = {{32, 3375}, {49, 3750}, {64, 3375}, {49, 1875}, {128, 3375}, {98, 1875}, {8, 153}, {64, 1125}, {196, 1875}, {16, 153}, {128, 1125}, {226, 1081}, {32, 153}, {147, 646} };
#elif (F_PLL==240000000)
  const tmclk clkArr[numfreqs] = {{16, 1875}, {29, 2466}, {32, 1875}, {89, 3784}, {64, 1875}, {147, 3125}, {4, 85}, {32, 625}, {205, 2179}, {8, 85}, {64, 625}, {89, 473}, {16, 85}, {128, 625} };
#endif

  for (int f = 0; f < numfreqs; f++) {
    if ( freq == samplefreqs[f] ) {
      while (I2S0_MCR & I2S_MCR_DUF) ;
      I2S0_MDR = I2S_MDR_FRACT((clkArr[f].mult - 1)) | I2S_MDR_DIVIDE((clkArr[f].div - 1));
      return;
    }
  }
}
// ***************************************************** SAMPLE RATE
void  set_SR (int sr) {
  #ifdef DEBUG_SERIAL
     Serial.println("change SR");
  #endif
  SR_real=SR[sr].osc_frequency;
  SRtext=SR[sr].txt;
  AudioNoInterrupts();
  setI2SFreq (SR_real);
  delay(200); // this delay seems to be very essential !
  set_freq_Oscillator (osc_frequency);
  AudioInterrupts();
  delay(20);
  update_display();
}

// ***************************************************** SPECTRUM GRAPH

void spectrum() { // spectrum analyser code by rheslip - modified
#ifdef USETFT
  uint16_t OFFSET =ILI9341_TFTHEIGHT-BOTTOM_OFFSET-TOP_OFFSET;  //default 320 - 40 - 90 = 190
  int16_t FFT_sbin [128];
  // limit until bin 119 to prevent overflow
  for (int16_t x = 2; x < 119; x++) {
     FFT_sbin[x] = int(myFFT.output[x]);//-FFTavg[x]*0.9;
     int barnew = (FFT_sbin[x]) ;
     // this is a very simple first order IIR filter to smooth the reaction of the peak values of the bars
     int bar = 0.3 * barnew + 0.7 * barm[x];
     bar=constrain(bar,0,OFFSET-1);
     barnew=constrain(barnew,0,OFFSET-1);
     
     int g_x=x*2;

     int spectrumline=barm[x];
     int spectrumline_new=barnew;
     //build spectrum graph bars
     //start with the coloured part
     tft.drawFastVLine(g_x,TOP_OFFSET,spectrumline_new, COLOR_GREEN); // X, 90, length bar, colour
     //tft.drawFastVLine(g_x+1,TOP_OFFSET,spectrumline_new, COLOR_GREEN);
     //erase anything higher than the current value
     tft.drawFastVLine(g_x,TOP_OFFSET+spectrumline_new,OFFSET-spectrumline_new, COLOR_BLACK); // X, 90+length bar, 190- length-bar BLACK
     //tft.drawFastVLine(g_x+1,TOP_OFFSET+spectrumline_new,OFFSET-spectrumline_new, COLOR_BLACK);
     //for the maxima
     tft.drawFastVLine(g_x+1,TOP_OFFSET,OFFSET, COLOR_BLACK);//clear the full bar
     tft.drawFastVLine(g_x+1,TOP_OFFSET,spectrumline_new, COLOR_GREEN);  //draw the green
     if (spectrumline>4) 
        {tft.drawFastVLine(g_x+1,TOP_OFFSET+spectrumline-5,5, COLOR_WHITE); //draw the moving average peak-hold
        }
     
     //tft.drawFastVLine(g_x+1,TOP_OFFSET+spectrumline,OFFSET -spectrumline, COLOR_BLACK);
     barm[x] = bar;
  }
  #endif
}


// **************** General graph and detector selective functions *******************************************************
void update_Display(void) 
{
#ifdef USETFT 

// code for 256 point FFT
 if (myFFT.available()) {
  const uint16_t Y_OFFSET = TOP_OFFSET;
  static int count = TOP_OFFSET;
  
  // lowest frequencybin to detect as a batcall
  int batCall_LoF_bin= int((TE_low*1000.0)/(SR_real / FFT_points));
  // highest frequencybin to detect as a batcall
  int batCall_HiF_bin= int(80000.0/(SR_real / FFT_points));

  uint8_t spec_hi=120; //default 120
  uint8_t spec_lo=2; //default 2
  uint8_t spec_width=2;

  uint16_t FFT_pixels[240]; // maximum of 240 pixels, each one is the result of one FFT
  memset(FFT_pixels,0,sizeof(FFT_pixels)); //blank the pixels
 
    int FFT_peakF_bin=0;
    int peak=256;
    int avgFFTbin=0;
    // there are 128 FFT different bins only 120 are shown on the graphs
    FFT_count++;
    for (int i = spec_lo; i < spec_hi; i++) {
      int val = myFFT.output[i]*10 + 10; //v1
      avgFFTbin+=val;
      //detect the peakfrequency
      if (val>peak)
       { peak=val;
         FFT_peakF_bin=i;
        }
      if (val<5)
           {val=5;}
      
      uint8_t pixpos=(i-spec_lo)*spec_width;
      FFT_pixels[pixpos] = tft.color565(
               min(255, val*2), //very low values will show as red
              (val/3>255)? 255 : val/3, // up to 3*255 will be coloured a mix of red/green (yellow) 
               val/9>255 ?255: val/9 ); // uo to 9*255 will be coloured a mix of red/green/blue (white)

      for (int j=1; j<spec_width; j++)
         { FFT_pixels[pixpos+j]=FFT_pixels[pixpos];
         }
    }

   
    avgFFTbin=avgFFTbin/(spec_hi-spec_lo);

    if ((peak/avgFFTbin)<1.05) //very low peakvalue so probably a lot of noise, dont detect this as a peak
     { FFT_peakF_bin=0;
     }

  int powerSpectrum_Maxbin=0;
  // detected a peak in the bat frequencies
    if ((FFT_peakF_bin>batCall_LoF_bin) and (FFT_peakF_bin<batCall_HiF_bin))
    { //collect data for the powerspectrum
      for (int i = spec_lo; i < spec_hi; i++)
      {    //add new samples
          FFTpowerspectrum[i]+=myFFT.output[i];
          //keep track of the maximum
          if (FFTpowerspectrum[i]>powerspectrum_Max)
            { powerspectrum_Max=FFTpowerspectrum[i];
              powerSpectrum_Maxbin=i;
            }
      }
      //keep track of the no of samples with bat-activity
      powerspectrumCounter++;
    }
 
    if (display_mode==spectrumgraph)
         { 
         if (detector_mode==detector_Auto_TE)
            {  if(FFT_count%5==0) //update the spectrum every 5th sample
                     { spectrum();
                     }
             }
          else
          { if(FFT_count%2==0) //update the spectrum every 2nd sample
                spectrum();
          }
        }   

    // update spectrumdisplay after every 50th FFT sample with bat-activity
    if (display_mode==waterfallgraph)
      if ((powerspectrumCounter>50)  )
       { powerspectrumCounter=0;
          int binLo=spec_lo; int binHi=0;
          float bin2frequency=(SR_real / FFT_points)*0.001;
         //clear powerspectrumbox
         tft.fillRect(0,TOP_OFFSET-POWERGRAPH-SPECTRUMSCALE,ILI9341_TFTWIDTH,POWERGRAPH, COLOR_BLACK);
         // keep a minimum maximumvalue to the powerspectrum
         
         //find the nearest frequencies below 10% of the maximum to show on screen as values
         if (powerSpectrum_Maxbin!=0)
           {
              boolean searchedge=true;
              //lowest value
              int i=powerSpectrum_Maxbin;
              while(searchedge) {
                   if ((FFTpowerspectrum[i]/(powerspectrum_Max+1))>0.1)
                     { i--;
                       if (i==spec_lo)
                        {searchedge=false;
                         binLo=spec_lo; }
                     }
                    else
                     { searchedge=false;
                       binLo=i;
                     }

                  }
              //highest value
              searchedge=true;
              i=powerSpectrum_Maxbin;
              while(searchedge) {
                   if ((FFTpowerspectrum[i]/(powerspectrum_Max+1))>0.1)
                     { i++;
                       if (i==spec_hi)
                        {searchedge=false;
                         binHi=0; }
                     }
                    else
                     { searchedge=false;
                       binHi=i;
                     }
                  }
           }
          //draw spectrumgraph
          for (int i=spec_lo; i<spec_hi; i++)
            { int ypos=FFTpowerspectrum[i]/powerspectrum_Max*POWERGRAPH;
              tft.drawFastVLine((i-spec_lo)*spec_width,TOP_OFFSET-ypos-SPECTRUMSCALE,ypos,COLOR_RED);
              if (i==powerSpectrum_Maxbin)
                { tft.drawFastVLine((i-spec_lo)*spec_width,TOP_OFFSET-ypos-SPECTRUMSCALE,ypos,COLOR_YELLOW);
                }
              //reset powerspectrum for next samples
              FFTpowerspectrum[i]=0;
            }
          
          if (powerSpectrum_Maxbin!=0)
          {
          tft.setCursor(150,TOP_OFFSET-POWERGRAPH);
          tft.setTextColor(ENC_VALUE_COLOR);
          tft.print(int(binLo*bin2frequency) );
          tft.print(" ");
         
          tft.setTextColor(ENC_MENU_COLOR);
          tft.print(int(powerSpectrum_Maxbin*bin2frequency) );
           tft.print(" ");
          tft.setTextColor(ENC_VALUE_COLOR);
          tft.print(int(binHi* bin2frequency) );
          }
          powerspectrum_Max=powerspectrum_Max*0.5; //lower the max after a graphupdate
       }

       

    uint16_t milliGap=50; //param*10;
    
    /*************************  SIGNAL DETECTION ***********************/
    //signal detected in the detection range
    if ((FFT_peakF_bin>batCall_LoF_bin) and (FFT_peakF_bin<batCall_HiF_bin)) 
      {
        // check if it is a new discovered signal in the detection range
        if (not Ultrasound_detected)
          { started_detection=0; //start timing of the call length
            //clicker=0;
           

            if (detector_mode==detector_Auto_heterodyne)
               if (since_heterodyne>1000) //update the most every second
                {osc_frequency=int((FFT_peakF_bin*(SR_real / FFT_points)/500))*500; //round to nearest 500hz
                 set_freq_Oscillator(osc_frequency);
                 since_heterodyne=0;
                 //granular1.stop();
                }

            //restart the TimeExpansion only if the previous call was completely played
            if ((detector_mode==detector_Auto_TE) and (TE_ready) )
             { granular1.stop();
               granular1.beginTimeExpansion(GRANULAR_MEMORY_SIZE);
               granular1.setSpeed(1.0/TE_speed);
               TE_ready=false;
             }
          } /* end ultrasound detected */

        // what if the ultrasound comes in nearly continously ...we than need to stop the play automatically !
        // if after 80ms after detecting we still have ultrasound coming we need to trigger a new detection

        //clicker++;
        Ultrasound_detected=true; 
        
     }
   else // in the last sample there is no peak in the detection range
        {
          if (Ultrasound_detected) //previous sample was still a call
           { callLength=started_detection; // got a pause so store the time since the start of the call
             end_detection=0; //start timing the length of the replay
             }

          Ultrasound_detected=false; // set the end of the call
        }


    // restart TimeExpansion recording a bit after the call has finished completely
    if ((!TE_ready) and (started_detection>(milliGap)))
      { //stop the time expansion
        TE_ready=true;
        Ultrasound_detected=false; // allow a new signal to be processed
        granular1.stopTimeExpansion();
      }

   if (display_mode==waterfallgraph)
   {
     if (end_detection<50) //keep scrolling the screen until 50ms after the last bat-call
      { //if (TE_ready) //not playing TE
        { tft.writeRect( 0,count, ILI9341_TFTWIDTH,1, (uint16_t*) &FFT_pixels); //show a line with spectrumdata
          tft.setScroll(count);
         count++;
        }
      }
     if (count >= ILI9341_TFTHEIGHT-BOTTOM_OFFSET) count = Y_OFFSET;
   }
  }
#endif
}

// ******************************************************* MODES *****************************
void defaultMenuPosition()
{EncLeft_menu_idx=MENU_VOL;
 EncLeft_function=enc_value;
 EncRight_menu_idx=MENU_MIC;
 EncRight_function=enc_value;
}

int heterodynemixer=0;
int granularmixer=1;

void setMixer(int mode)
 {if (mode==heterodynemixer)
  { outputMixer.gain(1,0);  //stop granular output
    outputMixer.gain(0,1);  //start heterodyne output
  }
 if (mode==granularmixer)
  { outputMixer.gain(1,1);  //start granular output
    outputMixer.gain(0,0);  //shutdown heterodyne output
  }
}
//  ********************************************* MAIN MODE CHANGE ROUTINE *************************
void changeDetector_mode()
{
  #ifdef DEBUG_SERIAL
    Serial.print("Changedetector ");

    Serial.println(detector_mode);
  #endif    
  if (detector_mode==detector_heterodyne)
         { granular1.stop(); //stop other detecting routines
           osc_frequency=last_osc_frequency;
           set_freq_Oscillator (osc_frequency);
           setMixer(heterodynemixer);
           //switch menu to volume/frequency
           EncLeft_menu_idx=MENU_VOL;
           EncLeft_function=enc_value;
           EncRight_menu_idx=MENU_FRQ;
           EncRight_function=enc_value;
         }
  if (detector_mode==detector_divider)
         { granular1.beginDivider(GRANULAR_MEMORY_SIZE);
           setMixer(granularmixer);
           defaultMenuPosition();
         }

  if (detector_mode==detector_Auto_TE)
         {  granular1.beginTimeExpansion(GRANULAR_MEMORY_SIZE);
           setMixer(granularmixer);
           granular1.setSpeed(1.0/TE_speed); //default TE is 1/0.06 ~ 1/16 :TODO, switch from 1/x floats to divider value x
           defaultMenuPosition();
       }
  if (detector_mode==detector_Auto_heterodyne)
         { granular1.stop();
           setMixer(heterodynemixer);
           defaultMenuPosition();
         }

  if (detector_mode==detector_passive)
         { granular1.stop(); //stop all other detecting routines
           outputMixer.gain(2,1);  //direct line to output
           outputMixer.gain(1,0);  //shutdown granular output
           outputMixer.gain(0,0);  //shutdown heterodyne output
           defaultMenuPosition();
         }
       else // always shut down the direct output line except for passive
        {  outputMixer.gain(2,0);  //shut down direct line to output
        }

}


// ************************************  SD *****************************


#ifdef USESD // Used for playback
  #include <SD.h>
  File root;

//    #define MAX_FILES    50
//    #define MAX_FILE_LENGTH  13   // 8 chars plus 4 for.RAW plus NULL
//    char filelist[ MAX_FILES ][ MAX_FILE_LENGTH ];
//    int fileselect=0;
//    int referencefile=0;

// ************** //
  void countRAWfiles(void)
  {
      filecounter=0;
      root = SD.open("/");
      filemax=0;   
      while (true) {
          File entry =  root.openNextFile();
          if ((! entry) and (filecounter < MAX_FILES )) {
            break;
          }
          if (entry.isDirectory()) {
            // do nothing, only look for raw files in the root
          }
          else   {
          String fname=entry.name();
          if (fname.indexOf(".raw"))
            {strcpy(filelist[filecounter],entry.name() );
             //construct filenumber from filename to avoid overwriting
             int p=fname.indexOf("_");
             int num=fname.substring(1,p).toInt(); 
             if (num>filemax)
                 {filemax=num;}
                        
             //tft.println(entry.name());
             #ifdef DEBUG_SERIAL
              Serial.println(filelist[filecounter]);
            #endif
            filecounter++;
            }
          }
          entry.close();
        }
        
        
  }
// ************** //
  int initSD(void)
  {
    return SD.begin(BUILTIN_SDCARD); 
  }
  
#else
  int initSD(void) {}
  void countRAWfiles(void) {}
#endif


#ifdef USERECORD

#ifdef USESD1  //uSDFS used for recording (edit uSDFS/utility/sd_config.h)
  #include "uSDFS.h"       // uSDFS lib  - do not uSDFS master !!
  
  FRESULT rc;        /* Result code */
  FATFS fatfs;      /* File system object */
  FIL fil;        /* File object */

  
  //uint32_t count=0;
  uint32_t ifn=0;
  uint32_t isFileOpen=0;

//  uint32_t t0=0;
//  uint32_t t1=0;
  // ************** //


  void initPlay(void)
  { 
    //switch to the player using device 0 
    const char *Device = "1:/";  // 0: SPI 1;SDIO 2:MSC
    f_mount (nullptr, Device, 1);      /* Unmount a logical drive */

    SD.begin(BUILTIN_SDCARD); //make ready for playing
  }

  void initREC(void)
  { const char *Device = "1:/";  // 0: SPI 1;SDIO 2:MSC
    rc = f_mount (&fatfs, Device, 1);      /* Mount a logical drive */
    
      
    rc = f_chdrive(Device);
    
  }
// ************** //
  void startREC(void)
  {
    
    // close file
    if(isFileOpen)
    { //close file
      rc = f_close(&fil);
     
      isFileOpen=0;
    }

    if (filecounter<MAX_FILES) // limit files to MAX_FILES
    if(!isFileOpen)
    {
      filemax++;
      //automated filename BA_S.raw where A=file_number and S shows samplerate. Has to fit 8 chars
      // so max is B999_192.raw
      sprintf(filename, "B%u_%s.raw", filemax, SR[rec_SR].txt);
        #ifdef DEBUG_SERIAL
        Serial.println('start recording');
        Serial.println(filename);
        #endif
      rc = f_stat( filename, 0);
      #ifdef DEBUG_SERIAL
          //Serial.printf("stat %s\n",STAT_ERROR_STRING[rc]);
      #endif
      rc = f_open (&fil, filename, FA_WRITE | FA_CREATE_ALWAYS);
      #ifdef DEBUG_SERIAL
         //Serial.printf(" opened %s\n\r",FR_ERROR_STRING[rc]);
      #endif
      // check if file has errors
      if(rc == FR_INT_ERR)
      { // only option then is to close file
          rc = f_close(&fil);
          if(rc == FR_INVALID_OBJECT)
          {
            #ifdef DEBUG_SERIAL
            Serial.println("unlinking file");
            #endif
            rc = f_unlink(filename);
            
          }
          
      }
      // retry open file
      rc = f_open(&fil, filename, FA_WRITE | FA_CREATE_ALWAYS);

      
      isFileOpen=1;
    }
  }
// ************** //
  void writeREC(AudioRecordQueue *recorder)
  {  
    const uint32_t N_BUFFER = 2;
    const uint32_t N_LOOPS = BUFF*N_BUFFER; // !!! NLOOPS and BUFFSIZE ARE DEPENDENT !!! NLOOPS = BUFFSIZE/N_BUFFER
    // buffer size total = 256 * n_buffer * n_loops
    // queue: write n_buffer blocks * 256 bytes to buffer at a time; free queue buffer;
    // repeat n_loops times ( * n_buffer * 256 = total amount to write at one time)
    // then write to SD card

    if (recorder->available() >= int(N_BUFFER) )
    {// one buffer = 256 (8bit)-bytes = block of 128 16-bit samples
      //read N_BUFFER sample-blocks into memory
      for (uint i = 0; i < N_BUFFER; i++) 
      {
        //copy a new bufferblock from the audiorecorder into memory
        memcpy(sample_buffer + i*256 + nj * 256 * N_BUFFER, recorder->readBuffer(), 256);
        //free the last buffer that was read
        recorder->freeBuffer();
      }

      nj++;

      if (nj >  (N_LOOPS-1))
      {
        nj = 0;
        //old code used to copy into a 2nd buffer, not needed since the writing to SD of the buffer seems faster than the filling
        //this allows larger buffers to be used
        //memcpy(sample_buffer2,sample_buffer,BUFFSIZE);
        //push to SDcard
        rc =  f_write (&fil, sample_buffer, N_BUFFER * 256 * N_LOOPS, &wr);
      }
    }
  }
// ************** //
  void stopREC(AudioRecordQueue *recorder)
  {
    #ifdef DEBUG_SERIAL
      Serial.print("stopRecording");
    #endif
    recorder->end();
    recorderActive=false;
    while (recorder->available() > 0) 
    {
      rc = f_write (&fil, (byte*)recorder->readBuffer(), 256, &wr);
  //      frec.write((byte*)recorder->readBuffer(), 256);
        recorder->freeBuffer();
    }
    //close file
    rc = f_close(&fil);
    
    //
    isFileOpen=0;
    //  frec.close();
    //  playfile = recfile;
  
    //  LeftButton_Mode = MODE_DISPLAY; 
    //  clearname();
    #ifdef DEBUG_SERIAL
      Serial.println (" Recording stopped!");
    #endif

    countRAWfiles(); //update the counter !
    
  }
#else
  void initREC(void){}
  void startREC(void){}
  void writeREC(AudioRecordQueue *recorder){}
  void stopREC(AudioRecordQueue *recorder){}

#endif

#ifdef USESD2
  //#include "SdFs.h"
  //#include "logger_setup.h"
  //  extern "C" uint32_t usd_getError(void);

#endif



// ****************************************************  RECORDING
void startRecording() {

  startREC();
  set_SR(rec_SR); //switch to recording samplerate
  //clear the screen completely
  tft.setScroll(0);
  tft.fillRect(0,0,ILI9341_TFTWIDTH,ILI9341_TFTHEIGHT,COLOR_BLACK);
  tft.setTextColor(ENC_VALUE_COLOR);
  tft.setFont(Arial_28);
  tft.setCursor(0,100);
  tft.print("RECORDING");
  tft.print(filename);
  tft.setFont(Arial_16);
  
  update_display();

  granular1.stop(); //stop granular
  outputMixer.gain(1,0);  //shutdown granular output
  
  //switch off several circuits
  mixFFT.gain(0,0);
  
  last_detector_mode=detector_mode; // save last used detectormode
  detector_mode=detector_heterodyne; // can only listen to heterodyne when recording 

  outputMixer.gain(0,1);

  nj=0;
  //tft.sleep(true);
  recorder.begin();

}

// ************************************************ continueRecording
void continueRecording() {
  writeREC(&recorder);
}

// ******************************************************** STOP RECORDING
void stopRecording() {
  stopREC(&recorder);
  //tft.sleep(false);
  set_SR(oper_SR); //switch back to operational samplerate
  //switch on FFT
  tft.fillScreen(COLOR_BLACK);
  mixFFT.gain(0,1);

  detector_mode=last_detector_mode;
  changeDetector_mode();
  
}
// ******************************************  END RECORDING *************************


#endif //USERECORD


// **************** ******************************PLAYING ************************************************


void startPlaying(int SR) {
//      String NAME = "Bat_"+String(file_number)+".raw";
//      char fi[15];
//      NAME.toCharArray(fi, sizeof(NAME));
playActive=true;
#ifdef DEBUG_SERIAL
      Serial.println (" Started playing");
#endif


 #ifdef DEBUG_SERIAL
      Serial.println (" Set mixers done");
#endif 
  //direct play is used to test functionalty based on previous recorded data
  //this will play a previous recorded raw file through the system as if it were live data coming from the microphone
// if (EncLeft_menu_idx==MENU_PLD)
//   {
//       outputMixer.gain(2,0);  //shutdown direct audio from player to output
//       outputMixer.gain(0,1);  //default mode will be heterodyne based output
//       if (detector_mode==detector_Auto_TE)
//          { outputMixer.gain(1,1);  //start granular output processing
//            outputMixer.gain(0,0);
//          }
//   }

//allow settling
  delay(100);
  //last_SR=SR; //store current samplerate 
  SR=cyclic_constrain(ply_SR,SR_8K,MAX_PLY_SR);
  set_SR(SR);

#ifdef DEBUG_SERIAL
      Serial.println (" Set sample rate done");
#endif 


#ifdef USESD
  fileselect=cyclic_constrain(fileselect,0,filecounter-1);
  strncpy(filename, filelist[fileselect],  13);

  #ifdef DEBUG_SERIAL
      Serial.println (" fileselect finished");
#endif 
  //default display is waterfall
  //display_mode=waterfallgraph;
  //update_display();
  //check if the file can be played
 #ifdef DEBUG_SERIAL
      Serial.println (" just before playing");
      #endif
 if (player.play(filename))
    { 
      #ifdef DEBUG_SERIAL
      Serial.println (" playing OK");
      #endif

    }
    else
    {
     #ifdef DEBUG_SERIAL
      Serial.println (" Playing error");
     #endif
    }
    
   
     
#endif  

}




// ********************************************************** STOP PLAYING ***************************************
void stopPlaying() {

#ifdef DEBUG_SERIAL
  Serial.print("stopPlaying");
#endif
  player.stop();
  
#ifdef DEBUG_SERIAL
  Serial.println (" Playing stopped");
#endif
  playActive=false;
  
  // //restore last SR setting
  set_SR(oper_SR);
  
  osc_frequency=last_osc_frequency;
  //restore heterodyne frequency
  set_freq_Oscillator (osc_frequency);
 
}

// **************** CONTINUE PLAYING
void continuePlaying() {
  //the end of file was reached
  if (playActive)
   {if (!player.isPlaying()) {
     stopPlaying();
    if (continousPlay) //keep playing until stopped by the user
      { startPlaying(ply_SR); //use the samplerate set by the user
      }
    }
   }
}


//*******************************EEPROM *********************************************************
// original routines copied from https://github.com/DD4WH/Teensy-ConvolutionSDR/blob/master/Teensy_Convolution_SDR.ino
#ifdef USEEEPROM

#include <EEPROM.h>
#include <util/crc16.h>

#define EE_CONFIG_VERSION "0003"  //4 char ID of the EE structure, will change with each change in EEprom structure
#define EE_CONFIG_START 0    
//general variables stored in EEprom

char versionEE[5];

struct config_t {
  unsigned char BatVersion; 
  uint16_t EEsaved_count;
  int detector_mode;
  int display_mode;
  int ply_SR;
  int rec_SR;
  int oper_SR;
  int8_t mic_gain;
  int8_t volume;
  int osc_frequency;
  int8_t TE_speed;
  int8_t TE_low; 
  int preset_idx; //0= default values; 1=user values;
  int Encoder_direction; //1 or -1

  char version_of_settings[4]; //versionstring to track changes in the EEstructure
  uint16_t crc;   // added when saving
} E;


boolean loadFromEEPROM(struct config_t *ls) {  //mdrhere
#if defined(EEPROM_h)
  char this_version[] = EE_CONFIG_VERSION;
  unsigned char thechar = 0;
  uint8_t thecrc = 0;
  config_t ts, *ts_ptr;  //temp struct and ptr to hold the data
  ts_ptr = &ts;

  // To make sure there are settings, and they are YOURS! Load the settings and do the crc check first
  for (unsigned int t = 0; t < (sizeof(config_t) - 1); t++) {
    thechar = EEPROM.read(EE_CONFIG_START + t);
    *((char*)ts_ptr + t) = thechar;
    thecrc = _crc_ibutton_update(thecrc, thechar);
  }
  if (thecrc == 0) { // have valid data
    //printConfig_t(ts_ptr);
    #ifdef DEBUG_SERIAL
      Serial.printf("Found EEPROM version %s", ts_ptr->version_of_settings);  //line continued after version
    #endif
    if (ts.version_of_settings[3] == this_version[3] &&    // If the latest version
        ts.version_of_settings[2] == this_version[2] &&
        ts.version_of_settings[1] == this_version[1] &&
        ts.version_of_settings[0] == this_version[0] ) {
      for (int i = 0; i < (int)sizeof(config_t); i++) { //copy data to location passed in
        *((unsigned char*)ls + i) = *((unsigned char*)ts_ptr + i);
      }
      #ifdef DEBUG_SERIAL
      Serial.println(", loaded");
      #endif

      return true;
    } else { // settings are old version
      #ifdef DEBUG_SERIAL
      Serial.printf(", not loaded, current version is %s\n", this_version);
      #endif
      return false;
    }
  } else {
    #ifdef DEBUG_SERIAL
    Serial.println("Bad CRC, settings not loaded");
    #endif
    return false;
  }
#else
  return false;
#endif
}
//routine to save data to EEprom but only those bytes that need to be saved, to conserve EEprom
boolean saveInEEPROM(struct config_t *pd) {
#if defined(EEPROM_h)
  int byteswritten = 0;
  uint8_t thecrc = 0;
  boolean errors = false;
  unsigned int t;
  for (t = 0; t < (sizeof(config_t) - 2); t++) { // writes to EEPROM
    thecrc = _crc_ibutton_update(thecrc, *((unsigned char*)pd + t) );
    if ( EEPROM.read(EE_CONFIG_START + t) != *((unsigned char*)pd + t) ) { //only if changed
      EEPROM.write(EE_CONFIG_START + t, *((unsigned char*)pd + t));
      // and verifies the data
      if (EEPROM.read(EE_CONFIG_START + t) != *((unsigned char*)pd + t))
      {
        errors = true; //error writing (or reading) exit
        break;
      } else {
        //Serial.print("EEPROM ");Serial.println(t);
        byteswritten += 1; //for debuggin
      }
    }
  }
  EEPROM.write(EE_CONFIG_START + t, thecrc);   //write the crc to the end of the data
  if (EEPROM.read(EE_CONFIG_START + t) != thecrc)  //and check it
    errors = true;
  if (errors == true) {
    #ifdef DEBUG_SERIAL
    Serial.println(" error writing to EEPROM");
    #endif
  } else {
    #ifdef DEBUG_SERIAL
    Serial.printf("%d bytes saved to EEPROM version %s \n", byteswritten, EE_CONFIG_VERSION);  //note: only changed written
    #endif
  }
  return errors;
#else
  return false;
#endif
}

boolean EEPROM_LOAD() { 
  config_t E;
  if (loadFromEEPROM(&E) == true) {
    
    #ifdef DEBUG_SERIAL
      Serial.print("BatV ");
      Serial.println(E.BatVersion);
      Serial.print("Preset ");
      Serial.println(E.preset_idx);
      Serial.print("detector ");
      Serial.println(E.detector_mode);
      Serial.print("display ");
      Serial.println(E.display_mode);
      Serial.print("sampleR ");
      Serial.println(E.oper_SR);
      Serial.print("mic_gain ");
      Serial.println(E.mic_gain);
      Serial.print("volume ");
      Serial.println(E.volume);
      Serial.print("TE_SPEED ");
      Serial.println(E.TE_speed);
      Serial.print("TE_low ");
      Serial.println(E.TE_low);
      
    #endif
    EEsaved_count=E.EEsaved_count;

    for (int i = 0; i < 4; i++)
        versionEE[i]= E.version_of_settings[i];
    
    if (E.preset_idx==1) //user has set preset_idx to default to usersettings
      {
        ply_SR=E.ply_SR; //replay speed
        rec_SR=E.rec_SR;
        oper_SR=E.oper_SR; //sample rate 
        preset_idx=E.preset_idx; // default or user settings at startup
        if (preset_idx!=0) 
          {preset_idx=1;} //enforce possible wrong saving
        detector_mode=E.detector_mode; // detector mode
        startup_detector=detector_mode;
        display_mode=E.display_mode; //display mode
        startup_display=display_mode;

        if (display_mode==no_graph)
             {  tft.setScroll(0);
                tft.setRotation( 0 );
              }
        if (display_mode==waterfallgraph)
              {  tft.setRotation( 0 );
              }
        if (display_mode==spectrumgraph)
             {  tft.setScroll(0);
                tft.setRotation( 0 );
              }
        tft.fillScreen(COLOR_BLACK); //blank the screen

        tft.print(display_mode);    
        TE_speed=E.TE_speed; //replay speed for TE
        TE_low=E.TE_low; //low frequency for TE
        osc_frequency=E.osc_frequency; // set centre frequency
        volume=E.volume;
        set_volume(volume);
        mic_gain=E.mic_gain;
        set_mic_gain(mic_gain);
        ENCODER_TURN=E.Encoder_direction;
        enc_dn=ENCODER_TURN*-1;
        enc_up=ENCODER_TURN;

      }

    return true;
    }
    else
    {
      return false; // crc was wrong so probably a changed setup of the EE structure
    }
    
}   

void EEPROM_SAVE() {
  config_t E;
  E.EEsaved_count=EEsaved_count+1; //increase the save counter
  E.BatVersion = versionno;
  E.ply_SR=ply_SR;
  E.rec_SR=rec_SR;
  E.oper_SR=oper_SR;
  E.preset_idx=preset_idx;
  E.detector_mode=startup_detector;
  E.display_mode=startup_display;
  E.TE_low=TE_low;
  E.TE_speed=TE_speed;
  E.osc_frequency=osc_frequency;
  E.volume=volume;
  E.mic_gain=mic_gain;
  E.Encoder_direction=ENCODER_TURN;

  E.crc = 0; //will be overwritten
  //printConfig_t(&E);  //for debugging
  char theversion[] = EE_CONFIG_VERSION;
  for (int i = 0; i < 4; i++)
    E.version_of_settings[i] = theversion[i];
  saveInEEPROM(&E);
} // end void eeProm SAVE

#endif //ifdef useEEPROM




//*****************************************************update encoders
void updateEncoder(uint8_t Encoderside )
 {
  /************************setup vars*************************/
   int encodermode=-1; // menu=0 value =1;
   int change=0;
   int menu_idx=0;
   int choices=0;

    //get encoderdata depending on which encoder sent data
   if (Encoderside==enc_leftside)
    { encodermode=EncLeft_function;
      change=EncLeftchange;
      menu_idx=EncLeft_menu_idx;
      choices=Leftchoices; //available menu options
    }

   if (Encoderside==enc_rightside)
    { encodermode=EncRight_function;
      change=EncRightchange;
      menu_idx=EncRight_menu_idx;
      choices=Rightchoices; //available menu options
    }

  /************************react to changes from the encoder*************************/
  //encoder is in menumode
  if (encodermode==enc_menu)
    { menu_idx=menu_idx+change;

       //remove functionality when SD is not active, so no SDCARD mounted or SDCARD is unreadable
      if (!SD_ACTIVE)
        { if ((menu_idx==MENU_PLAY) or (menu_idx==MENU_REC) )
           { // move menu to next available option
           if (change==1) 
             {menu_idx=MENU_PLAY+1; }
           if (change==-1)
             {menu_idx=MENU_REC-1;}
             
           }
        }

      //allow revolving choices based on the encoder (left or right !!)
      menu_idx=cyclic_constrain(menu_idx,0,choices-1);
      
      if (Encoderside==enc_leftside)
          { EncLeft_menu_idx=menu_idx; //limit the menu
               }

     //limit the changes of the rightside encoder for specific functions
      if (Encoderside==enc_rightside)
          { EncRight_menu_idx=menu_idx; //limit the menu
               }
    }

  //encoder is in valuemode and has changed position so change an active setting
  if ((encodermode==enc_value) and (change!=0))
    { //changes have to be based on the choosen menu position
      /******************************VOLUME  ***************/
      if (menu_idx==MENU_VOL)
        { volume+=change;
          volume=constrain(volume,0,90);
          float V=volume*0.01;
          AudioNoInterrupts();
          sgtl5000.volume(V);
          AudioInterrupts();
        }

      /******************************MAIN SR   ***************/
      if ((menu_idx==MENU_SR) and (LeftButton_Mode!=MODE_PLAY))  //selects a possible SR but only if we are not in the playing mode
        { oper_SR+=change;
          oper_SR=constrain(oper_SR,SR_MIN,SR_MAX); //not cyclic
          set_SR(oper_SR);
          
        }

     /******************************MIC_GAIN  ***************/
      if (menu_idx==MENU_MIC)
        {
         mic_gain+=change;
         mic_gain=constrain(mic_gain,0,63);
         set_mic_gain(mic_gain);
         
        }
      /******************************FREQUENCY  ***************/
      if (menu_idx==MENU_FRQ)
         { int delta=500;
           uint32_t currentmillis=millis();
           //when turning the encoder faster make the changes larger
           if ((currentmillis-lastmillis)<500)
              { delta=1000;}
           if ((currentmillis-lastmillis)<250)
              { delta=2000;}
           if ((currentmillis-lastmillis)<100)
              { delta=5000;}

          osc_frequency=osc_frequency+delta*change;
          // limit the frequency to 500hz steps
          osc_frequency=constrain(osc_frequency,7000,int(SR_real/2000)*1000-1000);
          last_osc_frequency=osc_frequency; //always backup the F setting
          set_freq_Oscillator (osc_frequency);
          lastmillis=millis();
         }
      
      /******************************SETTINGS MENU  ***************/
    
        // for the settings menu the left encoder selects the option and the right encoder changes the values
        if ((menu_idx==MENU_SETTINGS) and (Encoderside==enc_leftside))
          {
            setmenu_pos+=change;
            setmenu_pos=cyclic_constrain(setmenu_pos,SETMENU_MIN,SETMENU_MAX);
          }
        //the rightside encoder is used to change the current value of a given menu-entry
        if ((menu_idx==MENU_SETTINGS) and (Encoderside==enc_rightside))
        { 
          //encoder direction
          if (setmenu_pos==SETMENU_ENC)
            { ENCODER_TURN=ENCODER_TURN*-1; //reverse
              enc_dn=ENCODER_TURN*-1;
              enc_up=ENCODER_TURN;
             }
          //time expansion lowest frequency
          if (setmenu_pos==SETMENU_TE_LOW)
            { TE_low+=change;
              TE_low=cyclic_constrain(TE_low,15,35);
             }
          //time expansion replay speed   
          if (setmenu_pos==SETMENU_TE_SPD)
            {  TE_speed+=change;
               TE_speed=cyclic_constrain(TE_speed,5,30);
             }
          //operational sample rate   
          if (setmenu_pos==SETMENU_SR)
            { oper_SR+=change;
              oper_SR=constrain(oper_SR,SR_MIN,SR_MAX);
              set_SR(oper_SR);
              
             }   
          //default display mode   
          if (setmenu_pos==SETMENU_DISPLAY)
            { startup_display+=change;
              startup_display=cyclic_constrain(startup_display,0,3); //allowed settings
             }   
          //default detector mode
          if (setmenu_pos==SETMENU_DETECTOR)
            { startup_detector+=change;
              startup_detector=cyclic_constrain(startup_detector,0,4); //allowed settings
             }      
          
          //startup settings 
          if (setmenu_pos==SETMENU_STARTUPMODE) //0= default, 1=user settings from EEprom
            { preset_idx+=change;
              preset_idx=cyclic_constrain(preset_idx,0,1); //allowed settings
             }      
          //default sampleRate for replay
          if (setmenu_pos==SETMENU_SR_PLY)
            { ply_SR+=change;
              ply_SR=cyclic_constrain(ply_SR,SR_8K,MAX_PLY_SR);
            }
          //default sampleRate for recording  
          if (setmenu_pos==SETMENU_SR_REC)
            { rec_SR+=change;
              rec_SR=cyclic_constrain(rec_SR,SR_44K,SR_352K);
            }
          //set time
          if (setmenu_pos==SETMENU_TIME)
            {
              int delta=change; //seconds
               uint32_t currentmillis=millis(); //collect timestamp of the change
               //change the current time based on the rotational speed
               if ((currentmillis-lastmillis)<500) 
                  { delta=change*5;} //5 seconds
               if ((currentmillis-lastmillis)<250)
                  { delta=change*10;} //10 seconds
               if ((currentmillis-lastmillis)<100)
                  { delta=change*60;} //1 minutes 

              time_t time_tst = now()+delta;
              tmElements_t tmtm;
              breakTime(time_tst, tmtm);
              Teensy3Clock.set(makeTime(tmtm));
              setTime(makeTime(tmtm));

              lastmillis=currentmillis; //record last time a setting was changed
            }
             //set date
            if (setmenu_pos==SETMENU_DATE)
            {
              long delta=change; //days
               uint32_t currentmillis=millis(); //collect timestamp of the change
               //change the current time based on the rotational speed
               if ((currentmillis-lastmillis)<500) 
                  { delta=change*2;} //7 days
               if ((currentmillis-lastmillis)<250)
                  { delta=change*3;} //14 days
               if ((currentmillis-lastmillis)<100)
                  { delta=change*4;} //28days 

              time_t time_tst = now()+delta*60*60*24;
              tmElements_t tmtm;
              breakTime(time_tst, tmtm);
              Teensy3Clock.set(makeTime(tmtm));
              setTime(makeTime(tmtm));

              lastmillis=currentmillis; //record last time a setting was changed
            }
            if (setmenu_pos==SETMENU_PARAM)
              {  param+=change;
                 param=constrain(param,0,25);
                #ifdef USE_PWMTFT
                 analogWriteResolution(8);
                 analogWrite(3,param*10);
                #endif
              }
           
        }  
  

      /******************************SELECT A FILE  ***************/
      if ((EncLeft_menu_idx==MENU_PLAY) and (EncLeft_function==enc_value))//menu selected file to be played
         {  
           #ifdef USESD
           fileselect+=EncLeftchange;
           fileselect=cyclic_constrain(fileselect,0,filecounter-1);
           
           #endif
         }

      /******************************CHANGE PLAY SR   ***************/
      if ((EncLeft_menu_idx==MENU_PLAY) and (EncRight_menu_idx==MENU_SR) and (EncRight_function==enc_value))//menu play selected on the left and right
          {if ((LeftButton_Mode==MODE_PLAY))
              {  ply_SR+=EncRightchange;
                 ply_SR=cyclic_constrain(ply_SR,SR_8K,MAX_PLY_SR);
                 #ifdef DEBUG_SERIAL
                  Serial.println("REnc SR change");
                 #endif
                 set_SR(ply_SR);
                 
                
              }
        }

      

    }
 }
// **************************  ENCODERS
void update_Encoders()
{
//only react to changes large enough (depending on the steps of the encoder for one rotation)
 long EncRightnewPos = EncRight.read()/4;
 if (EncRightnewPos>EncRightPos)
   { EncRightchange=enc_up; } // up
   else
   if (EncRightnewPos<EncRightPos)
    { EncRightchange=enc_dn; } // down
   else
    { EncRightchange=enc_nc; } //no change =0

 if (EncRightchange!=0)
    {updateEncoder(enc_rightside);
     }

 EncRightPos=EncRightnewPos;

 long EncLeftnewPos = EncLeft.read()/4;
 if (EncLeftnewPos>EncLeftPos)
   { EncLeftchange=enc_up; }
   else
   if (EncLeftnewPos<EncLeftPos)
   { EncLeftchange=enc_dn; }
   else
   { EncLeftchange=enc_nc; }

 if (EncLeftchange!=0)
    {updateEncoder(enc_leftside);
    }

 EncLeftPos=EncLeftnewPos;

 //update display only if a change has happened to at least one encoder
 if ((EncRightchange!=0) or (EncLeftchange!=0))
      update_display();

}
// ******************************************************************************  BUTTONS
void update_Buttons()
{// Respond to button presses
 // try to make the interrupts as short as possible when recording

 if (recorderActive) // RECORDING MODE so do minimal checks !!
   {  micropushButton_L.update(); //ONLYU check the left encoderbutton
      if ((micropushButton_L.risingEdge())  )
       { 
          #ifdef DEBUG_SERIAL
              Serial.println("micropushL 1");
          #endif
         #ifdef USERECORD 
         stopRecording();
         #endif
         delay(100); //wait a bit
        
         recorderActive=false;
          update_display();
       }
   }
 else // ************** NORMAL BUTTON PROCESSING
  {
  
  encoderButton_L.update();
  encoderButton_R.update();
  micropushButton_L.update();
  micropushButton_R.update();

  //rightbutton is completely dedicated to detectormode
   if (micropushButton_R.risingEdge() ) {
         #ifdef DEBUG_SERIAL
              Serial.println("micropushR ");
          #endif
          // micropush generates a risingEdge direct after startup !!
         {
           if (detector_mode==detector_heterodyne)
             {
               last_osc_frequency=osc_frequency; //store osc_frequency when leaving HT mode
             }
           
           detector_mode++;
          if (detector_mode>detector_passive)
            {detector_mode=0;}

          changeDetector_mode();
          update_display();
         }
         
    }
   //leftbutton function is based on leftbutton_mode)
    if (micropushButton_L.risingEdge()) {
          #ifdef DEBUG_SERIAL
              Serial.println("micropushL ");
          #endif

        if (display_mode==settings_page) //get out of the settings directly
         { display_mode=last_display_mode-1;
           LeftButton_Mode=MODE_DISPLAY;
           EncLeft_function=enc_menu;
           EncRight_function=enc_menu; 
           EncRight_menu_idx=MENU_MIC;
         }

        if (LeftButton_Mode==MODE_DISPLAY) 
          {
           display_mode+=1;
           display_mode=cyclic_constrain(display_mode,no_graph,waterfallgraph);
           if (display_mode==no_graph)
             {  tft.setScroll(0);
                tft.setRotation( 0 );
              }
           if (display_mode==waterfallgraph)
              {  tft.setRotation( 0 );
              }
           if (display_mode==spectrumgraph)
             {  tft.setScroll(0);
                tft.setRotation( 0 );
              }
             tft.fillScreen(COLOR_BLACK); //blank the screen
          }
            
        if (LeftButton_Mode==MODE_PLAY)
          {
            if (playActive==false) //button pressed whilst not playing so start
              { playActive=true;
                startPlaying(ply_SR);
              }
              else
              { stopPlaying();
                playActive=false;
              }
          }    

        if (LeftButton_Mode==MODE_REC) 
        { 
          #ifdef DEBUG_SERIAL
              Serial.println("micropushL 2");
          #endif
        
          if (recorderActive==false)  // when recorder is active interaction gets picked up earlier !!
            {   recorderActive=true;
                update_display();
                #ifdef USERECORD
                startRecording();
                #endif
            }

        }
      //no function yet
      update_display();
    }

    /************  LEFT ENCODER BUTTON CONFIRMATION *******************/
    if (encoderButton_L.risingEdge())
    {
      EncLeft_function=!EncLeft_function; 
      
      if ((EncLeft_menu_idx==MENU_SETTINGS)  )   //settings_page 
          { 
            if (display_mode==settings_page) //leaving settings mode
              {
               #ifdef DEBUG_SERIAL
                Serial.println("Leave settings menu");
               #endif
               display_mode=last_display_mode; //restore previous display_mode
               detector_mode=last_detector_mode;
               tft.fillScreen(COLOR_BLACK); 
               EncRight_function=enc_menu;
               EncRight_menu_idx=MENU_MIC;
               }
              else
              {
                #ifdef DEBUG_SERIAL
                Serial.println("Enter settings menu");
               #endif

              last_display_mode=display_mode;  
              last_detector_mode=detector_mode;
              display_mode=settings_page; //show the other user-defined settings
              tft.fillScreen(COLOR_BLACK);
              tft.setScroll(0);
              tft.fillRect(0,TOP_OFFSET-50,240,ILI9341_TFTHEIGHT-TOP_OFFSET+50-BOTTOM_OFFSET,COLOR_BLACK); 
              EncLeft_function=enc_value; // option selection
              setmenu_pos=0;
              EncRight_menu_idx=MENU_SETTINGS;
              EncRight_function=enc_value; //value selection
              
              }
            
         
         }
      
     
     if (SD_ACTIVE)
     {
        //user has confirmed the choice for the leftbuttonmenu
        if ((EncLeft_menu_idx==MENU_REC) and (EncLeft_function==enc_value) )  
          {LeftButton_Mode=MODE_REC; //select the choosen function for the leftbutton
           initREC();
           } 

       //play menu is active, user is selecting files
        if ((EncLeft_menu_idx==MENU_PLAY) and (EncLeft_function==enc_value)) //choose to select values
         { //keep track of the SR
           //last_SR=oper_SR; //store the last set SR
           //SR=ply_SR;
           
           LeftButton_Mode=MODE_PLAY; // directly set LEFTBUTTON to play/stop mode
           last_osc_frequency=osc_frequency; //keep track of heterodyne setting
           initPlay(); //switch SD to playing
           set_SR(ply_SR); //switch to the play SR
           //shut down input
           inputMixer.gain(0,0); //switch off the mic-line as input
           inputMixer.gain(1,1); //switch on the playerline as input
                      
           outputMixer.gain(2,1);  //player to output
           outputMixer.gain(1,0);  //shutdown granular output
           outputMixer.gain(0,0);  //shutdown heterodyne output
           
           countRAWfiles(); // update the filelist
           //set the right encoder to samplerate
           EncRight_menu_idx=MENU_SR;
           EncRight_function=enc_value; 

           #ifdef DEBUG_SERIAL
              Serial.println("direct set MODEPLAY");
           #endif   
           

         }
         
         if (EncLeft_function==enc_menu) //user has pressed to go back to the menu, restart the detector
         {
           #ifdef DEBUG_SERIAL
              Serial.println("SD ACTIVE switch back to menu");
           #endif  
            //restore operational SR setting and last_osc for HT
            osc_frequency=last_osc_frequency;
            set_SR(oper_SR);
                                 
            outputMixer.gain(2,0); //stop the direct line output
            outputMixer.gain(1,1); // open granular output
            outputMixer.gain(0,1); // open heterodyne output

            inputMixer.gain(0,1); //switch on the mic-line
            inputMixer.gain(1,0); //switch off the playerline
 
         }


        //automatically change LEFTbutton back to displaymode if it was on play previously
        if ((EncLeft_function==enc_menu) and (LeftButton_Mode==MODE_PLAY))
          {LeftButton_Mode=MODE_DISPLAY;
           
           set_SR(oper_SR);
           }
                
     } //END SD_ACTIVE
     update_display();
    }

    /************  RIGHT ENCODER BUTTON *******************/

    if (encoderButton_R.risingEdge())
    {
      if (display_mode==settings_page)
         {
           EEPROM_SAVE();
           tft.setTextColor(COLOR_WHITE);
           tft.setCursor(150,ILI9341_TFTHEIGHT-BOTTOM_OFFSET-20);
           tft.print("Saved");
      
         }
      else {
       EncRight_function=!EncRight_function; //switch between menu/value control
      }

      update_display();
    }

  } // ************** END NORMAL BUTTON PROCESSING

}
// **************************  END BUTTONS



//###########################################################################
//###########################################################################
//##########################   MAIN ROUTINE   ###############################
//###########################################################################
//###########################################################################

void setup() {
 #ifdef DEBUG_SERIAL
  Serial.begin(9800);
  delay(5000); //wait 5 seconds 
 #endif

 #ifdef USE_PWMTFT
    analogWrite(3,10); //put screen always at low power
 #endif

  #ifdef USETFT
  tft.begin();
  
  tft.setRotation( 0 );
  tft.setScroll(0);
  tft.setTextColor(COLOR_WHITE);
  tft.fillScreen(COLOR_BLACK);
  tft.setCursor(0,0);
  
  #endif
//setup Encoder Buttonpins with pullups

  pinMode(encoderButton_RIGHT,INPUT_PULLUP);
  pinMode(encoderButton_LEFT,INPUT_PULLUP);

  pinMode(MICROPUSH_RIGHT,INPUT_PULLUP);
  pinMode(MICROPUSH_LEFT,INPUT_PULLUP);

  // startup menu
  EncLeft_menu_idx=MENU_VOL; //volume
  EncRight_menu_idx=MENU_FRQ; //frequency
  EncLeft_function=enc_menu;
  EncRight_function=enc_menu;

  // Audio connections require memory.
  AudioMemory(300);

  setSyncProvider(getTeensy3Time);
  
// Enable the audio shield. select microphone as input. and enable output
  sgtl5000.enable();
  sgtl5000.inputSelect(myInput);
  float V=volume*0.01;
  sgtl5000.volume(V);
  sgtl5000.micGain (mic_gain);
  //sgtl5000.adcHighPassFilterDisable(); // does not help too much!
  sgtl5000.lineInLevel(0);
  sgtl5000.audioProcessorDisable();
  
/* Valid values for dap_avc parameters
// **BUG** with a max gain of 0 turning the AVC off leaves a hung AVC problem where the attack seems to hang in a loop. with it set 1 or 2, this does not occur.
	maxGain; Maximum gain that can be applied 	0 - 0 dB 	1 - 6.0 dB 	2 - 12 dB
	lbiResponse; Integrator Response 	0 - 0 mS 	1 - 25 mS 	2 - 50 mS 	3 - 100 mS
	hardLimit 	0 - Hard limit disabled. AVC Compressor/Expander enabled. 	1 - Hard limit enabled. The signal is limited to the programmed threshold (signal saturates at the threshold)
	threshold : 	floating point in range 0 to -96 dB
	attack :	floating point figure is dB/s rate at which gain is increased
	decay :	floating point figure is dB/s rate at which gain is reduced
*/
// Initialise the AutoVolumeLeveller
  // sgtl5000.autoVolumeControl(1, 1, 0, -6, 40, 20); 
  // sgtl5000.autoVolumeEnable();
  // sgtl5000.audioPostProcessorEnable();

  mixFFT.gain(0,1);
 
/* EEPROM CHECK  */
#ifdef USEEEPROM
 if (EEPROM_LOAD()==false) //load data fromEEprom, if it returns false (probably due to a change in structure) than start by saving default data"
   {
     EEPROM_SAVE();
   }
 #endif   

  analogReference(DEFAULT);
  analogReadResolution(10);
  analogReadAveraging(32);

  int mv1 = 1195*1024/analogRead(71); //3v3 in at Teensy

 #ifdef USE_VIN_ADC
    adc->setAveraging(32, ADC_0); // set number of averages
    adc->setResolution(12,ADC_0); // set bits of resolution
    adc->setReference(ADC_REFERENCE::REF_1V2, ADC_0);
    adc->setConversionSpeed(ADC_CONVERSION_SPEED::LOW_SPEED,ADC_0); // change the conversion speed
    adc->setSamplingSpeed(ADC_SAMPLING_SPEED::LOW_SPEED,ADC_0); // change the sampling speed
    //R1=17.33K R2=2.161 RT=19.49  2.161/19.49=0.111
    VIN= (adc->analogRead(A3))*2.702;///1024; //rough estimate of battery on VIN based on measurements
#endif

// Init TFT display
#ifdef USETFT
  // tft.begin();
  tft.setRotation( 0 );
  tft.setTextColor(COLOR_WHITE);
  tft.fillScreen(COLOR_BLACK);
  
  tft.setCursor(80,50);
  tft.setFont(Arial_20);
  char tstr[9];
  struct tm tx = seconds2time(RTC_TSR);
  snprintf(tstr,9, "%02d:%02d:%02d", tx.tm_hour, tx.tm_min, tx.tm_sec);
  tft.print(tstr);
  tft.setCursor(0,90);
  tft.print("Teensy Batdetector");
  
  tft.setCursor(20,120);
  
  tft.println(versionStr);
  tft.println();
  tft.setFont(Arial_16);
  snprintf(tstr,9, "%9lX",chipNum[2]);  
  tft.print("Teensy SN:");
  tft.println(tstr);
  #ifdef USEEEPROM
  tft.print("EEprom  v:");
  snprintf(tstr,9,"%s",versionEE);
  //tft.println(String(versionEE));
  tft.println(tstr);
  tft.print("  saved #:");
  tft.println(EEsaved_count);
  if (preset_idx==1)
  {tft.println("Settings:USER");
  }
  else
  {tft.println("Settings:DEFAULT");
    }
    
    
  #endif
  
  tft.print("V:");
  tft.print(mv1);
  #ifdef USE_VIN_ADC
   
    tft.print(" VCC ");
    tft.println(VIN);
   
  #endif 
  delay(3000); //wait 3 seconds to clearly show the data
  tft.fillScreen(COLOR_BLACK);
  tft.setCursor(0, 0);
  tft.setScrollarea(TOP_OFFSET,BOTTOM_OFFSET);
  
  update_display();

  if (display_mode==spectrumgraph)
             {  tft.setScroll(0);
                tft.setRotation( 0 );
              }

#endif

//Init SD card use
// uses the SD card slot of the Teensy, NOT that of the audio board !!!!!

  if(!initSD())
  {
      #ifdef DEBUG_SERIAL
          Serial.println("Unable to access the SD card");
          delay(500);
      #endif
    SD_ACTIVE=false;
    tft.fillCircle(10,TOP_OFFSET,5,COLOR_RED);
  }
  else  {
    SD_ACTIVE=true;
    tft.fillCircle(10,TOP_OFFSET,5,COLOR_GREEN);
    countRAWfiles();
  }



// ***************** SETUP AUDIO *******************************
set_SR (oper_SR); //set operational sample rate
set_freq_Oscillator (osc_frequency);
inputMixer.gain(0,1); //microphone active
inputMixer.gain(1,0); //player off

outputMixer.gain(0,1); // heterodyne1 to output
outputMixer.gain(1,0); // granular to output off
outputMixer.gain(2,0); // player to output off

// the Granular effect requires memory to operate
granular1.begin(granularMemory, GRANULAR_MEMORY_SIZE);

// reset the FFT denoising array at startup
for (int16_t i = 0; i < 128; i++) {
  FFTavg[i]=0;
    }

//switch to the preset or default detector_mode
changeDetector_mode();

 // update all encoders/buttons before starting to prevent false changes
  encoderButton_L.update();
  encoderButton_R.update();
  micropushButton_L.update();
  micropushButton_R.update();


    
} // END SETUP

//************************************************************************         LOOP         ******************
//start the processing loop !
void loop()
{
// BATTERY SAVING. put processor asleep until the next interrupt
//asm(" WFI"); 20190818, TESTED does not make a difference in mA usage

// If we're playing or recording, carry on...
  if (LeftButton_Mode == MODE_REC) {
    #ifdef USERECORD
    continueRecording();
    #endif
  }

  if (LeftButton_Mode == MODE_PLAY) 
  {if (playActive)
      {continuePlaying();
       }
  }
  
 update_Buttons();
 
  // during recording screens are not updated to reduce interference !
 if (not recorderActive)
  { update_Encoders();
    #ifdef USETFT
    update_Display();
    #endif
   

  //update the time regularly 
  struct tm tx = seconds2time(RTC_TSR);
   
  if (tx.tm_min!=old_time_min)
  { tft.setFont(Arial_16);
    tft.setCursor(180,20);
    tft.fillRect(150,20,90,20,MENU_BCK_COLOR);
    tft.setTextColor(ENC_MENU_COLOR);
    char tstr[9];
    snprintf(tstr,9, "%02d:%02d", tx.tm_hour, tx.tm_min);
    tft.print(tstr);
    old_time_min=tx.tm_min;
    #ifdef USE_VIN_ADC
     VIN= (adc->analogRead(A3))*2.702; //empirical !!
    #endif  

   }

  }

  
}