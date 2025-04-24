// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

//custom log category name, verbosity default, verbosity compile-time
DECLARE_LOG_CATEGORY_EXTERN(LogQuickActions, Log, All);

class FQuickActionsModule : public IModuleInterface
{
public:

#pragma region ModuleInterface
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
#pragma endregion ModuleInterface

private:

#pragma region ContentBrowserExtension
	void InitCBExtension();
	TSharedRef<FExtender> CustomCBExtender(const TArray<FString>& SelectedPaths);
	TArray<FString> SelectedFolderPaths;
	void AddCBMenuEntry(FMenuBuilder& MenuBuilder);
#pragma endregion ContentBrowserExtension

#pragma region Slate
	void RegisterSlateTab();
	void OnSpawnTabClicked();
	TSharedRef<SDockTab> OnSpawnTestTab(const FSpawnTabArgs& args);
#pragma endregion Slate
};
