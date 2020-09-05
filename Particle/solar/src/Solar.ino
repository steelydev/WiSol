#include <math.h>
#include <SmoothSamples.h>
#include <RunLoop.h>
#include <NVStorage.h>
#include <sstream>

/*

DATE        VERSION  WHO  DESCRIPTION
----------  -------  ---  -----------------
3/13/2020   3.1.0    GWM  -Add shower mode

*/

const String version        = "3.1.0";

const int PANELS_PIN        = A0;
const int STORAGE_PIN       = A1;
const int PUMP_PIN          = D0;
const int PUMP_LED_PIN      = D7;
const int HEATER            = D1;

// defaults
const int ON_DIFFERENCE     = 10;
const int OFF_DIFFERENCE    = 3;
const int MIN_STORAGE_TEMP  = 115;
const int SWING             = 3;
const int MAX_STORAGE_TEMP  = 145;
const String PUBLISH_DEBUG  = "Solar-debug-";

// setup timer for shower mode
RunLoop timerRunLoop;
const time_t INTERVAL_showerMode   = RlTimer::makeInterval(0,20,0,0); // ss,mm,hh,dd  20 minutes
RlTimer showerModeTimer("showerMode", showerMode, INTERVAL_showerMode, 1);

//* ABC for Murata NXRT15XH103FA1B030
const String shA = "0.8877708660";
const String shB = "2.511722394";
const String shC = "1.956489284";
// */

/* ABC for TDK B57164K103J
const String shA = "1.294101041";
const String shB = "2.161266819";
const String shC = "0.8871575401";
// */

typedef enum {
  OFF = 0,
  AUTO,
  ON,
  SHOWER
 } Mode;

int debug = 10;
String s;
bool pumpOn = false; // pump
bool heatOn = false; // aux heater
String fullStatus;

SmoothSamples panels(5,1,5);
SmoothSamples storage(5,1,5);

// Non-volatile store
const char INITIALIZE_FLAG[10] = "Solar";
NVStorage nv;

SYSTEM_MODE(SEMI_AUTOMATIC);
// ===========================================================================

void setup()
{
  System.set(SYSTEM_CONFIG_SOFTAP_PREFIX, "WiSol");
  Serial.begin(115200);
  blink(D7,40);

 /*
 while (true) {
      pumpOff();
      delay(200);
      pumpOn();
      delay(100);
  // */

  turnPumpOff();
  turnHeatOff();

  pinMode(PANELS_PIN, AN_INPUT);
  pinMode(STORAGE_PIN, AN_INPUT);

  Particle.connect();

  Particle.function("setDebug",       setDebug);
  Particle.function("setDiff",        setDiff);
  Particle.function("setPumpMode",    setPumpMode);
  Particle.function("setSH",          setSH);
  Particle.function("setHeatMode",    setHeatMode);
  Particle.function("setHeat",        setHeat);

  Particle.variable("pumpOn",         pumpOn);
  Particle.variable("heatOn",         heatOn);
  Particle.variable("fullStatus",     fullStatus);

  showerModeTimer.invalidate();
  nv.get();
  buildFullStatus();
  WF_log(fullStatus,0);

  if (strcmp(nv._store.initializeFlag,INITIALIZE_FLAG) != 0) {  // Initialize NV storage
    strcpy(nv._store.initializeFlag,INITIALIZE_FLAG);
    nv._store.pumpMode = AUTO;
    nv._store.onDifference = ON_DIFFERENCE;
    nv._store.offDifference = OFF_DIFFERENCE;
    shA.toCharArray(nv._store.A,16);
    shB.toCharArray(nv._store.B,16);
    shC.toCharArray(nv._store.C,16);
    nv._store.heatMode = AUTO;
    nv._store.minStorageTemp = MIN_STORAGE_TEMP;
    nv._store.storageSwing = SWING;
    nv.save();
    WF_log("nv.save()",0);
  }

  buildFullStatus();
  WF_log(fullStatus,0);

  delay(1000);

  Particle.publish("Solar-startup","{\"Particle\":\""+System.version()+"\", \"App\":\""+version+"\"}");
  Particle.publish("Solar-startup",fullStatus);

} // ===========================================================================

