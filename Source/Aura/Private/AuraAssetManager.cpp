// Copyright Li Qian


#include "AuraAssetManager.h"

UAuraAssetManager* UAuraAssetManager::Get()
{
	check(GEngine);
	
	UAuraAssetManager* AuraAssetManager = Cast<UAuraAssetManager>(GEngine->AssetManager);
	return &*AuraAssetManager;	//老师的是：return *AuraAssetManager
}
