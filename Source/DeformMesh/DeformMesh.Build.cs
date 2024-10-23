// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class DeformMesh : ModuleRules
{
    public DeformMesh(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        //Whether to add all the default include paths to the module (eg. the Source/Classes folder, subfolders under Source/Public).
        // bAddDefaultIncludePaths = true;

        PublicDependencyModuleNames.AddRange(new string[] { });

        PrivateDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine" });

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true

        // 声明本模块中哪些目录可以被外部其他模块作为头文件路径
        PublicIncludePaths.AddRange(new string[] {
            "DeformMesh/Public",
        });

        // 声明本模块中哪些目录可以被模块内部作为头文件路径 
        PrivateIncludePaths.AddRange(new string[] {
            "DeformMesh/Private",
        });
    }
}
