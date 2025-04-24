// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetActions/QuickAssetActions.h"
#include "../DebugHeader.h"
#include "EditorUtilityLibrary.h"
#include "EditorAssetLibrary.h"


void UQuickAssetActions::TestAction()
{
	ScreenPrint(TEXT("TestAction called! Log from screen"), FColor::Emerald);	
	LogPrint(TEXT("TestAction called! Log from log"));
}

//duplicate selected asset asking number of copies
void UQuickAssetActions::Duplicate(int32 numberOfCopies)
{
	if (numberOfCopies <= 0)
	{
		ScreenAndLogPrint(TEXT("Number of copies must be greater than 0"), FColor::Red);
		ShowDialog(EAppMsgType::Ok, TEXT("Number of copies must be greater than 0"));
		return;
	}	

	//get array of selected assets data
	TArray<FAssetData> selectedAssetsData = UEditorUtilityLibrary::GetSelectedAssetData();
	uint32 counter = 0;

	for (const FAssetData& selectedAssetData : selectedAssetsData)
	{
		//get asset source
		const FString sourcePath = selectedAssetData.ObjectPath.ToString();

		for (int i = 0; i < numberOfCopies; i++)
		{
			//create new different asset name 
			const FString newAssetName = selectedAssetData.AssetName.ToString() + TEXT("_Copy_") + FString::FromInt(i + 1);
			//create complete destination path
			const FString destinationPath = FPaths::Combine(selectedAssetData.PackagePath.ToString(), newAssetName);
			//try to duplicate the asset
			if (UEditorAssetLibrary::DuplicateAsset(sourcePath, destinationPath))
			{
				//save asset
				UEditorAssetLibrary::SaveAsset(destinationPath, false);
				counter++;
			}
		}
	}

	if (counter > 0)
	{
		ScreenAndLogPrint(FString::Printf(TEXT("Duplicated %d assets"), counter), FColor::Emerald);
		ShowNotifyInfo(FString::Printf(TEXT("Duplicated %d assets"), counter));
	}
	else
	{
		ScreenAndLogPrint(TEXT("No assets duplicated"), FColor::Red);
		ShowDialog(EAppMsgType::Ok, TEXT("No assets duplicated"));
	}
}

void UQuickAssetActions::FixPrefix()
{
	TArray<UObject*> selectedAssets = UEditorUtilityLibrary::GetSelectedAssets();
	uint32 counter = 0;

	for (UObject* obj : selectedAssets)
	{
		//is a pointer so check if valid
		if (!obj) {
			continue;
		}

		//get prefix from Map and continue if not found
		FString* prefix = prefixMap.Find(obj->GetClass());
		if (!prefix || prefix->IsEmpty())
		{
			ScreenAndLogPrint(TEXT("Failed to find prefix for class " + obj->GetClass()->GetName()));
			continue;
		}

		//avoid errors like M_M_Name
		FString oldName = obj->GetName();
		if (oldName.StartsWith(*prefix))
		{
			ScreenAndLogPrint(oldName + TEXT(" already has prefix " + *prefix));
			continue;
		}

		//for material instance specific asset i make ad hoc fix
		if (obj->IsA<UMaterialInstanceConstant>())
		{
			oldName.RemoveFromStart(TEXT("M_"));
			oldName.RemoveFromEnd(TEXT("_Inst"));
		}

		//compose new name and rename
		FString newName = *prefix + oldName;
		UEditorUtilityLibrary::RenameAsset(obj, newName);
		++counter;
	}

	//logs
	if (counter > 0)
	{
		ScreenAndLogPrint(FString::Printf(TEXT("Fix prefix for %d assets"), counter), FColor::Emerald);
		ShowNotifyInfo(FString::Printf(TEXT("Fix prefix for %d assets"), counter));
	}
	else
	{
		ScreenAndLogPrint(TEXT("No assets fixed"), FColor::Red);
		ShowDialog(EAppMsgType::Ok, TEXT("No assets fixed"));
	}
}

void UQuickAssetActions::BatchRename(
	const FString& prefix,
	const FString& suffix,
	const FString& prefixSeparator,
	const FString& suffixSeparator
)
{
	if (prefix.IsEmpty() && suffix.IsEmpty())
	{
		return FixPrefix();
	}

	TArray<UObject*> selectedAssets = UEditorUtilityLibrary::GetSelectedAssets();
	uint32 counter = 0;

	for (UObject* obj : selectedAssets)
	{
		//is a pointer so check if valid
		if (!obj) {
			continue;
		}

		//get prefix from Map and continue if not found
		FString* epicGamesPrefix = prefixMap.Find(obj->GetClass());
		if (!epicGamesPrefix || epicGamesPrefix->IsEmpty())
		{
			ScreenAndLogPrint(TEXT("Failed to find prefix for class " + obj->GetClass()->GetName()));
			continue;
		}

		const FString oldName = obj->GetName();
		FString newName = oldName;
		if (!prefix.IsEmpty())
		{
			if (oldName.StartsWith(*epicGamesPrefix))
			{
				ScreenAndLogPrint(oldName + TEXT(" already has prefix " + *epicGamesPrefix));
				newName.InsertAt(epicGamesPrefix->Len(), prefixSeparator + prefix);
			}
			else
			{
				newName.InsertAt(0, *epicGamesPrefix);
				newName.InsertAt(epicGamesPrefix->Len(), prefixSeparator + prefix);
			}
		}
		else
		{
			if (oldName.StartsWith(*epicGamesPrefix))
			{
				ScreenAndLogPrint(oldName + TEXT(" already has prefix " + *epicGamesPrefix));
			}
			else
			{
				newName.InsertAt(0, *epicGamesPrefix);
			}
		}

		if (!suffix.IsEmpty())
		{
			newName.Append(suffixSeparator + suffix);
		}

		UEditorUtilityLibrary::RenameAsset(obj, newName);
	}
}