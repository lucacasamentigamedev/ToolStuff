#pragma once

#include "Widgets/SCompoundWidget.h"

class SMyCustomWidget : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SMyCustomWidget) {}
	SLATE_ARGUMENT(FString, TestString)
	SLATE_END_ARGS();

public:
	void Construct(const FArguments& InArgs);

private:
	FReply HandleButtonCLick();
};