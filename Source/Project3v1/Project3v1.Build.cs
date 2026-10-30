// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Project3v1 : ModuleRules
{
	public Project3v1(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"Project3v1",
			"Project3v1/Variant_Platforming",
			"Project3v1/Variant_Platforming/Animation",
			"Project3v1/Variant_Combat",
			"Project3v1/Variant_Combat/AI",
			"Project3v1/Variant_Combat/Animation",
			"Project3v1/Variant_Combat/Gameplay",
			"Project3v1/Variant_Combat/Interfaces",
			"Project3v1/Variant_Combat/UI",
			"Project3v1/Variant_SideScrolling",
			"Project3v1/Variant_SideScrolling/AI",
			"Project3v1/Variant_SideScrolling/Gameplay",
			"Project3v1/Variant_SideScrolling/Interfaces",
			"Project3v1/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
