#include "gps.h"
int main() {
	int i;
	struct gps_data_t gpsdata;
	i = gps_open("localhost", "2947", &gpsdata);
	printf("open status is %d\n", i);

	(void)gps_stream(&gpsdata, WATCH_ENABLE | WATCH_JSON, NULL);
	printf("overall status is %d\n", gpsdata.status);
	while (gpsdata.status == 0) {
		if (gps_waiting(&gpsdata, 500)) {

			if (gps_read(&gpsdata) == -1) {
				printf("error inside read\n");
			}
			else {
				// We have data
				if (gpsdata.set && gpsdata.status > 0) {
					printf("data set\n");
					printf("latitude is %lf\n", gpsdata.fix.latitude);
				}
				else
					printf(".");
			}
		}
		else {
			printf("reconnecting\n");
			gps_stream(&gpsdata, WATCH_ENABLE | WATCH_JSON, NULL);
		}
	}

	(void)gps_stream(&gpsdata, WATCH_DISABLE, NULL);
	(void)gps_close(&gpsdata);
	return 0;
}
