#include "arduino.h"
#include "config.h"
#include "odspeedmeter.h"

OdSpeedMeter::OdSpeedMeter() : speed(0), odometer(0), speedUpdateTime(0), odometerUpdateTime(0) 
{
}

void OdSpeedMeter::update() {
	int fTime = millis();

	// speedoeter
	int delta = fTime - speedUpdateTime;
	if (delta > 1000 / SPEED_FREQ) {
		speedUpdateTime = fTime;
	}

	// odometer
	delta = fTime - odometerUpdateTime;
	if (delta > 1000 / ODOMETER_FREQ) {
		odometer += speed / 3600. / 1000. * delta;
		odometerUpdateTime = fTime; 
	}
}

char* OdSpeedMeter::getSpeed(char* buffer) {
	strcpy(buffer, "Tmp: ");
	strcpy(buffer + 5, int(speed));
	return buffer;
}

char* OdSpeedMeter::getOdometer(char* buffer) {
    char tmp[24];

    dtostrf(int(odometer * 10) / 10., 0, 1, tmp);
    sprintf(buffer, "%skm", tmp);
	return buffer;
}