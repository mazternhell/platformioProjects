/*
 * Tasks.h
 * 
 * FreeRTOS task definitions for dual-core operation.
 * DisplayTask runs on Core 1, SensorTask runs on Core 1.
 */

#ifndef TASKS_H
#define TASKS_H

#include "Globals.h"
#include "Hardware.h"
#include "DisplayUI.h"

// ==================================================
// TASK FUNCTIONS
// ==================================================
void DisplayTask(void *parameter);
void SensorTask(void *parameter);

// ==================================================
// TASK INITIALIZATION
// ==================================================
void startTasks();

#endif // TASKS_H
