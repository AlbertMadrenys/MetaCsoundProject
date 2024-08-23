/////////////////////////////////////////////////////////////////////
// FMetaCsoundModule: Implementation of IModuleInterface
// for the MetaCsound module.
// 
// Copyright (C) 2024 Albert Madrenys
//
// This software is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3.0 of the License, or (at your option) any later version.
//
/////////////////////////////////////////////////////////////////////
#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FMetaCsoundModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	void* DynamicLibExampleHandle;
};
