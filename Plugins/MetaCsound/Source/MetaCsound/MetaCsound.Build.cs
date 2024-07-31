// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MetaCsound : ModuleRules
{
	public MetaCsound(ReadOnlyTargetRules Target) : base(Target)
	{

		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"MetasoundGraphCore",
				"MetasoundFrontend",
				"MetasoundEditor",
			}
			);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
                "CoreUObject",
                "Engine",
            }
			);

		// TODO: Add Linux and macOS dependencies

        PublicIncludePaths.AddRange(
			new string[]
			{
				"C:/Program Files/Csound6_x64/include/csound/",
			}
			);

        // Include the import library
        PublicAdditionalLibraries.Add("C:/Program Files/Csound6_x64/lib/csound64.lib");

		// Put the library along with the executable
		RuntimeDependencies.Add("C:/Program Files/Csound6_x64/bin/csound64.dll");

		// Load library
		PublicDelayLoadDLLs.Add("csound64.dll");

		bEnableUndefinedIdentifierWarnings = false;
	}
}
