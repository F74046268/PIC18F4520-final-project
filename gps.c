#include "gps.h"

// acquired GPS coordinates
uint8_t hour, minute, seconds, year, month, day;
uint16_t milliseconds;

float latitude, longitude, geoidheight, altitude;
float speed, angle, magvariation, HDOP;

char lat, lon, mag;

boolean fix;
uint8_t fixquality, satellites;

boolean paused;


uint8_t parseHex(char c)
{
    if (c <= '9')
        return c - '0';    //0~9的文字 轉成 0~9的數字

    if (c <= 'F')
        return (c - 'A') + 10;   //A~F的文字 轉成 10~15的數字

    return 0;
}

boolean GPS_parse(char *nmea)
{
    // do checksum check

    // first look if we even have one
    if (nmea[strlen(nmea)-4] == '*') {
        uint16_t sum = parseHex(nmea[strlen(nmea)-3]) * 16;
        sum += parseHex(nmea[strlen(nmea)-2]);

        // check checksum
        uint8_t i;
        for (i=1; i < (strlen(nmea)-4); i++) {  // * 號之前的值做XOR, 當結果的bits為偶數個1,XOR輸出為0,checksum成立
            sum ^= nmea[i];
        }
        if (sum != 0) {
            // bad checksum :(
            return false;
        }
    }

    // look for a few common sentences
    if (strstr(nmea, "$GPGGA")) {
        // found GGA
        char *p = nmea;
        // get time
        p = strchr(p, ',')+1;   //逗號後面的string
        float timef = atof(p); //string轉float
        uint32_t time = timef;
        hour = time / 10000;   // 共六位數字 時時 分分 秒秒
        minute = (time % 10000) / 100;
        seconds = (time % 100);

        // milliseconds = fmod(timef, 1.0) * 1000;

        // parse out latitude
        p = strchr(p, ',')+1;
        latitude = atof(p);     //緯度 度分分.分分分分

        p = strchr(p, ',')+1;
        if (p[0] == 'N') lat = 'N';
        else if (p[0] == 'S') lat = 'S';
        else if (p[0] == ',') lat = 0;
        else return false;

        // parse out longitude
        p = strchr(p, ',')+1;
        longitude = atof(p);    //經度 度分分.分分分分

        p = strchr(p, ',')+1;
        if (p[0] == 'W') lon = 'W';
        else if (p[0] == 'E') lon = 'E';
        else if (p[0] == ',') lon = 0;
        else return false;

        p = strchr(p, ',')+1;
        fixquality = atoi(p);   //GPS等級，0：表示資料可用；1：非DGPS定位資料；2：DGPS定位資料

        p = strchr(p, ',')+1;
        satellites = atoi(p);   //所使用的衛星數

        p = strchr(p, ',')+1;
        HDOP = atof(p);        //平面精度指標

        p = strchr(p, ',')+1;
        altitude = atof(p);     //天線高度(以平均海水面為基準)
        p = strchr(p, ',')+1;
        p = strchr(p, ',')+1;
        geoidheight = atof(p);   //大地起伏值
        return true;
    }
    if (strstr(nmea, "$GPRMC")) {
        // found RMC
        char *p = nmea;

        // get time
        p = strchr(p, ',')+1;
        float timef = atof(p);
        uint32_t time = timef;
        hour = time / 10000;
        minute = (time % 10000) / 100;
        seconds = (time % 100);

        // milliseconds = fmod(timef, 1.0) * 1000;

        p = strchr(p, ',')+1;   //定位狀態，A：資料可用，V：資料不可用。
        if (p[0] == 'A')
            fix = true;
        else if (p[0] == 'V')
            fix = false;
        else
            return false;

        // parse out latitude
        p = strchr(p, ',')+1;
        latitude = atof(p);

        p = strchr(p, ',')+1;
        if (p[0] == 'N') lat = 'N';
        else if (p[0] == 'S') lat = 'S';
        else if (p[0] == ',') lat = 0;
        else return false;

        // parse out longitude
        p = strchr(p, ',')+1;
        longitude = atof(p);

        p = strchr(p, ',')+1;
        if (p[0] == 'W') lon = 'W';
        else if (p[0] == 'E') lon = 'E';
        else if (p[0] == ',') lon = 0;
        else return false;

        // speed    海里 在main,c裡轉轉成公里
        p = strchr(p, ',')+1;
        speed = atof(p);

        // angle
        p = strchr(p, ',')+1;
        angle = atof(p);

        //ddmmyy
        p = strchr(p, ',')+1;
        uint32_t fulldate = atof(p);
        day = fulldate / 10000;
        month = (fulldate % 10000) / 100;
        year = (fulldate % 100);

        // we dont parse the remaining, yet!
        return true;
    }

    return false;
}

void GPS_common_init(void)
{
    hour = minute = seconds = year = month = day = fixquality = satellites = 0; // uint8_t
    lat = lon = mag = 0; // char
    fix = false; // boolean
    milliseconds = 0; // uint16_t
    latitude = longitude = geoidheight = altitude = speed = angle = magvariation = HDOP = 0.0; // float
}

GPS_DATE_INFO GPS_getDateInfo()
{
    GPS_DATE_INFO retVal;

    retVal.seconds = seconds;
    retVal.minute = minute;
    retVal.year = year;
    retVal.month = month;
    retVal.day = day;
    retVal.hour = hour + 8;
    if (retVal.hour >= 24) {
        retVal.day = day + 1;
        retVal.hour -= 24;
    }
    

    if (month == 1  || month == 3  || month == 5 || month == 7 ||
            month == 8  || month == 10 || month == 12) {
        if (retVal.day == 32) {
            retVal.month++;
            retVal.day = 1;
        }
    } else if (month == 4  || month == 6  || month == 9 || month == 11) {
        if (retVal.day == 31) {
            retVal.month++;
            retVal.day = 1;
        }
    } else if (month == 2) {
        if (retVal.day == 29 && year % 4 != 0) {
            retVal.month = 3;
            retVal.day = 1;
        } else if (day == 30 && year % 4 == 0) { // leap year
            retVal.month = 3;
            retVal.day = 1;
        }
    }
    
    if(retVal.month == 13){
        retVal.year = year + 1;
        retVal.month = 1;
    }
    
  //  retVal.year = retVal.month == 13 ? year + 1 : year;
    retVal.year = 19;
    retVal.milliseconds = milliseconds;

    return retVal;
}

GPS_SIGNAL_INFO GPS_getSignalInfo()
{
    GPS_SIGNAL_INFO retVal;

    retVal.fix = fix;
    retVal.fixquality = fixquality;
    retVal.satellites = satellites;

    return retVal;
}

GPS_LOCATION_INFO GPS_getLocationInfo()
{
    GPS_LOCATION_INFO retVal;

    retVal.latitude = latitude;
    retVal.longitude = longitude;
    retVal.geoidheight = geoidheight;
    retVal.altitude = altitude;
    retVal.speed = speed;
    retVal.angle = angle;
    retVal.magvariation = magvariation;
    retVal.HDOP = HDOP;

    return retVal;
}