// Copyright Epic Games, Inc. All Rights Reserved.

#include "MetaCsound.h"
#include "MetasoundFrontendRegistries.h"
#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FMetaCsoundModule"

void FMetaCsoundModule::StartupModule()
{
	FMetasoundFrontendRegistryContainer::Get()->RegisterPendingNodes();

	//const FString LibExamplePath = FPaths::Combine(*BasePluginDir, TEXT("Source/ThirdParty/lib_win64_example.dll"));
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

namespace Metasound
{
	namespace StandardNodes
	{
		const FName Namespace = "UE";
		const FName AudioVariant = "Audio";
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FMetaCsoundModule, MetaCsound)

