// Changelog
// Date          Version    Who    Description
// ==========    =======    ===    =============================
// 2015-08-26    1.0        GWM    -Initial implementation
// 2015-09-08    1.1        GWM    -Change to keep track of bad samples
//
//

#ifndef SmoothSamples_h
#define SmoothSamples_h
#include "application.h"

class SmoothSamples
{
  private:
    static const int MAX_SAMPLES {10};
    static const int BAD_SAMPLES {3};

    int _maxSamples {10};
    int *_sampleArray;
    int _currentSample {0};
    int _secondsBetweenSamples {1};
    double _degreesPerSecond {1};
    int _badSamples {0};

  public:
    // Init
    SmoothSamples() :
      _maxSamples {MAX_SAMPLES}, _currentSample {0}, _secondsBetweenSamples {1}, _degreesPerSecond {1.0}
    {
      _sampleArray = new int[_maxSamples];
    }
    SmoothSamples(int secondsBetweenSamples, double degreesPerSecond, int samples) :
      _secondsBetweenSamples {secondsBetweenSamples}, _degreesPerSecond {degreesPerSecond}, _maxSamples {samples}
    {
      _sampleArray = new int[_maxSamples];
    }
    ~SmoothSamples() {
      delete[] _sampleArray;
    }

    void addSample(int); // Add a raw sample to our sample base
    void addBad(); // Indicate a bad sample
    int samples();  // Number of samples in base
    int lastSample(); // Last sample added to base

    int get(); // Return a smoothed sample
    bool good(); // True if not too many bad samples in a row

    // Stats
    double mean();
    double median();
    double variance();
    double standardDeviation();

    // Output
    String print();
};
#endif
