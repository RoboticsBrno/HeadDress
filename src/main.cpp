
#include <Arduino.h>                //include basic Arduino library
#include <SmartLeds.h>              //SmartLeds - library for controll intelligent leds (NEOIPIXEL) maked by: https://github.com/yaqwsx
#include <WiFi.h>                   //basic library for control WiFi on ESP32
#include <AsyncTCP.h>               //must have library for Asynchronous Web Server maked by: https://github.com/me-no-dev
#include <ESPAsyncWebServer.h>      //library for Asynchronous Web Server on ESP32 maked by: https://github.com/me-no-dev
#include <webpage.h>                //header file with web page, which will ESP send to user device
#include <DNSServer.h>

AsyncWebServer server(80);          //starts web server on port 80
DNSServer dnsServer;

#define LED_PIN 18                  //data pin for intelligent leds at the top of hat
#define LED_COUNT 14                //number of leds at the top of hat

SmartLed leds (LED_WS2812, LED_COUNT, LED_PIN, 1, SingleBuffer);                  //creates member called leds - top of the hat

const char *ssid = "Celenka";           //change here for WiFi AP SSID
const char *password = "robotika";      //change here for WiFi AP password

volatile int brightness = 255;          //default brightness (0-255, 255 = full)
volatile int STEP = 10;                 //step for brightness regulation
volatile uint8_t mode, hue, randHue;    //mode selection, hue of color, hue for disco

volatile int snakePosition = 5;                     //starting snake position (maybe it can be 0??)
volatile uint8_t beatPosition = 0;                  //beat starting value
volatile int beatCount = 1;                         //beat count up (== 1) or count down (== -1)
volatile bool rearLED, rearLED_last = 0;            //statuses of rear leds

void clearLeds()                            //makes leds at the top clean
{
    for (int i = 0; i != LED_COUNT; ++i)
        leds[i] = Hsv {0, 0, 0};
    leds.show();
}

void controlLeds()                      //main function to drive all leds effects
{
    switch (mode){
    case 0:     //nothing - "Budiž tma"
        for (int i = 0; i != LED_COUNT; ++i)
            leds[i] = Hsv {0, 0, 0};
        leds.show();
        for (int i = 0; i != LED_COUNT; ++i)
            leds[i] = Hsv {0, 0, 0};
        leds.show();
        break;
    case 1:
        break;
    case 2:     //wave - "Chci být duha"
        ++hue;
        for (int i = 0; i != LED_COUNT; ++i)
            leds[i] = Hsv {static_cast< uint8_t >(hue + 10 * i), 255, brightness};
        leds.show();
        delay(10);
        break;
    case 3:     //snake - "Jdu si hrát na policajty"
        leds [snakePosition] = Hsv {(hue + 10), 255, brightness};
        leds.show();
        leds.wait();
        for (int i = 0; i != LED_COUNT; ++i)
            leds [i] = Hsv {0, 0, 0};
        snakePosition++;
        if (snakePosition == LED_COUNT){
            snakePosition = 0;
            hue += 10;
        }
        delay(20);
        break;
    case 4:     //disco - "Uděláme diskošku"
        randHue = random(0, 255);
        for (int i = 0; i != LED_COUNT; ++i)
            leds[i] = Hsv {randHue, 255, brightness};
        leds.show();
        delay(200);
        break;
    case 5:     //beat - "Moje klidná mysl"
        for(int i = 0; i != LED_COUNT; ++i)
            leds [i] = Hsv {(hue + 10), 255, beatPosition};
        leds.show();
        delay(8);
        for (int i = 0; i != LED_COUNT; ++i)
            leds [i] = Hsv {0, 0, 0};
        if (beatPosition == 0){
            beatCount = 1;
            hue += 10;
        }
        if (beatPosition >= brightness)
            beatCount = -1;
        beatPosition += beatCount;
        break;


    default:        //default = nothing lights
        for (int i = 0; i != LED_COUNT; ++i)
            leds[i] = Hsv {0, 0, 0};
        leds.show();
        break;
    }
}

void setup ()
{
    Serial.begin(115200);               //begin Serial line with baudrate 115200
    WiFi.softAP(ssid, password);        //create WiFi AP

    dnsServer.start(53, "*", WiFi.softAPIP());

    Serial.println(WiFi.softAPIP());    //print IP addres of ESP to debug Serial line

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){       //while webserver catches /GET request, procces this function

        int paramsNr = request->params();                   //store number of parameters from user
        Serial.println(paramsNr);                           //print it to debug serial
        for(int i=0; i != paramsNr; ++i){                   //process all parameters
            AsyncWebParameter* p = request->getParam(i);    //store actual parameter to 'p'
            if (p->name() != "cmd")                         //if name of parameter == "cmd" continue
                continue;
            String cmd = p->value();                        //parse parameter value to separate string called "cmd"
            if (cmd == "wave"){
                mode = 2;
            }
            else if (cmd == "snake"){
                mode = 3;
            }
            else if (cmd == "disco"){
                mode = 4;
            }
            else if (cmd == "beat"){
                mode = 5;
            }
            else if (cmd == "off"){
                clearLeds();
                mode = 0;
                clearLeds();
            }
            else if (cmd == "up"){
                brightness += STEP;
                if (brightness > 255)
                    brightness = 255;
            }
            else if (cmd == "down"){
                brightness -= STEP;
                if (brightness < 0)
                    brightness = 0;
            }
        }
        request->send_P(200, "text/html", index_html);      //sends HTML page to user
    });

    server.begin();         //start webserver
}

void loop ()
{
    dnsServer.processNextRequest();
    controlLeds();          //periodiccaly turns function to control all effects....LEDs, piezo
}