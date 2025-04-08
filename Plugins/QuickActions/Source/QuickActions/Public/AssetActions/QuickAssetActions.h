#pragma once

#include "CoreMinimal.h"
#include "AssetActionUtility.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceConstant.h"
#include "QuickAssetActions.generated.h"

/**
 * 
 */
UCLASS()
class QUICKACTIONS_API UQuickAssetActions : public UAssetActionUtility
{
	GENERATED_BODY()

public:

	UFUNCTION(CallInEditor)
	void TestAction();
	
	UFUNCTION(CallInEditor)
	void Duplicate(int32 numberOfCopies);

	UFUNCTION(CallInEditor)
	void FixPrefix();

private:
	TMap <UClass*, FString> prefixMap = {
		{ UBlueprint::StaticClass(), TEXT("BP_")},
		{ UMaterial::StaticClass(), TEXT("M_")},
		{ UMaterialInstanceConstant::StaticClass(), TEXT("MI_")}
	};
};
