#ifndef CLI_H
#define CLI_H

#include <Arduino.h>

// Initialize CLI
void initializeCLI();

// Process input for CLI
void handleInput(String input, Stream &stream);

#endif // CLI_H
