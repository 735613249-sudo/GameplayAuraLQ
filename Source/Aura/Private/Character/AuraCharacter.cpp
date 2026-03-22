// Copyright Li Qian


#include "Character/AuraCharacter.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Player/AuraPlayerController.h"
#include "Player/AuraPlayerState.h"
#include "UI/HUD/AuraHUD.h"

AAuraCharacter::AAuraCharacter()		// 实现构造函数，初始化角色移动和旋转行为
{
	// 1. 移动组件配置：GetCharacterMovement()返回ACharacter的核心移动组件（CharacterMovementComponent）
	GetCharacterMovement()->bOrientRotationToMovement = true;			// 角色旋转自动朝向移动方向（而非固定朝向控制器视角）
	// 旋转速率：仅Yaw（偏航，左右转）为400度/秒，Pitch（俯仰）、Roll（翻滚）为0（禁止上下/翻滚旋转）
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 400.0f, 0.0f);
	GetCharacterMovement()->bConstrainToPlane = true;		//锁定到平面,将角色移动约束到平面（默认XY平面，即水平地面），禁止沿Z轴（垂直）自由移动
	GetCharacterMovement()->bSnapToPlaneAtStart = true;		// 角色初始化时自动吸附到约束平面，避免悬空/陷入地面
	
	// 2. 控制器旋转配置：禁用角色旋转跟随控制器
	bUseControllerRotationPitch = false;		// 角色俯仰旋转（抬头/低头）不跟随控制器
	bUseControllerRotationRoll = false;			// 角色翻滚旋转不跟随控制器
	bUseControllerRotationYaw = false;			// 角色偏航旋转（左右转）不跟随控制器（交由移动方向控制）
}

void AAuraCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	// Init ability actor info for the Server 服务器
	InitAbilityActorInfo();
	AddCharacterAbilities();
}

void AAuraCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	
	// Init ability actor info for the Client 客户端
	InitAbilityActorInfo();
}

int32 AAuraCharacter::GetPlayerLevel()
{
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	return AuraPlayerState->GetPlayerLevel();
}

void AAuraCharacter::InitAbilityActorInfo()
{
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	AuraPlayerState->GetAbilitySystemComponent()->InitAbilityActorInfo(AuraPlayerState, this);
	Cast<UAuraAbilitySystemComponent>(AuraPlayerState->GetAbilitySystemComponent() )->AbilityActorInfoSet();
	AbilitySystemComponent = AuraPlayerState->GetAbilitySystemComponent();
	AttributeSet = AuraPlayerState->GetAttributeSet();

	if (AAuraPlayerController* AuraPlayerController = Cast<AAuraPlayerController>(GetController() ) )
	{
		if (AAuraHUD* AuraHUD = Cast<AAuraHUD>(AuraPlayerController->GetHUD()))
		{
			AuraHUD->InitOverlay(AuraPlayerController, AuraPlayerState, AbilitySystemComponent, AttributeSet);
		}
	}
	InitializeDefaultAttributes();
}
