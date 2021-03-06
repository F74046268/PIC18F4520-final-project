#ifndef GPS_H
#define	GPS_H
#ifdef	__cplusplus
extern "C" {
#endif

#include <pic18f4520.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>


// how long to wait when we're looking for a response
#define MAXWAITSENTENCE 5

// boolean data type
#define boolean unsigned char
#define true 1
#define false 0

// other data types
#define uint8_t unsigned char
#define uint16_t unsigned int
#define uint32_t unsigned long

// structures
typedef struct _GPS_DATE_INFO {
    uint8_t hour;
    uint8_t minute;
    uint8_t seconds;
    uint8_t year;
    uint8_t month;
    uint8_t day;
    uint16_t milliseconds;
} GPS_DATE_INFO;

typedef struct _GPS_SIGNAL_INFO {
    boolean fix;
    uint8_t fixquality;
    uint8_t satellites;
} GPS_SIGNAL_INFO;

typedef struct _GPS_LOCATION_INFO {
    float latitude; //緯度
    float longitude; //經度
    float geoidheight; //大地水準面
    float altitude;  //海拔
    float speed; //速度
    float angle; //GPS接收機與它所接收到訊號的衛星所構成的角度
    float magvariation; //磁變
    float HDOP; //GPS精度因子
} GPS_LOCATION_INFO;

// internal functions
void GPS_common_init(void);
char *GPS_lastNMEA(void); //上一筆GPS傳的資料 NMEA為資料傳輸協定的格式
void GPS_pause(boolean b);
boolean is_GPS_paused();
boolean GPS_parse(char *);
char GPS_read(void);
boolean GPS_newNMEAreceived(void);

// get GPS information
GPS_DATE_INFO GPS_getDateInfo(void);
GPS_SIGNAL_INFO GPS_getSignalInfo(void);
GPS_LOCATION_INFO GPS_getLocationInfo(void);


#ifdef	__cplusplus
}
#endif
#endif	/* GPS_H */