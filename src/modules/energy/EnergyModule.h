#pragma once
#include <Arduino.h>
#include <time.h>

// ---------------------------------------------------------
// SYSTEM ENERGY (namespace: "energy_sys")
// ---------------------------------------------------------
void   EnergySys_begin();
void   EnergySys_update();
void   EnergySys_addmAh(float mAh);

float  EnergySys_getTodaymAh();
float  EnergySys_getAvgDailymAh();
float  EnergySys_getMonthlymAh();
String EnergySys_getLastResetStr();
time_t EnergySys_getLastResetTimestamp();
float* EnergySys_getHistoryArray();

int    EnergySys_getPeakCurrentmA();
int    EnergySys_getAvgCurrentmA();

void   EnergySys_resetDaily();
void   EnergySys_resetMonthly();
void   EnergySys_forceSave();
void   EnergySys_forceLoad();

// ---------------------------------------------------------
// MOTOR ENERGY (namespace: "energy_motor")
// ---------------------------------------------------------
void   EnergyMotor_begin();
void   EnergyMotor_update();
void   EnergyMotor_addmAh(float mAh);

float  EnergyMotor_getTodaymAh();
float  EnergyMotor_getAvgDailymAh();
float  EnergyMotor_getMonthlymAh();
String EnergyMotor_getLastResetStr();
time_t EnergyMotor_getLastResetTimestamp();
float* EnergyMotor_getHistoryArray();

int    EnergyMotor_getPeakCurrentmA();
int    EnergyMotor_getAvgCurrentmA();

void   EnergyMotor_resetDaily();
void   EnergyMotor_resetMonthly();
void   EnergyMotor_forceSave();
void   EnergyMotor_forceLoad();

// ---------------------------------------------------------
// Combined initialization
// ---------------------------------------------------------
void Energy_begin();
void Energy_update();
