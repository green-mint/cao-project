#include <WiFi.h>              // Include WIFi Library for ESP32
#include <WebServer.h>         // Include WebSwever Library for ESP32
#include <ArduinoJson.h>       // Include ArduinoJson Library
#include "DHT.h"               // Include DHT Library
#include <WebSocketsServer.h>  // Include Websocket Library
#include "ESPAsyncWebServer.h"

#define DHTPIN 21      // DHT PIn
#define DHTTYPE DHT22  // DHT Type
#define LED1 15
#define LED2 2
#define LED3 4


const char* ssid = "saad";            // Your SSID
const char* password = "saad5215";  // Your Password

int interval = 100;                // virtual delay
unsigned long previousMillis = 0;  // Tracks the time since last event fired


void webSocketEvent(byte num, WStype_t type, uint8_t* payload, size_t length);

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta http-equiv="X-UA-Compatible" content="IE=edge">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Document</title>
  <link rel="stylesheet" href="style.css">

  <style>
    #container {
    display: flex;
    flex-direction: row;
    height: 150px;
    justify-content: space-between;
}

.bar {
    background-color: black;
}
  </style>
</head>
<body>
  <button id="btn">Button</button>

  <div id="container">
  </div>
  
  <script>
    window.addEventListener("load", () => {
  const btn = document.getElementById("btn");
  let Socket = new WebSocket("ws://" + "192.168.43.241" + ":81/");

  Socket.onopen = () => {
    console.log("Connected to server");
  };

  const NUM_BARS = 3;
  const NUM_NOTES = 16;
  const BAR_WIDTH = 20;
  const THRESHOLDS = [200, 100, 45];

  let audioSource;
  let analyser;

  const container = document.getElementById("container");
  container.style.width = `${NUM_BARS * BAR_WIDTH}px`;

  btn.addEventListener("click", () => {
    const audio = new Audio("f1theme.mp3");
    const audioCtx = new AudioContext();
    audio.play();
    audioSource = audioCtx.createMediaElementSource(audio);
    analyser = audioCtx.createAnalyser();
    audioSource.connect(analyser);
    analyser.connect(audioCtx.destination);

    analyser.fftSize = NUM_NOTES * 2;

    const frequencyData = new Uint8Array(analyser.frequencyBinCount);
    const barBinLength = parseInt(NUM_NOTES / NUM_BARS);

    // create divs for each bar
    for (let i = 0; i < NUM_BARS; i++) {
      const bar = document.createElement("div");
      bar.id = `bar${i}`;
      bar.style.width = `${BAR_WIDTH}px`;
      bar.classList.add("bar");
      container.appendChild(bar);
    }

    // set interval
    const interval = setInterval(() => {
      analyser.getByteFrequencyData(frequencyData);
      let data = "";

      for (let i = 0; i < NUM_BARS; i++) {
        const bar = document.getElementById(`bar${i}`);
        let barMean = 0;
        for (let j = 0; j < barBinLength; j++) {
          barMean += frequencyData[i * barBinLength + j];
        }
        barMean /= barBinLength;
        bar.style.height = `${(barMean * 100) / 256}%`;

        if (barMean > THRESHOLDS[i]) {
          data += "1";
        } else {
          data += "0";
        }
      }
      Socket.send(data);
    }, 300);

    audio.addEventListener("ended", () => {
      clearInterval(interval);
      container.innerHTML = "";
    });
  });
});

  </script>
</body>
</html>
)rawliteral";

WebServer server(80);                               // create instance for web server on port "80"
WebSocketsServer webSocket = WebSocketsServer(81);  //create instance for webSocket server on port"81"


void setup() {
  // put your setup code here, to run once:
  pinMode(LED1, OUTPUT);                    // Set PIN22 As output(LED Pin)
  pinMode(LED2, OUTPUT);                    // Set PIN22 As output(LED Pin)
  pinMode(LED3, OUTPUT);                    // Set PIN22 As output(LED Pin)
  
  Serial.begin(115200);                    // Init Serial for Debugging.
  WiFi.begin(ssid, password);              // Connect to Wifi
  while (WiFi.status() != WL_CONNECTED) {  // Check if wifi is connected or not
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", []() {
    server.send(200, "text/html", index_html);
  });

  server.begin();                     // init the server
  webSocket.begin();                  // init the Websocketserver
  webSocket.onEvent(webSocketEvent);  // init the webSocketEvent function when a websocket event occurs
}


void loop() {
  server.handleClient();                                              // webserver methode that handles all Client
  webSocket.loop();                                                   // websocket server methode that handles all Client
  unsigned long currentMillis = millis();                             // call millis  and Get snapshot of time
  if ((unsigned long)(currentMillis - previousMillis) >= interval) {  // How much time has passed, accounting for rollover with subtraction!
    previousMillis = currentMillis;  // Use the snapshot to set track time until next event
  }
}

// This function gets a call when a WebSocket event occurs
void webSocketEvent(byte num, WStype_t type, uint8_t* payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:  // enum that read status this is used for debugging.
      Serial.print("WS Type ");
      Serial.print(type);
      Serial.println(": DISCONNECTED");
      break;
    case WStype_CONNECTED:  // Check if a WebSocket client is connected or not
      Serial.print("WS Type ");
      Serial.print(type);
      Serial.println(": CONNECTED");
      digitalWrite(LED1, LOW);
      break;
    case WStype_TEXT:  // check responce from client
      // Serial.println(); // the payload variable stores teh status internally
      
      if (payload[0] == '1') digitalWrite(LED1, HIGH);
      else digitalWrite(LED1, LOW);
      if (payload[1] == '1') digitalWrite(LED2, HIGH);
      else digitalWrite(LED2, LOW);
      if (payload[2] == '1') digitalWrite(LED3, HIGH);
      else digitalWrite(LED3, LOW);

      delay(150);

      digitalWrite(LED1, LOW);
      digitalWrite(LED2, LOW);
      digitalWrite(LED3, LOW);

      break;
  }
}
