# _ESP_Dashboard_  

_This project uses the a CANBus and an ESP32 to drive a 2004 Dodge Intrepid Dashboard._

![IntrepidSchematic](/img/ESPCluster.png)  

## How to use  

### Hardware Required  
* 2004 Dodge Intrepid Instrument Panel
* Connection:  

| Signal    | Intrepid  | ESP32 |
|-----------|-----------|-------|
| 5V        | 5V		|  5V   |
| GLDATA    | GLDATA	|  D13  |
| GLCLK     | GLCLK 	|  D14  |
| LightLatch| LightLatch|  D6   |
| Gauge CS  | Gauge CS  |  D7   |
| VFDDATA   | VFDDATA   |  D13  |
| VFDCLK	| VFDCLK	|  D14  |
| VFD Pls	| VFD Pls	|  D12  |
| Backlight | UC_LMP_DRV|  D15  |
| PB Input  | ODO PB    |  D0   |
| L Turn    | C2-5		|  D21  |
| R Turn    | C1-8      |  D22  |
| HighBeam  | C1-10     |  D23  |
| OilPress  | C2-10     |  D2   |
| ABS		| ??		|  D18  |
| Airbag    | ??		|  D19  |
| GND       | GND       |  GND  |
| CAN TX    |			|  D5	|
| CAN RX	|			|  D4	|



### Arduino Library Needed  

```
https://github.com/coryjfowler/MCP_CAN_lib.git
```

* MCP_CAN_lib

### Intrepid Dashboard Help

* Intrepid Schematic (Not Complete):  

  ![IntrepidSchematic](/img/IntrepidSchematic.png)  