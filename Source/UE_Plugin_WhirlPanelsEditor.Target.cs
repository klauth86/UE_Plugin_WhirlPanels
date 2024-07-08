// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

public class UE_Plugin_WhirlPanelsEditorTarget : TargetRules
{
	public UE_Plugin_WhirlPanelsEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V4;

		ExtraModuleNames.AddRange( new string[] { "UE_Plugin_WhirlPanels" } );
	}
}
