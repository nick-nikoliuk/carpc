#ifndef __OD_SPEED_METER_H__
#define __OD_SPEED_METER_H__


class OdSpeedMeter {

private:
	float speed;
	float speedUpdateTime;
	float odometer;
	float odometerUpdateTime;
public:
	OdSpeedMeter();
	void update();
	char* getSpeed(char* buffer);	
	char* getOdometer(char* buffer);	

};

#endif