////////////////////////////////////////////////////////////////////
// Implementation of the FMetaCsoundModule class
// 
// Copyright (C) 2024 Albert Madrenys
//
// This software is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3.0 of the License, or (at your option) any later version.
//
/////////////////////////////////////////////////////////////////////

#include "MetaCsound.h"
#include "MetasoundFrontendRegistries.h"

void FMetaCsoundModule::StartupModule()
{
	FMetasoundFrontendRegistryContainer::Get()->RegisterPendingNodes();

	const FString LibExamplePath = TEXT("C:/Program Files/Csound6_x64/bin/csound64.dll");

	DynamicLibExampleHandle = FPlatformProcess::GetDllHandle(*LibExamplePath);
	
	if (DynamicLibExampleHandle != nullptr)
	{
		UE_LOG(LogTemp, Log, TEXT("csound64.dll loaded successfully!"));
	}
	else
	{
		UE_LOG(LogTemp, Fatal, TEXT("csound64.dll failed to load!"));
	}
	
}

void FMetaCsoundModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}
	
IMPLEMENT_MODULE(FMetaCsoundModule, MetaCsound)


