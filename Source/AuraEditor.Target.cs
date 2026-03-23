// Copyright Li Qian

using UnrealBuildTool;
using System.Collections.Generic;

public class AuraEditorTarget : TargetRules
{
	public AuraEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;

		ExtraModuleNames.AddRange( new string[] { "Aura" } );
		
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_6;
		bDebugBuildsActuallyUseDebugCRT = false;
	}
}
