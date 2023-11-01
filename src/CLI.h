#ifndef CLI_H
#define CLI_H

#include <Arduino.h>

// Initialize CLI
void initializeCLI();

// CLI task function
void cliTask(void * parameter);

// Handle input
void handleInput(String input);

// CLI commands
void handleParameterCommand(String input);

#endif // CLI_H
