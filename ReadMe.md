# _ESP_Dashboard_  

_This project uses the a Nano with a MCP2515 to drive a 2004 Dodge Intrepid Dashboard._

![IntrepidSchematic](/img/NanoCluster.png)  

## How to use  

### Hardware Required  
* 2004 Dodge Intrepid Instrument Panel
* Connection:  

| Signal    | Intrepid  | Nano |
|-----------|-----------|------|
| 5V        | 5V		| 5V   |
| GLDATA    | GLDATA	| D4   |
| GLCLK     | GLCLK 	| D5   |
| LightLatch| LightLatch| D6   |
| Gauge CS  | Gauge CS  | D7   |
| VFDDATA   | VFDDATA   | D8   |
| VFDCLK	| VFDCLK	| D9   |
| VFD Pls	| VFD Pls	| D10  |
| Backlight | UC_LMP_DRV| D2   |
| PB Input  | ODO PB    | D3   |
| L Turn    | C2-5		| A0   |
| R Turn    | C1-8      | A1   |
| HighBeam  | C1-10     | A2   |
| OilPress  | C2-10     | A3   |
| ABS		| ??		| A4   |
| Airbag    | ??		| A5   |
| GND       | GND       | D7   |

* MCP2515 Breakout Board
* Connection: 

| MCP2515   | Nano |
|-----------|------|
| INT       |  -   |
| SCK		| D13  |
| MOSI      | D11  |
| MISO	    | D12  |
| CS to 5V  |  -   |
| GND       | GND  |
| 5V    	| 5V   |

### Arduino Library Needed  

```
https://github.com/coryjfowler/MCP_CAN_lib.git
```

* MCP_CAN_lib

### Intrepid Dashboard Help

* Intrepid Schematic (Not Complete):  

  ![IntrepidSchematic](/img/IntrepidSchematic.png)  