void loop()
{

  int rawADC;
  float temperature;

  rawADC=analogRead(PANELS_PIN);
  WF_log("rawADC="+String(rawADC)+" on pin="+String(PANELS_PIN),0);
  temperature=Steinhart_Hart(rawADC,"F");
  WF_log("temperature="+String(temperature),0);
  panels.addSample(static_cast<int>(temperature));
  WF_log("panels="+String(panels.get()),0);

  rawADC=analogRead(STORAGE_PIN);
  WF_log("rawADC="+String(rawADC)+" on pin="+String(STORAGE_PIN),0);
  temperature=Steinhart_Hart(rawADC,"F");
  WF_log("temperature="+String(temperature),0);
  storage.addSample(static_cast<int>(temperature));
  WF_log("storage="+String(storage.get()),0);

  if (nv._store.pumpMode == OFF || showerModeTimer.isValid()) {
    turnPumpOff();
  } else if (nv._store.pumpMode == AUTO) {
    if (pumpOn) {
      if (panels.get() - storage.get() <= nv._store.offDifference) {
        turnPumpOff();
      }
    } else if (panels.get() - storage.get() >= nv._store.onDifference) {
        turnPumpOn();
    }
  } else if (nv._store.pumpMode == ON) {
    turnPumpOn();
  }

  if (nv._store.heatMode == OFF) {
    turnHeatOff();
  } else if (nv._store.heatMode == AUTO) {
    if (heatOn) {
      if (storage.get() >= nv._store.minStorageTemp + nv._store.storageSwing) {
        turnHeatOff();
      }
    } else if (storage.get() < nv._store.minStorageTemp) {
        if (!pumpOn) { // do not allow aux heat to run if solar pump is on
          turnHeatOn();
        }
    }
  } else if (nv._store.heatMode == ON) {
    turnHeatOn();
  }

  // heat fail-safe
  if (storage.get() > MAX_STORAGE_TEMP) {
    turnHeatOff();
  }

  buildFullStatus();
  WF_log(fullStatus,0);
  timerRunLoop.processTimers();
  delay(5000);

} // ===========================================================================

void buildFullStatus()
{
  
  String mode = String(nv._store.pumpMode);

  if (showerModeTimer.isValid()) { mode = String(SHOWER); }

  fullStatus = "{\"version\":\""+version+"\",\"pumpMode\":"+ mode +",\"pumpOn\":"+String(pumpOn)+",\"panels\":"+String(panels.get())+",\"storage\":"+String(storage.get());
  fullStatus += ",\"onDiff\":"+String(nv._store.onDifference)+",\"offDiff\":"+String(nv._store.offDifference);
  fullStatus += ",\"A\":"+String(nv._store.A)+",\"B\":"+String(nv._store.B)+",\"C\":"+String(nv._store.C);
  fullStatus += ",\"heatMode\":"+String(nv._store.heatMode)+",\"heatOn\":"+String(heatOn)+",\"minStorageTemp\":"+String(nv._store.minStorageTemp)+",\"storageSwing\":"+String(nv._store.storageSwing);
  fullStatus += "}";

  WF_log("{\"method\":\"buildFullStatus()\", \"fullStatus\":\"" + fullStatus + "\"}",0);

} // ===========================================================================

void publishFullStatus()
{
    buildFullStatus();
    Particle.publish("Solar-fullStatus",fullStatus);

} // ===========================================================================

void turnPumpOn()
{
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(PUMP_LED_PIN, OUTPUT);

  digitalWrite(PUMP_PIN, HIGH);
  digitalWrite(PUMP_LED_PIN, HIGH);

  pumpOn = true;

} // ===========================================================================

void turnPumpOff()
{
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(PUMP_LED_PIN, OUTPUT);

  digitalWrite(PUMP_PIN, LOW);
  digitalWrite(PUMP_LED_PIN, LOW);

  pumpOn = false;

} // ===========================================================================

void turnHeatOn()
{
  pinMode(HEATER, OUTPUT);

  digitalWrite(HEATER, HIGH);

  heatOn = true;

} // ===========================================================================

void turnHeatOff()
{
  pinMode(HEATER, OUTPUT);

  digitalWrite(HEATER, LOW);

  heatOn = false;

} // ===========================================================================

