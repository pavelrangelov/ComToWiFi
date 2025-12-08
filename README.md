Tool to implement serial port over WiFi for esp32
                       +------------------------------------------------+     +----------------------+        +-----------------+
                       |                     com0com                    |     |       Com2Wifi       |        |      ESP32      |
+----------------+     |                                                |     |          tx tcp port | -----> |                 |
|some application| <-> | (virt serial port 4) <---->virt serial port 5) | <-> |          rx tcp port | <----- |          (uart) | <--->
+----------------+     |                                                |     |        ctrl tcp port | <----> |                 |
                       +------------------------------------------------+     +----------------------+        +-----------------+
