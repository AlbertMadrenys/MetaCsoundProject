/////////////////////////////////////////////////////////////////////
// MetaCsound.build: MetaCsound module rules for UBT
// 
// Copyright (C) 2024 Albert Madrenys
//
// This software is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3.0 of the License, or (at your option) any later version.
//
/////////////////////////////////////////////////////////////////////

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

		// Unreal can't deploy dlls outside the plugin folder or in Unreal folder.
		// Csound6 will have to be installed manually
		// TODO: Deploy Csound when packaging game?
		//RuntimeDependencies.Add("C:/Program Files/Csound6_x64/bin/csound64.dll");

		// Load library
		PublicDelayLoadDLLs.Add("csound64.dll");

		bEnableUndefinedIdentifierWarnings = false;
	}
}
