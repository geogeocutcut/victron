/*
  config.h - config the keywords for Victron.DIRECT
*/


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

// BMV 700
#ifdef BMV_700

	const byte buffsize = 32;

	const byte num_keywords = 9;
	const char * keywords[] = {
	"V",
	"I",
	"P", // W
	"CE", // mA value "---" if not synchronized
	"TTG", // Minutes value "---" if not synchronized and -1 = infinite
	"SOC", // % value "---" if not synchronized
	"H1", // mA deepest discharge
	"H2", // mA last discharge
	"H3" // mA avg discharge
	};
	const unsigned int nomb_keywords = sizeof(keywords) / sizeof(*keywords);

	#define V 0     // ScV
	#define I 1     // ScI
	#define P 2   // PVV
	#define CE 3   // PVI = PVV / VPV
	#define TTG 4   // PVI = PVV / VPV
	#define SOC 5    // ScS
	#define H1 6
	#define H2 7
	#define H3 8
#endif