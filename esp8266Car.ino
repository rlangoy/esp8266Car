//#include <ESP8266WiFiMulti.h>
#include <ArduinoOTA.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include <WebSocketsServer.h>

#if defined(ESP8266)
#include <ESP8266WiFi.h>          
#else
#include <WiFi.h>          
#endif

#include <IotWebConf.h>

// -- Initial name of the Thing. Used e.g. as SSID of the own Access Point.
const char thingName[] = "RCDIYCar";

// -- Initial password to connect to the WifiMoistAlarm, when it creates an own Access Point.
const char wifiInitialApPassword[] = "123456789";

DNSServer dnsServer;

#define CAR_M1A     02            // Motor 1
#define CAR_M1B     04

#define CAR_M2A     00            // Motor 2
#define CAR_M2B     05

#define LED_U       16            //Onboard LED


ESP8266WebServer server(80);       // create a web server on port 80
IotWebConf iotWebConf(thingName, &dnsServer, &server, wifiInitialApPassword);

WebSocketsServer webSocket(81);    // create a websocket server on port 81

File fsUploadFile;                                    // a File variable to temporarily store the received file


const char *OTAName = "DiyCar";           // A name and a password for the OTA service
const char *OTAPassword = "esp8266";


const char* mdnsName = "DiyCar"; // Domain name for the mDNS responder

uint8_t socketNumber;


/*__________________________________________________________SETUP__________________________________________________________*/

void setup() {
  //Init motors
  pinMode(CAR_M1A, OUTPUT);
  pinMode(CAR_M1B, OUTPUT);
  pinMode(CAR_M2A, OUTPUT);     
  pinMode(CAR_M2B, OUTPUT);    

  //Init debug led
  pinMode(LED_U, OUTPUT);      // Initialize the LED_BUILTIN pin as an output

  

  Serial.begin(115200);        // Start the Serial communication to send messages to the computer
  delay(10);
  Serial.println("\r\n");


// -- Initializing the configuration.
  iotWebConf.init();
  
  startOTA();                  // Start the OTA service
  
  startSPIFFS();               // Start the SPIFFS and list all contents

  startWebSocket();            // Start a WebSocket server
  
  startMDNS();                 // Start the mDNS responder

  startServer();               // Start a HTTP server with a file read handler and an upload handler
  
}

/*__________________________________________________________LOOP__________________________________________________________*/


unsigned long prevMillis = millis();
int m_moistVal=512;
int m_oldAd0=-1;
int hue = 0;

int m_adcIntVal=0;
int m_adcIndex=0;
int m_adcCurVal=0;

//Return avg of 20 samples
int smoothedADC()
{
     m_adcIntVal+=analogRead(A0);
     m_adcIndex++;
     if(m_adcIndex == 20)
     {
       m_adcCurVal=m_adcIntVal/(20);
       m_adcIndex=0;
       m_adcIntVal=0;
     }
      return m_adcCurVal;
}

void loop() {
  webSocket.loop();                           // constantly check for websocket events
  iotWebConf.doLoop();                        // Update
  server.handleClient();                      // run the server
  ArduinoOTA.handle();                        // listen for OTA events

  delay(9);                                   //  wait 9ms  
}



void setSliderValue(int val)
{
     //The message is sent in JSON format :  '{"A0":"0"}'    obj-A0 and value 
     String JSONmessage = "{\"A0\":\"" + String(val)+"\"}";
     webSocket.broadcastTXT(JSONmessage);    // send message to all connected web-sockets
}

/*__________________________________________________________SETUP_FUNCTIONS__________________________________________________________*/



void startOTA() { // Start the OTA service
  ArduinoOTA.setHostname(OTAName);
  ArduinoOTA.setPassword(OTAPassword);

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\r\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("OTA ready\r\n");
}

void startSPIFFS() { // Start the SPIFFS and list all contents
  SPIFFS.begin();                             // Start the SPI Flash File System (SPIFFS)
  Serial.println("SPIFFS started. Contents:");
  {
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {                      // List the file system contents
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      Serial.printf("\tFS File: %s, size: %s\r\n", fileName.c_str(), formatBytes(fileSize).c_str());
    }
    Serial.printf("\n");
  }
}

void startWebSocket() { // Start a WebSocket server
  webSocket.begin();                          // start the websocket server
  webSocket.onEvent(webSocketEvent);          // if there's an incomming websocket message, go to function 'webSocketEvent'
  Serial.println("WebSocket server started.");
}

void startMDNS() { // Start the mDNS responder
  MDNS.begin(mdnsName);                        // start the multicast domain name server
  Serial.print("mDNS responder started: http://");
  Serial.print(mdnsName);
  Serial.println(".local");
}

