// Copyright Epic Games, Inc. All Rights Reserved.

#include "QuickActions.h"
#include "ContentBrowserModule.h"
#include "SlateWidgets/MyCustomWidget.h"
#include "SlateWidgets/SplineInstantiator.h"

#define LOCTEXT_NAMESPACE "FQuickActionsModule"

//custom log category
DEFINE_LOG_CATEGORY(LogQuickActions);

#pragma region ModuleInterface
void FQuickActionsModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	InitCBExtension();
	RegisterSlateTab();
}

void FQuickActionsModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#pragma endregion ModuleInterface

#pragma region ContentBrowserExtension
void FQuickActionsModule::AddCBMenuEntry(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddMenuEntry(
		FText::FromString(TEXT("Spawn test Nomad")),
		FText::FromString(TEXT("Slate test Nomad spawn")),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Details"),
		FExecuteAction::CreateRaw(this, &FQuickActionsModule::OnSpawnTabClicked)
	);
}

void FQuickActionsModule::InitCBExtension()
{
	FContentBrowserModule& CBModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	//unreal menu entry list
	TArray<FContentBrowserMenuExtender_SelectedPaths>& CBExtenders = CBModule.GetAllPathViewContextMenuExtenders();
	FContentBrowserMenuExtender_SelectedPaths CustomCBMenuDelegate;
	CustomCBMenuDelegate.BindRaw(this, &FQuickActionsModule::CustomCBExtender);
	CBExtenders.Add(CustomCBMenuDelegate);
}

TSharedRef<FExtender> FQuickActionsModule::CustomCBExtender(const TArray<FString>& SelectedPaths)
{
	//1 bind hook menu
	//2 menubuild
	//3 function
	TSharedRef<FExtender> MenuExtender(new FExtender());
	if (SelectedPaths.Num() > 0)
	{
		//1 bind hook menu
		MenuExtender->AddMenuExtension(FName("PathContextBulkOperations"),
			EExtensionHook::First,
			TSharedPtr<FUICommandList>(),
			FMenuExtensionDelegate::CreateRaw(this, &FQuickActionsModule::AddCBMenuEntry) //2 menubuild
		);
	}
	return MenuExtender;
}
#pragma endregion ContentBrowserExtension

#pragma region Slate

void FQuickActionsModule::RegisterSlateTab()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		FName("TestTab"),
		FOnSpawnTab::CreateRaw(this, &FQuickActionsModule::OnSpawnTestTab))
		.SetDisplayName(FText::FromString(TEXT("Nomad tab test")));
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		FName("SplineIstantiator"),
		FOnSpawnTab::CreateLambda([](const FSpawnTabArgs& args) {
			return SNew(SDockTab).TabRole(ETabRole::NomadTab)
				[
					SNew(SSplineInstantiator)
				];
		})
	);
}

void FQuickActionsModule::OnSpawnTabClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(FName("TestTab"));
}

TSharedRef<SDockTab> FQuickActionsModule::OnSpawnTestTab(const FSpawnTabArgs& args)
{
	return SNew(SDockTab).TabRole(ETabRole::NomadTab)
		[
			SNew(SMyCustomWidget).TestString(TEXT("Test data pasing"))
		];
}

#pragma endregion Slate

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FQuickActionsModule, QuickActions)