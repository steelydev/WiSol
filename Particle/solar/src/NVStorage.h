// Changelog
// Date          Version    Who    Description
// ==========    =======    ===    =============================
// 2015-09-13    1.0        GWM    -Initial implementation
//
// 2016-05-16    1.1        GWM    -Change to store and recall a struct at address 0 so no
//                                  address calcs are needed.
//
//

#ifndef NVStorage_h
#define NVStorage_h
#include "application.h"


class NVStorage
{

public:
  struct Store {
    int  pumpMode;
    int  onDifference;
    int  offDifference;
    char A[16];
    char B[16];
    char C[16];
    int  heatMode;
    int  minStorageTemp;
    int  storageSwing;
    char initializeFlag[10];
  } _store;

  NVStorage();
  void save();
  void get();
};
#endif
