// MyGameMode.cpp
#include "MyGameMode.h"
#include "MyFPSHUD.h"

AMyGameMode::AMyGameMode()
{
    HUDClass = AMyFPSHUD::StaticClass();
}