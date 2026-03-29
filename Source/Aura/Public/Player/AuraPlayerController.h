// Copyright Li Qian

#pragma once

#include "CoreMinimal.h"		// 虚幻最核心的基础头文件
#include "GameFramework/PlayerController.h"		// 引入虚幻自带的“玩家控制器”基类
#include "GameplayTagContainer.h"
#include "AuraPlayerController.generated.h"		// 虚幻自动生成的头文件（反射用）

class UDamageTextComponent;
// 提前声明类（避免重复包含，简化编译）
class UInputMappingContext;		// 增强输入的“规则集”（告诉游戏：哪个按键对应哪个操作）
class UInputAction;				// 输入动作（比如“移动”这个动作）
struct FInputActionValue;		// 输入动作的数值（比如按W键时，数值是1；按S是-1）
class IEnemyInterface;
class UAuraInputConfig;
class UAuraAbilitySystemComponent;
class USplineComponent;

/**
 * 
 */
UCLASS()
class AURA_API AAuraPlayerController : public APlayerController
{
	GENERATED_BODY()		// 虚幻强制要求的宏：自动生成反射相关代码，少了会报错
	
public:
	AAuraPlayerController();				// 构造函数的“声明”（具体实现写在.cpp里）
	virtual void PlayerTick(float DeltaTime) override;
	
	UFUNCTION(Client, Reliable)
	void ShowDamageNumber(float DamageAmount, ACharacter* TargetCharacter);
protected:
	// 重写虚幻自带的两个函数（相当于“修改父类的默认行为”）
	virtual void BeginPlay() override;				//虚拟的 声明BeginPlaye()函数 覆盖，游戏开始时执行的函数
	virtual void SetupInputComponent() override;	// 设置输入组件（绑定按键的地方）
private:
	UPROPERTY(EditAnywhere, Category = "Input")		// 虚幻的属性宏：标记这个变量能在编辑器里改，归类到“Input”标签下
	TObjectPtr<UInputMappingContext> AuraContext;	// 增强输入的“规则集”（后续在编辑器里绑定：WASD对应“移动”动作）
	
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;			// “移动”这个输入动作（比如WASD/摇杆对应这个动作）
	
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> ShiftAction;
	
	void ShiftPressed()	{ bShiftKeyDown = true; }
	void ShiftReleased() { bShiftKeyDown = false; }
	bool bShiftKeyDown = false;
	
	// 处理移动的函数（声明）：收到输入后，让角色移动
	void Move(const FInputActionValue& InputActionValue);
	
	void CursorTrace();
	IEnemyInterface* LastActor;
	IEnemyInterface* ThisActor;
	FHitResult CursorHit;
	
	void AbilityInputTagPressed(FGameplayTag InputTag);
	void AbilityInputTagReleased(FGameplayTag InputTag);
	void AbilityInputTagHeld(FGameplayTag InputTag);
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UAuraInputConfig> InputConfig;
	
	UPROPERTY()
	TObjectPtr<UAuraAbilitySystemComponent> AuraAbilitySystemComponent;
	
	UAuraAbilitySystemComponent* GetASC();
	
	FVector CachedDestination = FVector::ZeroVector;
	float FollowTime = 0.f;
	float ShortPressThreshold = 0.5f;
	bool bAutoRunning = false;
	bool bTargeting = false;
	
	UPROPERTY(EditDefaultsOnly)
	float AutoRunAcceptanceRadius = 50.f;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USplineComponent> Spline;
	
	void AutoRun();
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UDamageTextComponent> DamageTextComponentClass;
};
