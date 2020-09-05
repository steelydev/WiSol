#include "SmoothSamples.h"
#include "application.h"
#include <math.h>

/*
SmoothSamples::SmoothSamples() :
  _maxSamples(MAX_SAMPLES), _currentSample(0), _secondsBetweenSamples(1), _degreesPerSecond(1)
{
} // ----------------------------- */

/*
SmoothSamples::SmoothSamples(int secondsBetweenSamples, double degreesPerSecond, int samples) :
  SmoothSamples()
{
    _maxSamples = samples;
    _secondsBetweenSamples = secondsBetweenSamples;
    _degreesPerSecond = degreesPerSecond;
} // ----------------------------- */

int SmoothSamples::samples() {
  return _currentSample;
} // -----------------------------

int SmoothSamples::lastSample() {
  if (_currentSample==0) return 0;
  return _sampleArray[_currentSample-1];
} // -----------------------------

void SmoothSamples::addSample(int sample) {
  // adding a good sample to base, so reset bad samples
  _badSamples = 0;

  int fixedSample = sample;
  double allowedDelta = static_cast<double>(_secondsBetweenSamples * _degreesPerSecond);
  int delta = sample - this->lastSample();

  //Serial.print("SmoothSamples::addSample sample=");Serial.println(sample);
  //Serial.print("SmoothSamples::addSample _secondsBetweenSamples=");Serial.println(_secondsBetweenSamples);
  //Serial.print("SmoothSamples::addSample _degreesPerSecond=");Serial.println(_degreesPerSecond);
  //Serial.print("SmoothSamples::addSample allowedDelta=");Serial.println(allowedDelta);

  if (this->samples()>0) {
    // fix sample to degrees per second change
    if (abs(static_cast<double>(delta)) > allowedDelta) { // change too fast, limit it
      // Serial.print("SmoothSamples::addSample delta exceeded: "); Serial.println(delta);
      fixedSample = (delta > 0) ? this->lastSample() + static_cast<int>(allowedDelta) : this->lastSample() - static_cast<int>(allowedDelta);
    }
  }
  //Serial.print("SmoothSamples::addSample fixedSample=");Serial.println(fixedSample);

  /*Serial.print("SmoothSamples::addSample1 array=[");
  for (int i=0;i<this->samples();i++) {
    if (i!=0) Serial.print(",");
    Serial.print(_sampleArray[i]);
  }
  Serial.println("]");
  */

  //Serial.print("SmoothSamples::addSample _currentSample=");Serial.println(_currentSample);
  //Serial.print("SmoothSamples::addSample _maxSamples=");Serial.println(_maxSamples);

  // array is full, ripple all samples down
  if (_currentSample >= _maxSamples)  {
    for (int i=1; i < _maxSamples; i++) {
      _sampleArray[i-1] = _sampleArray[i];
    }
    _sampleArray[_maxSamples-1] = fixedSample;
  }  else {
    _sampleArray[_currentSample] = fixedSample;
    _currentSample++;
  }

} // -----------------------------

void SmoothSamples::addBad()
{ // increment number of bad samples
  _badSamples++;
} // -----------------------------

int SmoothSamples::get()
{ // return the current smoothed temperature
  return static_cast<int>(this->mean()+0.5);
} // -----------------------------

bool SmoothSamples::good()
{ // If we haven't had n bad samples in a row, return good
  if (_badSamples < BAD_SAMPLES)
    return true;
  else
    return false;
} // -----------------------------

double SmoothSamples::mean()
{ // return mean of sample base
  double total = 0;

  if (_currentSample == 0) return 0.0;

  for (int i=0; i < _currentSample; i++) {
    total += (double)_sampleArray[i];
  }

  return total / _currentSample;
} // -----------------------------

double SmoothSamples::median()
{ // return median of sample base
  int sortedArray[_currentSample];
  bool swapped = true;

  if (this->samples() == 0) return 0.0;
  if (this->samples() == 1) return (double)_sampleArray[0];

  // copy array
  for (int i=0; i < _currentSample; i++) {
    sortedArray[i] = _sampleArray[i];
  }

  while(swapped) {
    swapped = false;

    for (int i=1; i<this->samples(); i++) {
      if (sortedArray[i-1] > sortedArray[i]) {
        int j = sortedArray[i];
        sortedArray[i] = sortedArray[i-1];
        sortedArray[i-1] = j;
        swapped = true;
      }
    }
  }

  int middle = this->samples() / 2;

  if (this->samples()%2 == 0) {
    return (_sampleArray[middle-1] + _sampleArray[middle]) / 2;
  } else
    return (double)_sampleArray[middle];
} // -----------------------------

double SmoothSamples::variance()
{
  double mean = this->mean();
  double difference = 0.0, total = 0.0;
  if (this->samples()==0) return 0.0;

  for (int i=0; i < this->samples();i++) {
    difference = _sampleArray[i]-mean;
    difference = difference * difference;

    //Serial.print("SmoothSamples.variance() difference=");Serial.println(difference);
    total += difference;
    //Serial.print("SmoothSamples.variance() sqrOfDifference=");Serial.println(sqrOfDifference);
  }

  return total / this->samples();
} // -----------------------------

double SmoothSamples::standardDeviation()
{
  return sqrt(this->variance());
} // -----------------------------

String SmoothSamples::print()
{
  String st_temps = "[";

  for (int i=0;i<this->samples();i++) {
    if (i!=0) st_temps += ",";
    st_temps += String(_sampleArray[i]);
  }
  st_temps += "]";

  return st_temps;
} // -----------------------------
