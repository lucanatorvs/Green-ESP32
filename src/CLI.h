#ifndef CLI_H
#define CLI_H

#include <Arduino.h>

void initializeCLI();
void cliTask(void * parameter);
void handleInput(String input);
void handleParameterCommand(String input);

#endif // CLI_H
