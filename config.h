/*
  config.h - config the keywords for Victron.DIRECT
*/

#define MPPT_75_10
// MPPT 75 | 10
#ifdef MPPT_75_10

	const byte buffsize = 32;

	const byte num_keywords = 9;
	const char * keywords[] = {
	"V",
	"I",
	"VPV",
	"PPV",
	"CS",
	"H20",
	"H21",
	"H22",
	"H23"
	};
	const unsigned int nomb_keywords = sizeof(keywords) / sizeof(*keywords);

	#define V 0     // ScV
	#define I 1     // ScI
	#define VPV 2   // PVV
	#define PPV 3   // PVI = PVV / VPV
	#define CS 4    // ScS
	#define H20 5
	#define H21 6
	#define H22 7
	#define H23 8
#endif