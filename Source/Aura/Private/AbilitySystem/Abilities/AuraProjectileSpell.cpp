// Copyright Li Qian


#include "AbilitySystem/Abilities/AuraProjectileSpell.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Actor/AuraProjectile.h"
#include "Interaction/CombatInterface.h"
#include "Aura/Public/AuraGameplayTags.h"

void UAuraProjectileSpell::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                           const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                           const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UAuraProjectileSpell::SpawnProjectile(const FVector& ProjectileTargetLocation)
{
	const bool bIsServer = GetAvatarActorFromActorInfo()->HasAuthority();
	if (!bIsServer) return;
	
	ICombatInterface* CombatInterface = Cast<ICombatInterface>(GetAvatarActorFromActorInfo());
	
	if (CombatInterface)
	{
		const FVector SocketLocation = CombatInterface->GetCombatSocketLocation();
		FRotator Rotation = (ProjectileTargetLocation - SocketLocation).Rotation();
		Rotation.Pitch = 0.f;
		
		FTransform SpawnTransform;
		SpawnTransform.SetLocation(SocketLocation);
		SpawnTransform.SetRotation(Rotation.Quaternion());
		
		AAuraProjectile* Projectile = GetWorld()->SpawnActorDeferred<AAuraProjectile>(
			ProjectileClass,
			SpawnTransform,
			GetOwningActorFromActorInfo(),
			Cast<APawn>(GetOwningActorFromActorInfo()),
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		
		//老师的程序
		//const UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetAvatarActorFromActorInfo());
		
		//调试-----------  1
		UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
		if (!SourceASC || !DamageEffectClass)
		{
			Projectile->FinishSpawning(SpawnTransform);
			return;
		}
		//核心逻辑：创建伤害EffectSpec
		FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
		EffectContext.AddSourceObject(GetAvatarActorFromActorInfo());
		
		const FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), SourceASC->MakeEffectContext());
		if (!SpecHandle.IsValid())
		{
			Projectile->FinishSpawning(SpawnTransform);
			return;
		}
		
		//const FAuraGameplayTags GameplayTags = FAuraGameplayTags::Get();		//老师的程序
		
		// ========== 核心：设置动态伤害值，标准赋值，动态伤害 + 正确传参 ========== 	
		const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();
		const float ScaledDamage = Damage.GetValueAtLevel(10);		//参数里是GetAbilityLevel()
		FName DamageDataName = GameplayTags.Damage.GetTagName();
		UAbilitySystemBlueprintLibrary::AssignSetByCallerMagnitude(SpecHandle, DamageDataName, ScaledDamage);
		
		FGameplayEffectSpec* Spec = SpecHandle.Data.Get();
		if (Spec)
		{
			// 直接调用原生接口
			Spec->SetSetByCallerMagnitude(GameplayTags.Damage, ScaledDamage);
		}
		
		//老师的程序
		//UAbilitySystemBlueprintLibrary::AssignSetByCallerMagnitude(SpecHandle,  GameplayTags.Damage.GetTagName(), ScaledDamage);
		
		Projectile->DamageEffectSpecHandle = SpecHandle;
		
		Projectile->FinishSpawning(SpawnTransform);
		
	}
	// ========== 复制这里开始 ==========  调试—— 3
	if (!DamageEffectClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("DamageEffectClass 未赋值！请检查蓝图配置"));
		return;
	}
	
}
