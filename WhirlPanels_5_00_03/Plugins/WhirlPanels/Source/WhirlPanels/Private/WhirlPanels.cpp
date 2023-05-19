// Copyright 2023 Pentangle Studio Licensed under the Unlicense License (the «LICENSE»);

#include "WhirlPanels.h"

#define LOCTEXT_NAMESPACE "FWhirlPanelsModule"

void FWhirlPanelsModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FWhirlPanelsModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FWhirlPanelsModule, WhirlPanels)