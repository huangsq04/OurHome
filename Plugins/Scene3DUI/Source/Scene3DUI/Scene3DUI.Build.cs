// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;
using Tools.DotNETCommon;

public class Scene3DUI : ModuleRules
{
	public Scene3DUI(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        BuildVersion version = EngineVersion;


        PublicIncludePaths.AddRange(
			new string[] {

				// ... add public include paths required here ...
			}
			);

        var EngineDir = Path.GetFullPath(Target.RelativeEnginePath);
        PrivateIncludePaths.AddRange(
			new string[] {
				"Scene3DUI/Private",
                "Scene3DUI/Public",
                 Path.Combine(EngineDir, @"Source\Runtime\Renderer\Private"),
                 Path.Combine(EngineDir, @"Source\Runtime\Launch"),
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
                "RHI",
                "Renderer",
                "RenderCore",
                "InputCore",
                "UMG",
                "LevelSequence",
                "Projects",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
                "RHI",
                "Renderer",
                "RenderCore",
				// ... add private dependencies that you statically link with here ...	
			}
			);

        if (version.MinorVersion < 22)
        {
            PublicDependencyModuleNames.AddRange(new string[] { "ShaderCore", });
            PrivateDependencyModuleNames.AddRange(new string[] { "ShaderCore", });
        }

		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}

    public BuildVersion EngineVersion
    {
        get
        {
            FileReference File = BuildVersion.GetDefaultFileName();
            BuildVersion Version;
            if (!BuildVersion.TryRead(File, out Version))
            {
                throw new BuildException("Unable to read version file ({0}). Check that this file is present and well-formed JSON.", File);
            }
            return Version;
        }
    }
}
