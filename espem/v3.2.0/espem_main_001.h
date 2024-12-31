/*  ESPEM - ESP Energy monitor
 *  A code for ESP32 based boards to interface with PeaceFair PZEM PowerMeters
 *  It can poll/collect PowerMeter data and provide it for futher processing in text/json format
 *
 *  (c) Emil Muratov 2018 - 2023
 */

//#pragma once

#define FW_NAME				   "espem"

#define FW_VERSION_MAJOR	   3
#define FW_VERSION_MINOR	   2
#define FW_VERSION_REVISION	   0

// make version as integer
#define FW_VERSION			   ((FW_VERSION_MAJOR) << 16 | (FW_VERSION_MINOR) << 8 | (FW_VERSION_REVISION))

// make version as string
#define FW_VERSION_STRING	   TOSTRING(FW_VERSION_MAJOR) "." TOSTRING(FW_VERSION_MINOR) "." TOSTRING(FW_VERSION_REVISION)

#define ESPEM_JSAPI_VERSION	   2
#define ESPEM_UI_VERSION	   2

#define BAUD_RATE			   115200  // serial debug port baud rate
#define HTTP_VER_BUFSIZE	   256

#define WEBUI_PUBLISH_INTERVAL 20

//#define     G_B00_PZEM_MODEL_PZEM003			1
//#define     G_B00_PZEM_MODEL_PZEM004V3 

#define     G_B00_PZEM_DUMMY


// Sketch configuration
#include <ESPAsyncWebServer.h>

#include "config.h"
#include "globals.h"	// EmbUI macro's for LOG
#include "uistrings.h"	// non-localized text-strings


// EMBUI
void create_parameters();  // декларируем для переопределения weak метода из фреймворка для WebUI
void sync_parameters();

// WiFi connection callback
void onSTAGotIP();
// Manage network disconnection
void onSTADisconnected();

void wver(AsyncWebServerRequest *request);

// Main headers
// #include "main.h"


#include <EmbUI.h>


#include "espem.h"
#include "interface.h"

extern "C" int	  clock_gettime(clockid_t unused, struct timespec *tp);

// PROGMEM strings
// sprintf template for json version data
static const char PGverjson[] = "{\"ChipID\":\"%s\",\"Flash\":%u,\"SDK\":\"%s\",\"firmware\":\"" FW_NAME "\",\"version\":\"" FW_VERSION_STRING "\",\"git\":\"%s\",\"CPUMHz\":%u,\"RAM Heap size\":%u,\"RAM Heap free\":%u,\"PSRAM size\":%u,\"PSRAM free\":%u,\"Uptime\":%u}";

// Our instance of espem

#if  defined(G_B00_PZEM_MODEL_PZEM003)
	Espem<pz003::metrics> *espem = nullptr;
	////Espem* espem		 = new Espem();
	//Espem* espem		  	= nullptr;

#elif defined(G_B00_PZEM_MODEL_PZEM004V3)
	Espem<pz004::metrics> *espem = nullptr;
#endif 

// ----
// MAIN Setup
void espem_init_v320() {

	#ifdef ESPEM_DEBUG
		ESPEM_DEBUG.begin(BAUD_RATE);  // start hw serial for debugging
	#endif

	LOG(println, F("Starting EspEM..."));

	// Start framework, load config and connect WiFi
	embui.begin();
	embui_actions_register();

	// create and run ESPEM object

	#if defined(G_B00_PZEM_MODEL_PZEM003)
            Espem<pz003::metrics> *espem = new Espem<pz003::metrics>();
		////espem = new Espem();
	#elif defined(G_B00_PZEM_MODEL_PZEM004V3)
            Espem<pz004::metrics> *espem = new Espem<pz004::metrics>();
		////espem = new Espem();
	#endif

	

	if (espem && espem->begin(embui.paramVariant(V_UART),
							  embui.paramVariant(V_RX),
							  embui.paramVariant(V_TX))) {
		espem->ds.setEnergyOffset(embui.paramVariant(V_EOFFSET));

		// postpone TimeSeries setup until NTP aquires valid time
		TimeProcessor::getInstance().attach_callback([espem]() {
		/////TimeProcessor::getInstance().attach_callback([]() {
				if(espem){
					espem->set_collector_state(mcstate_t::MC_RUN);
				}
				// we only need that setup once
				TimeProcessor::getInstance().dettach_callback(); 
			});
	}

	embui.server.on("/fw", HTTP_GET, [](AsyncWebServerRequest *request) { wver(request); });

	embui.setPubInterval(WEBUI_PUBLISH_INTERVAL);
}

// MAIN loop
void espem_run_v320() {
	embui.handle();
}

// send HTTP responce, json with controller/fw versions and status info
void wver(AsyncWebServerRequest *request) {
	char	 buff[HTTP_VER_BUFSIZE];

	timespec tp;
	clock_gettime(0, &tp);

	snprintf_P(buff, sizeof(buff), PGverjson,
			   ESP.getChipModel(),
			   ESP.getFlashChipSize(),
			   ESP.getSdkVersion(),

				#ifdef GIT_REV
					GIT_REV,
				#else
					"-",
				#endif
				
			   ESP.getCpuFreqMHz(),
			   ESP.getHeapSize(), ESP.getFreeHeap(),	// RAM
			   ESP.getPsramSize(), ESP.getFreePsram(),	// PSRAM
			   (uint32_t)tp.tv_sec
			   );

	request->send(200, FPSTR(PGmimejson), buff);
}
//
