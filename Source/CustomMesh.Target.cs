// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

public class CustomMeshTarget : TargetRules
{
	public CustomMeshTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V2;

		ExtraModuleNames.AddRange( new string[] { "CustomMesh", "DeformMesh", "CustomShadersDeclarations" } );
	}
}
