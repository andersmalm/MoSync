/*
Copyright (C) 2011 MoSync AB

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License,
version 2, as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
MA 02110-1301, USA.
*/
/**
* Usage example for the location API.
* A Moblet that retrieves Location events and prints raw data to the console.
*
* WARNING: The location API is experimental, not fully tested. It may not work as advertised.
* Implementation details are likely to change in the future.
*/


#include <conprint.h>
#include <maassert.h>
#include <maprofile.h>
#include <MAUtil/Moblet.h>

using namespace MAUtil;

class LocationMoblet : public Moblet {
private:
	int mLastTime;
	bool active;
public:

	/**
	* Constructor
	*
	* Prints an initial message and attempts to begin
	* collecting location information.
	*/
	LocationMoblet() {
#ifdef MA_PROF_SUPPORT_STYLUS
		printf("Instructions:\n-Press Fire/LSK to start/stop.\n-Tap the screen to start/stop.\n-Press 0/RSK to exit.\n");
#else	// MA_PROF_SUPPORT_STYLUS
		printf("Instructions:\n-Press Fire/LSK to start/stop.\n-Press 0/RSK to exit.\n");
#endif	// MA_PROF_SUPPORT_STYLUS

		start();
	}

	/**
	* Begins collecting location information.
	*/
	void start() {
		int res = maLocationStart();
		printf("Start: %i\n", res);
		active = res >= 0;
		mLastTime = maGetMilliSecondCount();
	}

	/**
	* Processes the user's key presses.
	*
	* @param keyCode     The key code of the key that was pressed
	*/
	void keyPressEvent(int keyCode, int nativeCode) {
		if(keyCode == MAK_0 || keyCode == MAK_SOFTRIGHT || keyCode == MAK_BACK)
			maExit(0);
		if(keyCode == MAK_FIRE || keyCode == MAK_SOFTLEFT) {
			if(active) {
				int res = maLocationStop();
				printf("Stop: %i\n", res);
				active = false;
			} else {
				start();
			}
		}
	}

	void pointerPressEvent(MAPoint2d /*point*/) {
		if(active) {
			int res = maLocationStop();
			printf("Stop: %i\n", res);
			active = false;
		} else {
			start();
		}
	}

	/**
	* Handles the custom location events generated by the location API.
	*/
	void customEvent(const MAEvent& event) {
		if(event.type == EVENT_TYPE_LOCATION) {
			MALocation& loc = *(MALocation*)event.data;

			if(loc.lon < -180.0 || loc.lon > 180.0 || loc.lat < -90.0 || loc.lat > 90.0) {
				printf("invalid lat or lon\n");
			} else {
				printf("%i %.8g %.8g %.4g %.4g %.4g\n",
					loc.state, loc.lat, loc.lon, loc.horzAcc, loc.vertAcc, loc.alt);
			}

			printf("%i ms\n", maGetMilliSecondCount() - mLastTime);
			mLastTime = maGetMilliSecondCount();
		} else if(event.type == EVENT_TYPE_LOCATION_PROVIDER) {
			const char *strings[]= {
				"AVAILABLE",
				"TEMPORARILY_UNAVAILABLE",
				"OUT_OF_SERVICE"
			};

			printf("gps provider: %s\n", strings[event.state-1]);

		} else {
			printf("custom event %i\n", event.type);
		}
	}
};

extern "C" int MAMain() {
	InitConsole();
	gConsoleLogging = 1;
	Moblet::run(new LocationMoblet());
	return 0;
}
