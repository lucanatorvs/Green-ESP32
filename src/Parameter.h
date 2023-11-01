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

void initializeParameter();
void setParameter(int index, int value);
void getParameter(int index);
void storeParametersToNVS();
void clearNVS();
void updateParametersFromNVS();

#endif // PARAMETER_H
