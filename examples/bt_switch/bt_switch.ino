#include <SoftwareSerial.h>// import the serial library
#include <CmdProcessor.h>

#define BT_TX_PIN 1
#define BT_RX_PIN 2
#define SWITCH_PIN 4

#define ON_CMD  "on"
#define OFF_CMD "off"
#define STATUS_CMD "status"
#define WAIT_CMD "wait"
#define BLINK_CMD "blink"

void statusHandler(const char* args, void* data);
void onHandler(const char* args, void* data); 
void offHandler(const char* args, void* data);
void waitHandler(const char* args, void* data);
void blinkHandler(const char* args, void* data);

SoftwareSerial bt(BT_TX_PIN, BT_RX_PIN);
CmdProcessor<5> cmdProc;

void setup() {
  //digitalWrite(SWITCH_PIN, LOW);
  pinMode(SWITCH_PIN, OUTPUT);
  pinMode(BT_TX_PIN, INPUT);
  pinMode(BT_RX_PIN, OUTPUT);

  bt.begin(9600);
  bt.println(F("Bluetooth Switch v1.0"));
  bt.flush();
  
  cmdProc.registerCmd(ON_CMD, &onHandler);
  cmdProc.registerCmd(OFF_CMD, &offHandler);
  cmdProc.registerCmd(STATUS_CMD, &statusHandler);
  cmdProc.registerCmd(WAIT_CMD, &waitHandler);
  cmdProc.registerCmd(BLINK_CMD, &blinkHandler);
}

void loop() {
  if (bt.available())
  {
    String cmdLine;
    while ( bt.available() > 0)
      cmdLine = cmdLine + (char)bt.read();
    if (cmdLine.endsWith(F("\r\n")))
      cmdLine = cmdLine.substring(0, cmdLine.length() - 2);
    cmdLine.trim();

    //bt.println(cmdLine.c_str());
    //bt.flush();
    if (cmdProc.process(cmdLine.c_str()))
      bt.println(F("OK"));
    else
      bt.println(F("ERR"));
    bt.flush();
  }
  delay(100);
}

void statusHandler(const char* args, void* data)
{
  bt.println((HIGH == digitalRead(SWITCH_PIN)) ? F("ON") : F("OFF"));
  //bt.flush();
}

void onHandler(const char* args, void* data)
{
  digitalWrite(SWITCH_PIN, HIGH);
}

void offHandler(const char* args, void* data)
{
  digitalWrite(SWITCH_PIN, LOW);
}

void waitHandler(const char* args, void* data)
{
  int milisecs = args ? atoi(args) : 1000;
  delay(milisecs);
}

void blinkHandler(const char* args, void* data)
{
   int times = args ? atoi(args) : 1;
    while (times--)
    {
      digitalWrite(SWITCH_PIN, HIGH);
      delay(500);
      digitalWrite(SWITCH_PIN, LOW);
      delay(500);
    }
}

