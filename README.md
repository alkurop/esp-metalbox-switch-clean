# Esp32 BLE Gamepad that works on Android and iPhone

Targets Esp32c3

In order to work with iOS and MacOS as an HID device, it emulates Google Stadia gameped by placing appropriate vid and pid, and also uses device repord that matches Google Stadia.

Features:
 - ble based on esp_hid/bluedroid example
 - 3 buttons
 - 1 led
 - deep sleep after 10 minutes of inactivity (based on esp timer)
 - wake up from deep slip on button click
 - battery checker every 5 minutes (based on esp timer). Battery checker pulls up the checker pin, reads from the reader pin, and then pulls down the checker pin. Afterwards it send battery service notification with BLE

