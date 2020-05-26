#include <TimeLib.h>
#include <ArduinoP13.h>
#include <Servo.h>

Servo elevation;
Servo azimuth;

// Current UTC [NOT LOCAL TIME]
int CurrentHour   = 1;
int CurrentMin    = 15;
int CurrentSec    = 00;
int CurrentDay    = 21;
int CurrentMonth  = 5;
int CurrentYear   = 2020;


// Set TLEs of your desired Satellite
const char *tleName = "METEOR-M 2";
const char *tlel1   = "1 40069U 14037A   20125.52516225 -.00000051  00000-0 -42286-5 0  9997";
const char *tlel2   = "2 40069  98.5072 164.7117 0006720  56.5344 303.6479 14.20671878301977";

/* Some Frequently used TLEs:
ISS (ZARYA)
1 25544U 98067A   20141.44172200  .00000634  00000-0  19412-4 0  9992
2 25544  51.6435 125.7943 0001445 337.3184 171.9893 15.49378091227714

NOAA 15                 
1 25338U 98030A   20124.94794743  .00000070  00000-0  48035-4 0  9998
2 25338  98.7208 150.2168 0011008  38.1025 322.0931 14.25962243142633

NOAA 18                 
1 28654U 05018A   20124.89114038  .00000074  00000-0  64498-4 0  9998
2 28654  99.0478 180.8944 0014042   1.3321 358.7885 14.12507537770620

NOAA 19                 
1 33591U 09005A   20125.24606893  .00000052  00000-0  53375-4 0  9997
2 33591  99.1966 129.4427 0013696 195.7711 164.3035 14.12406155579156

METEOR-M 2              
1 40069U 14037A   20125.52516225 -.00000051  00000-0 -42286-5 0  9997
2 40069  98.5072 164.7117 0006720  56.5344 303.6479 14.20671878301977 
*/


// Set your Callsign and current location details
const char  *pcMyName = "S21TO";     // Observer name
double       dMyLAT   = +23.7106;    // Latitude (Breitengrad): N -> +, S -> -
double       dMyLON   = +90.3978;    // Longitude (Längengrad): E -> +, W -> -
double       dMyALT   = 12.0;        // Altitude ASL (m)




int rangePin = 7;   // LED for in Range Indication
int NrangePin = 6;  // LED pin for Out of range indication


int epos = 0; 
int apos = 0;

double       dSatLAT  = 0;           // Satellite latitude
double       dSatLON  = 0;           // Satellite longitude
double       dSatAZ   = 0;           // Satellite azimuth
double       dSatEL   = 0;           // Satellite elevation

char         acBuffer[20];           // Buffer for ASCII time



void setup()
{
  setTime(CurrentHour,CurrentMin,CurrentSec,CurrentDay,CurrentMonth,CurrentYear);

  elevation.attach(9);
  azimuth.attach(10);

  elevation.write(epos);
  azimuth.write(apos);

  pinMode(NrangePin, OUTPUT);
  pinMode(rangePin, OUTPUT);

  Serial.begin(9600);
  delay(10);

  digitalWrite(NrangePin, HIGH);
  digitalWrite(rangePin, HIGH);
  delay(5000);
  }


void loop()
{

  char buf[80]; 

  int i;
  int          iYear    = year();        // Set start year
  int          iMonth   = month();       // Set start month
  int          iDay     = day();         // Set start day
  int          iHour    = hour();        // Set start hour [ substract -6 from current time ]
  int          iMinute  = minute();      // Set start minute
  int          iSecond  = second();      // Set start second



  P13Sun Sun;                                                       // Create object for the sun
  P13DateTime MyTime(iYear, iMonth, iDay, iHour, iMinute, iSecond); // Set start time for the prediction
  P13Observer MyQTH(pcMyName, dMyLAT, dMyLON, dMyALT);              // Set observer coordinates

  P13Satellite MySAT(tleName, tlel1, tlel2);                        // Create ISS data from TLE

  MyTime.ascii(acBuffer);             // Get time for prediction as ASCII string
  MySAT.predict(MyTime);              // Predict ISS for specific time
  MySAT.latlon(dSatLAT, dSatLON);     // Get the rectangular coordinates
  MySAT.elaz(MyQTH, dSatEL, dSatAZ);  // Get azimut and elevation for MyQTH

  Serial.print("Azimuth: ");
  Serial.println(dSatAZ,2);
  Serial.print("Elevation: ");
  Serial.println(dSatEL,2);
  Serial.println("");



  delay(500);


  // Servo calculation
  epos = (int)dSatEL;
  apos = (int)dSatAZ;

  if (epos < 0) {
      digitalWrite(NrangePin, HIGH);
      digitalWrite(rangePin, LOW);
  } else {
      digitalWrite(NrangePin, LOW);
      digitalWrite(rangePin, HIGH);

      if(apos < 180) {
        apos = abs(180 - (apos));
        epos = 180-epos;

      } else {
        apos = (360-apos);
        epos = epos;
      }
      azimuth.write(apos);
      delay(15);   
      elevation.write(epos);        
  }
}
