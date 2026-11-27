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
                        "SlateCore",
                        "GameplayAbilities",
                        "GameplayTags",
                        "GameplayTasks",
                });

                PrivateDependencyModuleNames.AddRange(new string[] { });

                // the module root is the only include path: all project includes are
                // module-relative (e.g. "Features/Combat/AbilitySystem/MeleeAbility.h")
                PublicIncludePaths.AddRange(new string[] {
                        "AbilityCharacter",
                });

                // Uncomment if you are using Slate UI
                // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

                // Uncomment if you are using online features
                // PrivateDependencyModuleNames.Add("OnlineSubsystem");

                // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
        }
}
