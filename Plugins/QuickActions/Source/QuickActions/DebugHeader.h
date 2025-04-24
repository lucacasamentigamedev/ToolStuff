//debug header for utility functions

#pragma once
#include "QuickActions.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"

// Print a message on-screen
inline void ScreenPrint(const FString& msg = TEXT("INSERT LOG"), const FColor& color = FColor::Red, const float duration = 10.f)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, duration, color, msg);
	}
}

// Print a message on log
inline void LogPrint(const FString& msg = TEXT("INSERT_LOG"))
{
	UE_LOG(LogQuickActions, Log, TEXT("%s"), *msg);
}

// Print a message on-screen and log it
inline void ScreenAndLogPrint(const FString& msg = TEXT("INSERT_LOG"), const FColor& color = FColor::Red, const float duration = 10.f)
{
	ScreenPrint(msg, color, duration);
	LogPrint(msg);
}

//Open custom dialog with message and title (error and warning)
inline EAppReturnType::Type ShowDialog(EAppMsgType::Type msgType, const FString& msg, const bool bIsWarning = true)
{
	FText title = bIsWarning ? FText::FromString(TEXT("Warning")) : FText::FromString(TEXT("Info"));
	return FMessageDialog::Open(msgType, FText::FromString(msg), &title);
}

//Open custom notification
inline void ShowNotifyInfo(const FString& msg)
{
	//create notification info
	FNotificationInfo notificationInfo(FText::FromString(msg));
	notificationInfo.bUseLargeFont = true;
	notificationInfo.FadeOutDuration = 5.f;

	//launch notification
	FSlateNotificationManager::Get().AddNotification(notificationInfo);
}