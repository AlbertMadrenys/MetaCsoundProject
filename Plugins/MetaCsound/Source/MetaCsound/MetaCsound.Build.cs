// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MetaCsound : ModuleRules
{
	public MetaCsound(ReadOnlyTargetRules Target) : base(Target)
	{
        
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(
            new string[]
            {
                "C:/Program Files/Csound6_x64/include/csound/",
			}
            );


        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "MetasoundFrontend",
                "MetasoundGraphCore",
                "Projects"
            }
            );


        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "MetasoundEngine",
            }
            );

        // Include the import library
        //PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "..", "ThirdParty", "lib_win64_import_example.lib"));
        PublicAdditionalLibraries.Add("C:/Program Files/Csound6_x64/lib/csound64.lib"); 

        // Put the library along with the executable
        //RuntimeDependencies.Add("$(PluginDir)/ThirdParty/lib_win64_example.dll");
        RuntimeDependencies.Add("C:/Program Files/Csound6_x64/bin/csound64.dll"); 
        // WIP Include sndfile.dll

        // Load library
        //PublicDelayLoadDLLs.Add("lib_win64_example.dll");
        PublicDelayLoadDLLs.Add("csound64.dll");

        bEnableUndefinedIdentifierWarnings = false;
    }
}
