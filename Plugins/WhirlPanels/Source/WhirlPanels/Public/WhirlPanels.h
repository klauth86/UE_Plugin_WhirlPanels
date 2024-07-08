// Copyright 2023 Pentangle Studio Licensed under the Unlicense License (the «LICENSE»);

#pragma once

#include "Modules/ModuleManager.h"

class FWhirlPanelsModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};