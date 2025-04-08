// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

//custom log category name, verbosity default, verbosity compile-time
DECLARE_LOG_CATEGORY_EXTERN(LogQuickActions, Log, All);

class FQuickActionsModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
