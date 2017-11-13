#include <ESP8266WiFi.h>
#include <CmdProcessor.h>

// WiFi Definitions
const char WiFiSSID[] = "WIFISSID";
const char WiFiPSK[] = "XXX";

const int LED_PIN = LED_BUILTIN;
const int SWITCH_PIN = 5;
const int SWITCH_ON = HIGH;
const int SWITCH_OFF = LOW;

// Supported commands
// http://ipaddress/switch/on
// http://ipaddress/switch/off
// http://ipaddress/switch/blink=3
// http://ipaddress/switch?blink=3&on&wait=5000&off
const char* ON_CMD = "on";
const char* OFF_CMD = "off";
const char* STATUS_CMD = "status";
const char* BLINK_CMD = "blink";
const char* WAIT_CMD = "wait";
const char* UNKNOWN_CMD = "unknown";

WiFiServer server(80);
CmdProcessor<5,'&', '='> cmdProc;

void setup() 
{
  // Init pins
  initHardware();

  // Register supported commands
  cmdProc.registerCmd(ON_CMD, [](const char*, void*){
    digitalWrite(SWITCH_PIN, SWITCH_ON);
  });
  cmdProc.registerCmd(OFF_CMD, [](const char*, void*){
    digitalWrite(SWITCH_PIN, SWITCH_OFF);
  });
  cmdProc.registerCmd(STATUS_CMD, [](const char*, void*){
    // do nothing, as we always answer with status
  });
  cmdProc.registerCmd(BLINK_CMD, [](const char* args, void*){
    int times = args ? atoi(args) : 1;
    while (times--)
    {
      digitalWrite(SWITCH_PIN, SWITCH_ON);
      delay(500);
      digitalWrite(SWITCH_PIN, SWITCH_OFF);
      delay(500);  
    }
  });
  cmdProc.registerCmd(WAIT_CMD, [](const char* args, void*){
    int milisecs = args ? atoi(args) : 1000;
    delay(milisecs);
  });
  
  connectToWiFi();
  server.begin();
}

void loop() 
{
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) 
    return;
 
  Serial.println("Client connected");

  // Read the first line of the request as we are expecting a GET
  String req = client.readStringUntil('\r');
  Serial.println(req);
  client.flush();

  // Process cmd
  if (cmdProc.process(getCmdLine( req ).c_str()))
  {
    // Prepare the Status response. Start with the common header:
    String response = "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: text/html\r\n\r\n";
    response += "<!DOCTYPE HTML>\r\n<html>\r\n";
    response += "Switch is ";
    response += (digitalRead(SWITCH_PIN) == SWITCH_ON)?"on":"off";
    response += "</html>\n";

    // Send the response to the client
    client.print(response);
    delay(1);   
  }
  Serial.println("Client disconnected");
}


void initHardware()
{
  digitalWrite(SWITCH_PIN, SWITCH_OFF);
  Serial.begin(9600);
  while(!Serial) {
    ;
  }
  Serial.print("Initializaing Hardware...");
  pinMode(LED_PIN, OUTPUT);
  pinMode(SWITCH_PIN, OUTPUT);
  Serial.println("Done.");
}

void connectToWiFi()
{
  Serial.print("Connecting to ");
  Serial.print(WiFiSSID);
  
  byte ledStatus = LOW;

  // Set WiFi mode to station (as opposed to AP or AP_STA)
  WiFi.mode(WIFI_STA);

  // WiFI.begin([ssid], [passkey]) initiates a WiFI connection
  // to the stated [ssid], using the [passkey] as a WPA, WPA2,
  // or WEP passphrase.
  WiFi.begin(WiFiSSID, WiFiPSK);

  // Use the WiFi.status() function to check if the ESP8266
  // is connected to a WiFi network.
  while (WiFi.status() != WL_CONNECTED)
  {
    // Blink the LED
    digitalWrite(LED_PIN, ledStatus); // Write LED high/low
    ledStatus = (ledStatus == HIGH) ? LOW : HIGH;
    if (ledStatus == HIGH)
      Serial.print(".");

    // Delays allow the ESP8266 to perform critical tasks
    // defined outside of the sketch. These tasks include
    // setting up, and maintaining, a WiFi connection.
    delay(100);
    // Potentially infinite loops are generally dangerous.
    // Add delays -- allowing the processor to perform other
    // tasks -- wherever possible.
  }
  Serial.println("Done.");
  Serial.print("Switch Server at ");
  Serial.println(WiFi.localIP());
}

String getCmdLine(String req)
{
  const char* SWITCH_REQ = "GET /switch";
  int ctx = req.indexOf(SWITCH_REQ);
  if ( ctx != -1)
  {
    int httpVer = req.indexOf(" HTTP/", ctx);
    if (httpVer != -1)
    {
      String cmdLine = req.substring(ctx + strlen(SWITCH_REQ), httpVer);
      if (cmdLine.startsWith("/") || cmdLine.startsWith("?"))
        return cmdLine.substring(1);
    }
  }
  return String(UNKNOWN_CMD);
}