double Steinhart_Hart(int rawADC, String scale)
{

	//char c[50];

	double A = atof(nv._store.A);
	A /= 1000.0;
	double B = atof(nv._store.B);
	B /= 10000.0;
	double C = atof(nv._store.C);
	C /= 10000000.0;

  double R;
  double tempK,tempC,tempR,tempF;

  // 0..4095 is range of ADC input; 10,000 is Resistance of thermistor at 25C
  R = log((((4095.0 * 10000.0)/rawADC) - 10000.0));
  tempK = 1.0 / (A + B*R + C*R*R*R);
  tempC = tempK - 273.15;               // Kelvin to Celsius
  tempF = (tempC * 9.0)/ 5.0 + 32.0;    // Celsius to Fahrenheit
  tempR = tempF + 459.67;               // Fahrenheit to Rankine

  if (scale == "C") return tempC;
  if (scale == "F") return tempF;
  if (scale == "K") return tempK;
  if (scale == "R") return tempR;

  return tempF;

} // ===========================================================================

void blink(int pin, int times) {
    pinMode(pin, OUTPUT);

    for (int i=0;i<times;i++){
      delay(50);
      digitalWrite(pin,HIGH);
      delay(50);
      digitalWrite(pin,LOW);
    }

} // ===========================================================================

int setPumpMode(String modeString) {
  // modeString is JSON {MODE: string}
  // where string = Off | Auto | On

  int newMode;
  String s = valueForKey("MODE",modeString);
  WF_log("{\"MODE\":"+s+", \"modeString\":"+modeString+"}",0);

  if (showerModeTimer.isValid()) {
     WF_log("{\"MESSAGE\":\"In shower timer. setPumpMode ignored.\"}",0);
    return 0;
  }

  if (s == "Off") {
    newMode = OFF;
  }
  else if (s == "Auto") {
    newMode = AUTO;
  }
  else if (s == "On") {
    newMode = ON;
  }
 else if (s == "Shower") {
   if (nv._store.pumpMode == AUTO || nv._store.pumpMode == ON) {
      showerModeTimer.reset(INTERVAL_showerMode,1);
      timerRunLoop.addTimer(&showerModeTimer,FIRE_WHEN_TIME);
   } else {
     newMode = nv._store.pumpMode;
   }
  }
  else
    return 1; // bad modeString

  if (newMode != nv._store.pumpMode) {
    nv._store.pumpMode = newMode;
    nv.save();
  }

  WF_log("{\"mode\":"+String(nv._store.pumpMode)+"}",0);
  buildFullStatus();
  return 0;

} // ===========================================================================

void showerMode() { // not really needed, just have to have a callback for showModeTimer
  showerModeTimer.invalidate();
  
} // ===========================================================================

int setHeatMode(String modeString) {
  // modeString is JSON {MODE: string}
  // where string = Off | Auto | On

  int newMode;
  String s = valueForKey("MODE",modeString);
  WF_log("{\"MODE\":"+s+", \"modeString\":"+modeString+"}",0);


  if (s == "Off") {
    newMode = OFF;
  }
  else if (s == "Auto") {
    newMode = AUTO;
  }
  else if (s == "On") {
    newMode = ON;
  }
  else
    return 1; // bad modeString

  if (newMode == nv._store.heatMode) return 0;  // no change

  nv._store.heatMode = newMode;
  nv.save();

  WF_log("{\"hmode\":"+String(nv._store.heatMode)+"}",0);
  buildFullStatus();
  return 0;

} // ===========================================================================

int setHeat(String param) {
  // param is JSON {MIN_STORAGE_TEMP: int, STORAGE_SWING: int }

  int newMode;
  String min = valueForKey("MIN_STORAGE_TEMP",param);
  WF_log("{\"MMIN_STORAGE_TEMP\":"+s+", \"param\":"+param+"}",0);
  if (min == "") return 1;

  String swing = valueForKey("STORAGE_SWING",param);
  WF_log("{\"STORAGE_SWING\":"+s+", \"param\":"+param+"}",0);
  if (swing == "") return 1;

  if (min.toInt() == nv._store.minStorageTemp && swing.toInt() == nv._store.storageSwing) return 0;  // no change

  nv._store.minStorageTemp = min.toInt();
  nv._store.storageSwing = swing.toInt();
  nv.save();

  buildFullStatus();
  return 0;

} // ===========================================================================