void startServer() { // Start a HTTP server with a file read handler and an upload handler
  server.on("/edit.html",  HTTP_POST, []() {  // If a POST request is sent to the /edit.html address,
    server.send(200, "text/plain", ""); 
  }, handleFileUpload);                       // go to 'handleFileUpload'

  server.on("/config", []{ iotWebConf.handleConfig(); });
  server.onNotFound(handleNotFound);          // if someone requests any other file or page, go to function 'handleNotFound'
                                              // and check if the file exists

  server.begin();                             // start the HTTP server
  Serial.println("HTTP server started.");
}

/*__________________________________________________________SERVER_HANDLERS__________________________________________________________*/

void handleNotFound(){ // if the requested file or page doesn't exist, return a 404 not found error
  if(!handleFileRead(server.uri())){          // check if the file exists in the flash memory (SPIFFS), if so, send it
    server.send(404, "text/plain", "404: File Not Found");
  }
}

bool handleFileRead(String path) { // send the right file to the client (if it exists)
  Serial.println("handleFileRead: " + path);
  
  if (path.endsWith("/")) path += "index.html";          // If a folder is requested, send the index file

  
  String contentType = getContentType(path);             // Get the MIME type
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) { // If the file exists, either as a compressed archive, or normal
    if (SPIFFS.exists(pathWithGz))                         // If there's a compressed version available
      path += ".gz";                                         // Use the compressed verion
    File file = SPIFFS.open(path, "r");                    // Open the file
    size_t sent = server.streamFile(file, contentType);    // Send it to the client
    file.close();                                          // Close the file again
    Serial.println(String("\tSent file: ") + path);

    return true;
  }
  Serial.println(String("\tFile Not Found: ") + path);   // If the file doesn't exist, return false
  return false;
}

void handleFileUpload(){ // upload a new file to the SPIFFS
  HTTPUpload& upload = server.upload();
  String path;
  if(upload.status == UPLOAD_FILE_START){
    path = upload.filename;
    if(!path.startsWith("/")) path = "/"+path;
    if(!path.endsWith(".gz")) {                          // The file server always prefers a compressed version of a file 
      String pathWithGz = path+".gz";                    // So if an uploaded file is not compressed, the existing compressed
      if(SPIFFS.exists(pathWithGz))                      // version of that file must be deleted (if it exists)
         SPIFFS.remove(pathWithGz);
    }
    Serial.print("handleFileUpload Name: "); Serial.println(path);
    fsUploadFile = SPIFFS.open(path, "w");            // Open the file for writing in SPIFFS (create if it doesn't exist)
    path = String();
  } else if(upload.status == UPLOAD_FILE_WRITE){
    if(fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize); // Write the received bytes to the file
  } else if(upload.status == UPLOAD_FILE_END){
    if(fsUploadFile) {                                    // If the file was successfully created
      fsUploadFile.close();                               // Close the file again
      Serial.print("handleFileUpload Size: "); Serial.println(upload.totalSize);
      server.sendHeader("Location","/success.html");      // Redirect the client to the success page
      server.send(303);
    } else {
      server.send(500, "text/plain", "500: couldn't create file");
    }
  }
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) { // When a WebSocket message is received
  switch (type) {
    case WStype_DISCONNECTED:             // if the websocket is disconnected
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED: {              // if a new websocket connection is established
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
      }
      break;
    case WStype_TEXT:                     // if new text data is received
       //Serial.printf("[%u] get Text: %s\n", num, payload);
        if (payload[0] == 'M') {                      // the browser sends an M when moisture alarm value is changed
          //Simple secoding of sendt data..
           m_moistVal = (uint32_t) strtol((const char *) &payload[1], NULL, 10);   // moisturevalue; }
           //Serial.printf("New moist: %d\n", m_moistVal);
           String JSONmessage = "{\"MoistAlarmValue\":\"" + String(m_moistVal)+"\"}";
            webSocket.broadcastTXT(JSONmessage);    // send message to all connected web-sockets
        }
        else if (payload[0] == 'I') {                      // the browser sends an M when moisture alarm value is changed
              //Update GUI  <- Websocket connected on client -> send initial data to html-form
              String JSONmessage = "{\"MoistAlarmValue\":\"" + String(m_moistVal)+"\"}";
             webSocket.broadcastTXT(JSONmessage);    // send message to all connected web-sockets
             
             JSONmessage = "{\"A0\":\"" + String(m_oldAd0)+"\"}";
             webSocket.broadcastTXT(JSONmessage);    // send message to all connected web-sockets

        }

      break;
  }
}

/*__________________________________________________________HELPER_FUNCTIONS__________________________________________________________*/

String formatBytes(size_t bytes) { // convert sizes in bytes to KB and MB
  if (bytes < 1024) {
    return String(bytes) + "B";
  } else if (bytes < (1024 * 1024)) {
    return String(bytes / 1024.0) + "KB";
  } else if (bytes < (1024 * 1024 * 1024)) {
    return String(bytes / 1024.0 / 1024.0) + "MB";
  }
}

String getContentType(String filename) { // determine the filetype of a given filename, based on the extension
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}
