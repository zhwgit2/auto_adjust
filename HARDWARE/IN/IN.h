#ifndef __IN_H
#define __IN_H	 
#include "sys.h"

	   
#define   G1   PDin(11)
#define   G2   PDin(10)
#define   G3   PDin(9)
#define   G4   PDin(8)

#define   K1   PDin(12)
#define   K2   PDin(13)
#define   K3   PDin(14)
#define   K4   PDin(15)

extern  u8  Calibration_Dir;
extern  u8  Cali_Complete;
extern  u8  Calibration_Add;

void Input_Init(void);	 //≥ı ºªØ
	

#endif