int setDiff(String differenceString) {
  // param is JSON {ON_DIFFERENCE: int, OFF_DIFFERENCE: int }

  int newDiff;
  bool save = false;

  String s = valueForKey("ON_DIFFERENCE",differenceString);
  if (s != "") {
    newDiff = s.toInt();
    if (newDiff != nv._store.onDifference) {
      nv._store.onDifference = newDiff;
      save = true;
    }
  }

  s = valueForKey("OFF_DIFFERENCE", differenceString);
  if (s != "") {
    newDiff = s.toInt();
    if (newDiff != nv._store.offDifference) {
      nv._store.offDifference = newDiff;
      save = true;
    }
  }

  if (save) nv.save();

  WF_log("{\"ON_DIFFERENCE\":"+String(nv._store.onDifference)+", \"OFF_DIFFERENCE\":"+String(nv._store.offDifference)+"}",0);

  buildFullStatus();
  return 0;

} // ===========================================================================

int setSH(String calibrationString) {
  // calibrationString is JSON {A: string, B: string, C: string}

  bool save = false;
  WF_log("{\"setSH\":"+calibrationString+"}",0);

    String s = valueForKey("A",calibrationString);
    if (s != "") {
      if (s != String(nv._store.A)) {
				s.toCharArray(nv._store.A,16);
        save = true;
      }
    }

    s = valueForKey("B",calibrationString);
    if (s != "") {
      if (s != String(nv._store.B)) {
				s.toCharArray(nv._store.B,16);
        save = true;
      }
    }

    s = valueForKey("C",calibrationString);
    if (s != "") {
      if (s != String(nv._store.C)) {
				s.toCharArray(nv._store.C,16);
        save = true;
      }
    }

	if (save) nv.save();

  WF_log("{\"A\":"+String(nv._store.A)+", \"B\":"+String(nv._store.B)+", \"C\":"+String(nv._store.C)+"}",0);

  buildFullStatus();
  return 0;

} // ===========================================================================

String valueForKey(String key, String dictionary) {
// returns value for key from dictionary in this format
  // {key : value, key: value, key :value }

  int e0,e1,i,j,k;
  String s=dictionary,value;

  // get rid of quotes and backslashes
  s.replace("\"","");
  s.replace("\\","");
  //s.replace("}",","); // makes finding end of last value easier

  // Find the key
  i = s.indexOf(key);

  if (i==-1) {
    WF_log("valueForKey() key=" + key + " not found in ->" + dictionary,0);
    return "";
  }

  // Find the end of key colon
  j = s.indexOf(":", i+1);

  if (j==-1) {
    WF_log("valueForKey() key=" + key + "malformed dictionary missing ':' ->" + dictionary,0);
   return "";
  }

  // see if there is an embedded dictionary
  e0 = s.indexOf("{",j+1);
  if (e0 != -1) {
    // have an embedded dict, find its end
    e1 = s.indexOf("}",e0+1) +1;
  }

  // find the comma after value
  k = s.indexOf(",",j+1);

  if (e0 > -1 && e0 < k) { // embedded dict before end of value, so return embedded dict
    k = e1;
  } else {
    if (k==-1) {
      // may be last value so look for ending }
      k = s.indexOf("}",j+1);

      if (k==-1) {
        WF_log("valueForKey() key=" + key + " malformed dictionary missing ',' or '}' after value ->" + dictionary,0);
        return "";
      }
    }
  }

  value = s.substring(j+1,k);
  value.trim();

  WF_log("valueForKey() key="+ key + ", value=" + value,0);

  return value;

  } // ===========================================================================

  int setDebug(String info) {
    WF_log("setDebug()",0);
    debug = info.toInt();

    WF_log("setDebug() debug="+String(debug),0);
    return 0;
    
  } // ===========================================================================

  void WF_log(String message, int level) {  // if message level <= debug level then output

    int show = debug % 10;

    if (level > show) {
      return;
    }

    if (debug >= 10) { // print message to serial
      Serial.println(message);
    }

    if (debug >= 20) { // publish message
      String data = "{message:" + message + "}";
      s = PUBLISH_DEBUG + System.deviceID();
      Particle.publish(s,data,300,PRIVATE);
      delay(375); // this is so there will be no more than 3 publish per second.
    }

  } // ===========================================================================
