#include "ColyseusModule.h"

#define LOCTEXT_NAMESPACE "FColyseusClientModule"

class FColyseusClientModule : public IColyseusClientModule
{
};

IMPLEMENT_MODULE(FColyseusClientModule, ColyseusClient)
