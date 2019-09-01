/*
   Simple Arduino spectrometer.
   Return all data serial CSV formatted and ready to be saved.
   Can be used to return relative or absolute values.
*/
#include <AS726X.h>

/* SETTINGS */
#define INTERVAL 2000 //measurement interval in ms
#define REL_VALUES true //true returns relative values, false absolute values
#define DRV_LED true //turn on (true) / off (false) the driver LED
#define MEASURE_TEMP true //additionally log sensor temperature

AS726X ams; //create ams library object

void setup()
{
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  //begin and make sure we can talk to the sensor
  if (!ams.begin())
  {
    Serial.println("Error, no sensor!");
    while (1);
  }

  // 0,1,2,3 -> 1,2,4,8 mA
  ams.setIndicatorCurrent(0); //set current for indicator LED 1mA
  ams.enableIndicator(); //turn on indicator LED

  //0,1,2,3 -> 12.5,25,50,100 mA
  ams.setBulbCurrent(0); //set driver bulb current to lowest

  //control the driver led
  if (DRV_LED) {
    ams.enableBulb();
  } else {
    ams.disableBulb();
  }

  // 0,1,2,3 -> VBGY, GYOR, all, One-Shot all
  ams.setMeasurementMode(2); //continuous reading of all channels
  ams.setGain(3); //0,1,2,3 -> 1x,3.7x,16x,64x
  ams.setIntegrationTime(50); //actual integration time will be time*2.8ms

  Serial.begin(57600); //start fast Serial
}

void loop()
{
  String temperature = "";

  if (MEASURE_TEMP)
  {
    uint8_t temp = ams.getTemperature() + 273.15; //get sensor temperature in Kelvin
    temperature = String(temp);
  }

  ams.takeMeasurements(); //takeMeasurementsWithBulb();

  while (!ams.dataAvailable())
  {
    delay(5); //wait till data is ready
  }

  float calibratedValues[6]; //this will hold all (6) the channel data

  calibratedValues[0] = ams.getCalibratedViolet();
  calibratedValues[1] = ams.getCalibratedBlue();
  calibratedValues[2] = ams.getCalibratedGreen();
  calibratedValues[3] = ams.getCalibratedYellow();
  calibratedValues[4] = ams.getCalibratedOrange();
  calibratedValues[5] = ams.getCalibratedRed();

  uint8_t array_size = 6; //sizeof(calibratedValues)/sizeof(float);

  if (REL_VALUES)
  {
    float sum_value = 0;

    for (uint8_t i = 0; i < array_size; i++) //add all the channel values
    {
      sum_value += calibratedValues[i];
    }

    for (uint8_t i = 0; i < array_size; i++) //compute the ratio of every channel
    {
      calibratedValues[i] /= sum_value;
    }
  }

  // VBGYOR
  for (uint8_t i = 0; i < array_size; i++)
  {
    float channel_data = calibratedValues[i];
    Serial.print(String(channel_data) + ",");

    //add comma and blank after every value except the last one
    if (i < array_size - 1 || MEASURE_TEMP)
    {
      Serial.print(",");
    }
  }

  Serial.print(temperature);
  Serial.println();
  delay(INTERVAL);
}
