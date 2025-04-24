// Fill out your copyright notice in the Description page of Project Settings.


#include "SlateWidgets/MyCustomWidget.h"
#include "../DebugHeader.h"

void SMyCustomWidget::Construct(const FArguments& inArgs)
{
	//permit the widget to get focus
	bCanSupportFocus = true;
	//get TestString var passed from QuiCkActions.cpp
	inArgs._TestString;
	//get font style
	FSlateFontInfo TitleFont = FCoreStyle::Get().GetFontStyle(FName("EmbossedText"));
	TitleFont.Size = 30;
	ChildSlot
	[
		//child slot with vertical box
		SNew(SVerticalBox)
		//vbox slot with title
		+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
					.Text(FText::FromString(TEXT("MyCustomSlateTab")))
					.Font(TitleFont)
					.ColorAndOpacity(FColor::White)
					.Justification(ETextJustify::Center)
			]
		+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SButton)
					.Text(FText::FromString("Click me"))
					.OnClicked(this, &SMyCustomWidget::HandleButtonCLick)
			]
	];
}

FReply SMyCustomWidget::HandleButtonCLick()
{
	ScreenAndLogPrint(TEXT("ButtonClicked"));
	return FReply::Handled();
}
