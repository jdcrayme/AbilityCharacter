// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class AbilityCharacter : ModuleRules
{
	public AbilityCharacter(ReadOnlyTargetRules Target) : base(Target)
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
            "Slate",
            "GameplayAbilities",
            "GameplayTags",
            "GameplayTasks",
        });

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"AbilityCharacter",
			"AbilityCharacter/Variant_Platforming",
			"AbilityCharacter/Variant_Platforming/Animation",
			"AbilityCharacter/Variant_Combat",
			"AbilityCharacter/Variant_Combat/AI",
			"AbilityCharacter/Variant_Combat/Animation",
			"AbilityCharacter/Variant_Combat/AbilitySystem",
			"AbilityCharacter/Variant_Combat/Gameplay",
			"AbilityCharacter/Variant_Combat/Interfaces",
			"AbilityCharacter/Variant_Combat/UI",
			"AbilityCharacter/Variant_SideScrolling",
			"AbilityCharacter/Variant_SideScrolling/AI",
			"AbilityCharacter/Variant_SideScrolling/Gameplay",
			"AbilityCharacter/Variant_SideScrolling/Interfaces",
			"AbilityCharacter/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
