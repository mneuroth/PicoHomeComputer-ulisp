#ifndef _ARDUINO_DESKTOP_H
#define _ARDUINO_DESKTOP_H

#include <stdio.h>
#include <string.h>
#include <time.h>

#ifdef _MSC_VER 
#include <conio.h>
//not #if defined(_WIN32) || defined(_WIN64) because we have strncasecmp in mingw
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#else
bool _kbhit() { return true; }
char _getch() { return 0; }
void _putch(char)  {}
#endif

#define HIGH 1
#define LOW  0

#define OUTPUT 1
#define INPUT  0
#define INPUT_PULLUP 2

#define MSBFIRST 1
#define LSBFIRST 0

#define _TIMER_3_IRQ 1
#define _TIMER_3_VECTOR 2

//#define T3CON_ENABLE_BIT 8
//#define T3CON_PRESCALER_BITS 8

#define T3CONCLR 4
//#define T3CON_ENABLE_BIT 8
//#define T3CON_PRESCALER_BITS 9

int TMR3;
int T3CON;
int PR3;
int T3CONSET;

#ifndef __linux__
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

typedef char int8_t;
typedef short int16_t;
typedef int int32_t;
typedef long int64_t;
#else
#include <cstdint>
#endif

typedef unsigned char byte;

typedef bool boolean;

typedef const char * PGM_P;

typedef int eAnalogReference;

class __FlashStringHelper;

class HardwareSerial;

class TwoWire {
public:
    //void send(char) {}
    void receive() {}
    void begin() {}
    void write(char ch) { printf("%c",ch); }
    char read() { return 0; }
    void beginTransmission(int) {}
    int endTransmission(bool = false) { return 0; }
    bool requestFrom(int, int ) { return 0; }
};

class HardwareSerial {
public:
    HardwareSerial(bool enable = true) 
    {
        m_bEnabled = enable;
        memset(m_sBuffer,0,1024);
        m_iPos = 0;
    }
    
    operator bool() { return true; }
    void begin(int) {}
    bool available() 
    { 
        bool ok = _kbhit() != 0;
//        printf("av() %d ", ok);
        return ok;
        /*
        if( isBufferEmpty() )
        {
            if (m_bEnabled)
            {
                printf("Enter: ");
                memset(m_sBuffer, 0, 1024);
                m_iPos = 0;
                char ch = _getch();
                //fscanf(stdin, "%1023[^\n]", &m_sBuffer);
                //scanf("%s", &m_sBuffer);
                //strcpy(m_sBuffer, "(* 99 99)\n");
                //getline(&m_sBuffer,1023,stdin);
                printf("read -> %s len=%zd\n", m_sBuffer, strlen(m_sBuffer));
                m_iPos = 0;
            }
            return false;
        }
        else
        {
            return true;
        }
//        printf("av? "); return true; 
        */
    }
    char read(bool inEscape = false) 
    { 
        if (inEscape) {
            return 0;
        }
        //printf("getc() ");
        char ch = _getch();
        _putch(ch);
        return ch;
        /*
        if( !isBufferEmpty() )
        {
            char ch = m_sBuffer[m_iPos++];
            //printf("[->%c] ",ch);
            return ch;
        }
        return 0; 
        */
    }
    void write(char ch) { printf("%c",ch); }
    void flush() {}
    void end() {}
    
    bool isBufferEmpty() const { /*printf("%zd %d",strlen(m_sBuffer),m_iPos);*/ return strlen(m_sBuffer) == 0 || m_iPos == strlen(m_sBuffer); }
    int m_iPos;
    char m_sBuffer[1024];
    bool m_bEnabled;
};

class SerialClass : public HardwareSerial {
public:
    SerialClass(bool enable)
        : HardwareSerial(enable)
    {}
};

TwoWire Wire;
SerialClass Serial(true);
SerialClass Serial1(false);

void delay(int) {}
unsigned long millis() { return 0; }
unsigned long micros() { return 0; }

int random(int) { return 0; }
void randomSeed(int) {}

void pinMode(int, int) {}
void digitalWrite(int, int) {}
int digitalRead(int) { return 0; }
bool bitRead(int, int) { return false; }
void analogWriteResolution(double) {}
void analogReadResolution(double) {}
double analogRead(int) { return 0; }
void analogWrite(int, double) {}
void analogReference(double) {}

void clearIntFlag(int) {}

int min(int a, int b) { return a<b ? a : b; }

#include <string.h>

#include <math.h>

#include <stdio.h>

#endif
