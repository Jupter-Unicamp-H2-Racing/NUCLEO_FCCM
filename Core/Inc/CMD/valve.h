#ifndef VALVE_H
#define VALVE_H

#include "context.h"

void SupplyValve_CMD(Valve_Status open);
void PurgeValve_CMD(Valve_Status open);
void Fan_CMD(int value);

#endif
