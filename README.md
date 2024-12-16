# Wi-Fi Captive Portal

This project implements an imitation of a EvilTwin attack using Esp32 microcontroller, it hosts a webserver that contains a html webpage with password user input label. 

## Ethical Use Warning

* This captive Wi-Fi code is intended for educational and ethical purposes only.

* It is designed to demonstrate how to create a captive portal for legitimate use cases,

* such as providing internet access in public spaces or for personal projects.

**Important:**

* - Do not use this code to intercept, monitor, or manipulate network traffic without the explicit

* - consent of the users involved.

* - Ensure that users are informed about the nature of the captive portal and any data that may

* - be collected during their use of the network.

* - Comply with all applicable laws and regulations regarding network usage and data privacy.

* - By using this code, you agree to use it responsibly and ethically.

## Features

- DNS redirection that supports many OS's (Chrome OS, linux, Windows...etc) and Web Browsers (Firefox, Chrome.. etc).

- Serves an HTML webpage when any client connects to the network.

- The victim's Password will appear in the Serial Monitor after he submits it.

## Compnents

- ESP32 WROOM 32U microcontroller.

- External Antenna.
  
  ### Note : This works with other esp boards too but the one i listed is what i used.

## IDE used

If you are using PlatformIO vscode extention then just copy the code and upload it to your code. 

However, if you are using Arduino IDE make sure to remove the "#include<Arduino.h" library.

## How to use ?

- Copy this code and upload it to your board.

- Place the it as close as possible to the original router.

- Hope the victim get tricked ;) .

## Adjustments

- Make sure to change the name of the target router place it close enougth that the board's signal suppresses the original router signal.

- You can leave the password NULL (open network...This is preferred btw) or you can set a password.

- Set your wi-fi ssid in the html code (line 4 from the buttom of the HTML section).

## License

This project is open-source and available under the MIT License.

---

Happy building! Feel free to raise issues or contribute to this project.
