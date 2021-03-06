// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Cs193u : ModuleRules
{
	public Cs193u(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "AIModule", "GameplayTasks", "UMG", "GameplayTags", "OnlineSubsystem", "HeadMountedDisplay"});

		PrivateDependencyModuleNames.AddRange(new string[] { });
	}
}
