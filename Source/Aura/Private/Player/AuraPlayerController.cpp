// Copyright Li Qian


#include "Player/AuraPlayerController.h"		// 引入自己的头文件（必须）

#include "AbilitySystemBlueprintLibrary.h"
#include "AuraGameplayTags.h"
#include "EnhancedInputSubsystems.h"			// 引入增强输入的子系统头文件（处理输入规则的核心）
#include "EnhancedInputComponent.h"				// 引入增强输入组件头文件（绑定按键的核心）
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "Components/SplineComponent.h"
#include "Input/AuraInputComponent.h"
#include "Interaction/EnemyInterface.h"

AAuraPlayerController::AAuraPlayerController()
{
	bReplicates = true;							// 开启网络同步（多人游戏时，控制器状态同步给其他玩家）
	
	Spline = CreateDefaultSubobject<USplineComponent>("Spline");
}

void AAuraPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
	CursorTrace();
	AutoRun();
}

void AAuraPlayerController::AutoRun()
{
	if (!bAutoRunning) return;
	if (APawn* ControlledPawn = GetPawn())
	{
		const FVector LocationOnSpline = Spline->FindLocationClosestToWorldLocation(ControlledPawn->GetActorLocation(), ESplineCoordinateSpace::World);
		const FVector Direction = Spline->FindDirectionClosestToWorldLocation(LocationOnSpline, ESplineCoordinateSpace::World);
		ControlledPawn->AddMovementInput(Direction);
		
		const float DistanceToDestination = (LocationOnSpline - CachedDestination).Length();
		if (DistanceToDestination <= AutoRunAcceptanceRadius)
		{
			bAutoRunning = false;
		}
	}
}

void AAuraPlayerController::CursorTrace()
{
	GetHitResultUnderCursor(ECC_Visibility,false,CursorHit);
	if (!CursorHit.bBlockingHit) return;
	
	LastActor = ThisActor;
	ThisActor = Cast<IEnemyInterface>(CursorHit.GetActor());

	if (LastActor != ThisActor)
	{
		if (LastActor)	LastActor->UnHighlightActor();
		if (ThisActor)	ThisActor->HighlightActor();
	}
	
	/**
	 * Line trace from cursor. There are several scenarios：
	 * A. LastActor is null && ThisActor is null
	 *		- Do nothing
	 * B. LastActor is null && ThisActor is valid
	 *		- Highlight ThisActor
	 * C. LastActor is valid && ThisActor is null
	 *		- UnHighlight LastActor
	 * D. Both actors are valid, but LastActor != ThisActor
	 *		- UnHighlight LastActor and Highlight ThisActor
	 * E. Both actors are valid, and are the same actor
	 *		- Do nothing
	 */

	// if (LastActor == nullptr)		//不等于空指针
	// {
	// 	if (ThisActor != nullptr)
	// 	{
	// 		// Case B
	// 		ThisActor->HighlightActor();
	// 	}
	// 	else
	// 	{
	// 		// Case A - both are null, do nothing
	// 	}
	// }
	// else	// LastActor is valid
	// {
	// 	if (ThisActor == nullptr)
	// 	{
	// 		// Case C
	// 		LastActor->UnHighlightActor();
	// 	}
	// 	else	// both actors are valid
	// 	{
	// 		if (LastActor != ThisActor)
	// 		{
	// 			// Case D
	// 			LastActor->UnHighlightActor();
	// 			ThisActor->HighlightActor();
	// 		}
	// 		else
	// 		{
	// 			// Case E - do nothing
	// 		}
	// 	}
	// }
}

void AAuraPlayerController::AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_LMB))
	{
		bTargeting = ThisActor ? true : false;
		bAutoRunning = false;
	}
}

void AAuraPlayerController::AbilityInputTagReleased(FGameplayTag InputTag)
{
	if (!InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_LMB))
	{
		if (GetASC())	GetASC()->AbilityInputTagReleased(InputTag);
		return;
	}
	
	if (bTargeting)
	{
		if (GetASC())	GetASC()->AbilityInputTagReleased(InputTag);
	}
	else
	{
		const APawn* ControlledPawn = GetPawn();
		if (FollowTime <= ShortPressThreshold && ControlledPawn)
		{
			if (UNavigationPath* NavPath = UNavigationSystemV1::FindPathToLocationSynchronously(this, ControlledPawn->GetActorLocation(), CachedDestination) )
			{
				Spline->ClearSplinePoints();
			 	for (const FVector& PointLoc : NavPath->PathPoints)
			 	{
					Spline->AddSplinePoint(PointLoc, ESplineCoordinateSpace::World);
			 		//DrawDebugSphere(GetWorld(), PointLoc, 8.f, 8, FColor::Green, false, 5.f);		//老师删掉了可视化路径线
			 	}
			 	CachedDestination = NavPath->PathPoints[NavPath->PathPoints.Num() - 1];
				bAutoRunning = true;
			}
		}
		FollowTime = 0.f;
		bTargeting = false;
	}
}

