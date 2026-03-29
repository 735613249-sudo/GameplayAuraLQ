// Copyright Li Qian

using UnrealBuildTool;
using System.Collections.Generic;

public class AuraTarget : TargetRules
{
	public AuraTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;

		ExtraModuleNames.AddRange( new string[] { "Aura" } );
        
		// ========== 编译加速核心代码 ==========
		bUseUnityBuild = true;        // 开启Unity编译（合并CPP文件）
		bForceUnityBuild = true;      // 强制开启，无视小文件
		bBuildEditor = true;
		bCompileISPC = false;         // 禁用无用编译
		bUsePCHFiles = true;          // 启用预编译头
	}
}
