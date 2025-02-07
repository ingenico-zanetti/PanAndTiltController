#include "AtCommand.hpp"
#include "AtCommandI.hpp"
#include "AtCommandPlus.hpp"
#include "AtCommandAmpersAnd.hpp"
#include "GlobalConfiguration.hpp"
#include "Stepper.hpp"

int ledStatus;
uint32_t oldSeconds;
uint32_t oldMillis;
uint32_t oldQuarter;

AtCommandAnalyzer analyzer;
AtCommandAnalyzer analyzer3;

HardwareSerial Serial3(PB11, PB10); // RX3, TX3

int pinState = LOW;


void Update_IT_callback(void){
  bool updatePan = panStepper.run();
  if(updatePan){
    panStepper.update();
  }
  bool updateTilt = tiltStepper.run();
  if(updateTilt){
    tiltStepper.update();
  }
}

void setup() {
  Serial.begin(115200);
  Serial3.begin(9600);

  pinMode(LED_BUILTIN, OUTPUT);
  ledStatus = HIGH;
  digitalWrite(LED_BUILTIN, ledStatus);
  
  oldMillis = millis();
  oldQuarter = oldMillis / 250;
  oldSeconds = oldMillis / 1000;

  analyzer.addCallback('I', handleATI);
  analyzer.addCallback('+', handlePlus);
  analyzer.addCallback('&', handleAmpersAnd);
  analyzer.setSerial(&Serial);

  analyzer3.addCallback('I', handleATI);
  analyzer3.addCallback('+', handlePlus);
  analyzer3.addCallback('&', handleAmpersAnd);
  analyzer3.setSerial(&Serial3);

  // while(0 == Serial.available());

#if 1
#if defined(TIM1)
  TIM_TypeDef *Instance = TIM1;
#else
  TIM_TypeDef *Instance = TIM2;
#endif


  // Instantiate HardwareTimer object. Thanks to 'new' instanciation, HardwareTimer is not destructed when setup() function is finished.
  HardwareTimer *MyTim = new HardwareTimer(Instance);
  MyTim->setOverflow(100, MICROSEC_FORMAT); // 10 KHz
  MyTim->attachInterrupt(Update_IT_callback); // bind argument to callback: When Update_IT_callback is called MyData will be given as argument
  MyTim->resume();
#endif
}

void loop() {
  uint32_t newMillis = millis();
  if(newMillis != oldMillis){
    oldMillis = newMillis;
  }
  {
    int available = Serial.available();
    if(available > 0){
#define BUFFER_SIZE (64)
      char buffer[BUFFER_SIZE];
      if((unsigned int)available > sizeof(buffer)){
        available = sizeof(buffer);
      }
      int lus = Serial.readBytes(buffer, available);
      if(lus > 0){
        for(int i = 0 ; i < lus ; i++){
          analyzer.addChar(buffer[i]);
        }
      }
    }
  }
  {
    int available = Serial3.available();
    if(available > 0){
#define BUFFER_SIZE (64)
      char buffer[BUFFER_SIZE];
      if((unsigned int)available > sizeof(buffer)){
        available = sizeof(buffer);
      }
      int lus = Serial3.readBytes(buffer, available);
      if(lus > 0){
        for(int i = 0 ; i < lus ; i++){
          analyzer3.addChar(buffer[i]);
        }
      }
    }
  }
  uint32_t newSeconds = newMillis / 1000;
  if(newSeconds != oldSeconds){
    oldSeconds = newSeconds;
      ledStatus = (HIGH == ledStatus) ? LOW : HIGH;
      digitalWrite(LED_BUILTIN, ledStatus);
  }
  uint32_t newQuarter = newMillis / 250;
  if(newQuarter != oldQuarter){
    oldQuarter = newQuarter;
  }
}
