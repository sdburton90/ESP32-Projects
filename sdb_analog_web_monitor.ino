//
//// Analog web monitor experiment based on Rui Santos code from randomnerds.com training module.  
//
// Load Wi-Fi library
#include <WiFi.h>

// Replace with your network credentials
const char* ssid     = "Your-SSID";
const char* password = "Your-Password";

int webport = 80;

// Set web server port number 
WiFiServer server(webport);

// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
String output16State = "off";
String output17State = "off";

// Assign input/output variables to GPIO pins
const int output16 = 16;   // reeally using 16 and 17
const int output17 = 17;
const int optIn = 36;     // optical light sensor on 36
int optValue = 0;

void setup() {
  Serial.begin(115200);
  // Initialize the output variables as outputs
  pinMode(output16, OUTPUT);
  pinMode(output17, OUTPUT);
  pinMode(optIn, INPUT);
  // Set outputs to LOW
  digitalWrite(output16, LOW);
  digitalWrite(output17, LOW);

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Port: " + String(webport) );
  server.begin();
}

void loop(){
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          if (currentLine.length() == 0) {    // Blank line is end of header.
            // checking if header is valid
            // dXNlcjpwYXNz = 'user:pass' (user:pass) base64 encode
            // Finding the right credential string, then loads web page
            if(header.indexOf("dXNlcjpwYXNz")) {   /** set to ("dXNlcjpwYXNz")>= 0 for authentication */
              // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
              // and a content-type so the client knows what's coming, then a blank line:
              client.println("HTTP/1.1 200 OK");
              client.println("Content-type:text/html");
              client.println("Connection: close");
              client.println();
              
              // turns the GPIOs on and off
              if (header.indexOf("GET /16/on") >= 0) {
                Serial.println("GPIO 16 on");
                output16State = "on";
                digitalWrite(output16, HIGH);
              } else if (header.indexOf("GET /16/off") >= 0) {
                Serial.println("GPIO 16 off");
                output16State = "off";
                digitalWrite(output16, LOW);
              } else if (header.indexOf("GET /17/on") >= 0) {
                Serial.println("GPIO 17 on");
                output17State = "on";
                digitalWrite(output17, HIGH);
              } else if (header.indexOf("GET /17/off") >= 0) {
                Serial.println("GPIO 17 off");
                output17State = "off";
                digitalWrite(output17, LOW);
              }
              // Display the HTML web page
              client.println("<!DOCTYPE html><html>");
              client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
              client.println("<link rel=\"icon\" href=\"data:,\">");
              // CSS to style the on/off buttons 
              // Feel free to change the background-color and font-size attributes to fit your preferences
              client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
              client.println(".button { background-color: #6CBF60; border: none; color: white; padding: 16px 40px;");
              client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
              client.println(".button2 {background-color: #3C9F40;}</style></head>");
              
              // Web Page Heading
              client.println("<body><h1>Your ESP32 Web Server</h1>");
              
              // Display current state, and ON/OFF buttons for GPIO 16  
              client.println("<p>GPIO 16 LED - State " + output16State + "</p>");
              // If the output26State is off, it displays the ON button       
              if (output16State=="off") {
                client.println("<p><a href=\"/16/on\"><button class=\"button\">Turn ON</button></a></p>");
              } else {
                client.println("<p><a href=\"/16/off\"><button class=\"button button2\">Turn OFF</button></a></p>");
              } 
                 
              // Display current state, and ON/OFF buttons for GPIO 17  
              client.println("<p>GPIO 17 LED - State " + output17State + "</p>");
              // If the output17State is off, it displays the ON button       
              if (output17State=="off") {
                client.println("<p><a href=\"/17/on\"><button class=\"button\">Turn ON</button></a></p>");
              } else {
                client.println("<p><a href=\"/17/off\"><button class=\"button button2\">Turn OFF</button></a></p>");
              }
              // read the state of the opto sensor
              delay(250);
              optValue = analogRead(optIn);            
              Serial.println("Light sensor value on GPIO" + String(optIn) + " is " + String(optValue));
              
              client.println("<p><br><b>Light sensor value on GPIO" + String(optIn) + " is " + String(optValue) + "</b></p>");
              client.println("</body></html>");
              
              // The HTTP response ends with another blank line
              client.println();
              // Break out of the while loop
              break;
            }
            // Wrong user or password, so HTTP request fails...   
            else {            
              client.println("HTTP/1.1 401 Unauthorized");
              client.println("WWW-Authenticate: Basic realm=\"Secure\"");
              client.println("Content-Type: text/html");
              client.println();
              client.println("<html>Authentication failed</html>");
            }   
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
