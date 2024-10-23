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

        // ������ģ������ЩĿ¼���Ա��ⲿ����ģ����Ϊͷ�ļ�·��
        PublicIncludePaths.AddRange(new string[] {
            "DeformMesh/Public",
        });

        // ������ģ������ЩĿ¼���Ա�ģ���ڲ���Ϊͷ�ļ�·�� 
        PrivateIncludePaths.AddRange(new string[] {
            "DeformMesh/Private",
        });
    }
}
