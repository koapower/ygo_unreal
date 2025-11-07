// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class YGO_unreal : ModuleRules
{
	public YGO_unreal(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"UMG",
			"Slate",
			"SlateCore"
		});

		PrivateDependencyModuleNames.AddRange(new string[] {  });

		// YGO Core 整合需要的模組
		// 目前暫時不加入 Lua 和 ocgcore,先測試基本框架
		// TODO: 未來加入 Lua 時取消註解以下行
		// PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "ThirdParty/lua"));
		// PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "ThirdParty/ocgcore"));

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
