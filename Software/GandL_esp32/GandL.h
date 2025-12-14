#ifndef GandL_h
#define GandL_h

typedef struct _tempData{
  uint16_t garageTemp;
  uint16_t currentTemp; // temp times 10
  uint16_t currentWind; //wind mph times 10
  uint8_t precipPercent;
  uint8_t forecast[10]; //tmax1,tmin1,tmax2......
}NodeRedData;

	void sendInfo(uint8_t gauge, uint16_t value);
  void sendVFD(uint8_t *c);
	void testVFD(uint16_t value);
  extern NodeRedData ServerData;
  extern void vfdWeatherPrep(void);
  extern void DecodeInit(void);
  void updateGuages_Lights(void);

#endif
