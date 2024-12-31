

#include <Arduino.h>

//#define V3_2_0
#ifdef 	V3_2_0
	#include "v3.2.0/espem_main_001.h"
#endif

#define embui4
#ifdef 	embui4
	#include "embui4/main_002.h"
#endif


void setup(){
	#ifdef 	V3_2_0
		espem_init_v320();
	#endif 

	#ifdef 	embui4
		espem_init_embui4();
	#endif 
}

void loop(){
	#ifdef 	V3_2_0
		espem_run_v320();
	#endif 
	
	#ifdef 	embui4
		espem_run_embui4();
	#endif 
}