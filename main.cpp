#include <vector>
#include <string>
#include "mbed.h"
#include "SDFileSystem.h"
#include "wave_player.h"
#include "TextLCD.h"
#include "PinDetect.h"

//Pin assignments
Serial pc(USBTX, USBRX);

PinDetect vDown(p21);
PinDetect vUp(p22);
PinDetect prev(p23);
PinDetect playPause(p24);
PinDetect next(p25);

DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);
DigitalOut led4(LED4);

TextLCD lcd(p15, p16, p13, p12, p11, p10); // rs, e, d4-d7

SDFileSystem sd(p5, p6, p7, p8, "sd"); //SD card

AnalogOut DACout(p18);

wave_player waver(&DACout);

//Globals
vector<string> filenames;
string pathPrefix = "/sd/myMusic/";
FILE *wave_file;
vector<string>::iterator it;
string songName;
Ticker volLight;
Ticker pauseLight;


void play(void)
{
    if (waver.getPlaying() == false)
    {
        waver.setPlaying(true);
        lcd.cls();
        lcd.printf("%s",songName.c_str());
    }
    else if (waver.getPlaying() == true)
    {
        waver.setPlaying(false);
        lcd.cls();
        lcd.printf("Paused");
    }
}

void nextSong(void)
{
    waver.setStop(true);
}

void previousSong(void)
{
    waver.setStop(true);
    if (it == filenames.begin())
    {
        it = filenames.end()-2;
    }
    else
    {
        it--;
        it--;
    }
}

void volumeUp(void)
{
    waver.volUp();
}

void volumeDown(void)
{
    waver.volDown();
}

void volLEDs()
{
    if (waver.getPlaying())
    {
        if (waver.getVol() == 16)
        {
            led1 = 1;
            led2 = 1;
            led3 = 1;
            led4 = 1;
        }
        else if ((11 <= (waver.getVol())) && ((waver.getVol()) <= 15))
        {
            led1 = 1;
            led2 = 1;
            led3 = 1;
            led4 = 0;
        }
        else if ((6 <= (waver.getVol())) && ((waver.getVol()) <= 10))
        {
            led1 = 1;
            led2 = 1;
            led3 = 0;
            led4 = 0;
        }
        else if ((1 <= (waver.getVol())) && ((waver.getVol()) <= 5))
        {
            led1 = 1;
            led2 = 0;
            led3 = 0;
            led4 = 0;
        }
        else if (waver.getVol() == 0)
        {
            led1 = 0;
            led2 = 0;
            led3 = 0;
            led4 = 0;
        }
    }
    
}

void pauseLEDs()
{
    if (!waver.getPlaying())
    {
        led1 = !led1;
        led2 = !led2;
        led3 = !led3;
        led4 = !led4;
    }
}

void read_file_names(char *dir)
{
    DIR *dp;
    struct dirent *dirp;
    dp = opendir(dir);
    
    while((dirp = readdir(dp)) != NULL)
        {
            filenames.push_back(string(dirp->d_name));
        }   
}


int main()
{
    playPause.mode(PullUp);
    next.mode(PullUp);
    prev.mode(PullUp);
    vUp.mode(PullUp);
    vDown.mode(PullUp);
    
    playPause.attach_asserted(&play);
    next.attach_asserted(&nextSong);
    prev.attach_asserted(&previousSong);
    vUp.attach_asserted(&volumeUp);
    vDown.attach_asserted(&volumeDown);
    
    playPause.setSampleFrequency();
    next.setSampleFrequency();
    prev.setSampleFrequency();
    vUp.setSampleFrequency();
    vDown.setSampleFrequency();
    
    volLight.attach(&volLEDs, 0.01);
    pauseLight.attach(&pauseLEDs, 1.0);
    
    read_file_names("/sd/myMusic");
    it = filenames.begin();
    while (1)
    {
        waver.setStop(false);
        while(waver.getStop() == false)
        {
            string fullName = pathPrefix + *it;
            char * cFullName = new char [fullName.length()+1];
            strcpy(cFullName, fullName.c_str());           
            songName = (*it);
            songName.erase(songName.length()-4);
            lcd.printf("%s",songName.c_str());
            wave_file=fopen(cFullName,"r");
            waver.play(wave_file);
            fclose(wave_file);
            lcd.cls();
            wait(1);
            
            if (it == filenames.end()-1)
            {
                it = filenames.begin();
            }
            else
            {
                it++;
            }
        }
    }
}