void AAuraPlayerController::AbilityInputTagHeld(FGameplayTag InputTag)
{
	if (!InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_LMB))
	{
		if (GetASC())	GetASC()->AbilityInputTagHeld(InputTag);
		return;
	}

	if (bTargeting)
	{
		if (GetASC())	GetASC()->AbilityInputTagHeld(InputTag);
	}
	else
	{
		FollowTime += GetWorld()->GetDeltaSeconds();
		if (CursorHit.bBlockingHit)	CachedDestination = CursorHit.ImpactPoint;

		if (APawn* ControlledPawn = GetPawn())
		{
			const FVector WorldDirection = (CachedDestination - ControlledPawn->GetActorLocation()).GetSafeNormal();
			ControlledPawn->AddMovementInput(WorldDirection);
		}
	}
}

UAuraAbilitySystemComponent* AAuraPlayerController::GetASC()
{
	if (AuraAbilitySystemComponent == nullptr)
	{
		AuraAbilitySystemComponent = Cast<UAuraAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn<APawn>() ) );
	}
	return AuraAbilitySystemComponent;
}

// BeginPlay的实现：游戏开始（玩家进入世界）时执行
void AAuraPlayerController::BeginPlay()
{
	Super::BeginPlay();					// 先执行父类（APlayerController）的BeginPlay，这是规矩
	check(AuraContext);					// 检查AuraContext是否有效（如果没配置，编辑器会报错，防止崩溃）
	
	//本地指针子系统 增强输入系统
	// 1. 启用增强输入的“规则集”（AuraContext），获取本地玩家的“增强输入子系统”（管理输入规则的核心模块）
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (Subsystem)
	{
		Subsystem->AddMappingContext(AuraContext,0);		// 添加规则集，优先级0（数字越小优先级越高）
	}
	
	// 2. 设置鼠标光标
	bShowMouseCursor = true;		// 显示鼠标光标（比如RPG游戏需要点UI，就要显示光标）
	DefaultMouseCursor = EMouseCursor::Default;		// 光标样式设为“默认”
	
	// 3. 设置输入模式：既能操作游戏（WASD移动），又能操作UI（点击按钮）
	FInputModeGameAndUI InputModeData;
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);		// 鼠标不锁定在游戏窗口里
	InputModeData.SetHideCursorDuringCapture(false);		// 即使鼠标被游戏捕捉，也不隐藏光标
	SetInputMode(InputModeData);		//传递输入模式数据，应用这个输入模式
}

// SetupInputComponent的实现：绑定“按键-函数”（比如按W键，调用Move函数）
void AAuraPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();			// 先执行父类的SetupInputComponent
	
	UAuraInputComponent* AuraInputComponent = CastChecked<UAuraInputComponent>(InputComponent);
	AuraInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAuraPlayerController::Move);
	AuraInputComponent->BindAbilityActions(InputConfig, this, &ThisClass::AbilityInputTagPressed, &ThisClass::AbilityInputTagReleased, &ThisClass::AbilityInputTagHeld);
}

// Move函数的实现：处理移动输入（核心！按WASD让角色移动）
void AAuraPlayerController::Move(const FInputActionValue& InputActionValue)
{
	// 1. 获取输入的数值（2D向量：X对应左右（AD），Y对应前后（WS））
	const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>();	
	// 2. 获取控制器的旋转角度（只取Yaw：左右转的角度，忽略俯仰/翻滚）
	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);
	
	// 3. 计算“前进方向”和“右方向”（根据控制器朝向，比如你转视角，前进方向也跟着变）
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);	 // 前
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);		 // 右

	// 4. 让角色移动：先获取控制器控制的角色（Pawn），再给角色加移动输入
	if (APawn* ControlledPawn = GetPawn<APawn>())
	{
		// 前后移动：前进方向 * Y轴值（W=1，S=-1）
		ControlledPawn->AddMovementInput(ForwardDirection, InputAxisVector.Y);
		// 左右移动：右方向 * X轴值（D=1，A=-1）
		ControlledPawn->AddMovementInput(RightDirection, InputAxisVector.X);
	}
}


