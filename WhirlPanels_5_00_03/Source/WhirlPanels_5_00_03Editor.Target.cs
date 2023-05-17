// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

public class WhirlPanels_5_00_03EditorTarget : TargetRules
{
	public WhirlPanels_5_00_03EditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V2;

		ExtraModuleNames.AddRange( new string[] { "WhirlPanels_5_00_03" } );
	}
}
