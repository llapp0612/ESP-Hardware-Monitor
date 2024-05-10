# ESP Hardware Monitor

This project involves building an ESP32 hardware monitor that displays PC hardware data on a 3.5-inch touch LCD screen. The monitor utilizes the following components:

1. Seengreat 3.5 Inch Touch LCD Screen for Arduino: This TFT resistive touch shield with a resolution of 480×320 provides a clear and engaging display for hardware data. Link: [Link-Text](https://www.amazon.de/dp/B0CHRFH48D?psc=1&ref=ppx_yo2ov_dt_b_product_details)
2. WINGONEER® Prototype PCB for Arduino UNO R3 Shield Board DIY: This PCB offers a solid platform for assembling the ESP32 and LCD screen. Link: [Link-Text](https://www.amazon.de/dp/B01FTVTJT2?psc=1&ref=ppx_yo2ov_dt_b_product_details)
3. AZDelivery ESP32 NodeMCU D1 R32 WLAN Development Board: This board provides WiFi and Bluetooth connectivity and is equipped with a CH340G chip. It serves as the central control unit of the monitor, facilitating communication with the Windows Form app via a serial connection. Link: [Link-Text](https://www.amazon.de/dp/B08BV3L92G?psc=1&ref=ppx_yo2ov_dt_b_product_details)

The data is collected via a Windows Form app in C# and then sent to the ESP32 through the serial connection. The PC app can be accessed via the following GitHub link: [Github](https://github.com/llapp0612/ESP-Hardware-Monitor-PC-App)

<iframe width="560" height="315" src="https://www.youtube.com/embed/xE64i9O95Yg" frameborder="0" allowfullscreen></iframe>

A detailed assembly guide for the ESP32 Hardware Monitor will be available soon. Stay tuned for updates!
