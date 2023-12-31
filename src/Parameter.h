#ifndef PARAMETER_H
#define PARAMETER_H

#include <Arduino.h>

struct Parameter {
    int index;
    String name;
    int defaultValue;
    int value;
};

extern Parameter parameters[];
extern const int numParameters;

// Initialize parameters
void initializeParameter();

// Parameter functions
void setParameter(int index, int value);
void getParameter(int index);
void storeParametersToNVS(int index = -1);
void clearNVS(int index = -1);
void updateParametersFromNVS(int index = -1);

#endif // PARAMETER_H
