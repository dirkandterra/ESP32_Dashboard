#ifndef GandL_h
#define GandL_h

	void sendInfo(uint8_t gauge, uint16_t value);
  void sendVFD(uint8_t *c, uint8_t n);
	void testVFD(uint16_t value);
  extern void vfdPrep(void);
  extern void DecodeInit(void);
  void updateGuages_Lights(void);

#endif
