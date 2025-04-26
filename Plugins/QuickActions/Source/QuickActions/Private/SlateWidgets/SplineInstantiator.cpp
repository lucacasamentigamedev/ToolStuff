
#include "SlateWidgets/SplineInstantiator.h"
#include "PropertyCustomizationHelpers.h"
#include "Components/SplineComponent.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Engine/StaticMeshActor.h"
#include "EngineUtils.h"
#include "Editor/UnrealEd/Public/Selection.h"
#include "../DebugHeader.h"

void SSplineInstantiator::Construct(const FArguments& InArgs) {
	ChildSlot
		[
			SNew(SVerticalBox)

			// title
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(FMargin(0, 10, 0, 0))
			[
				SNew(STextBlock)
					.Text(FText::FromString("Spline Instantiator"))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 24))
			]

			//sub-title
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
			[
				SNew(STextBlock)
					.Text(FText::FromString("Instantiate any number of actor copies (min 2) along the spline curve"))
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12))
					.ColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f))
			]

			// actor picker
			+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0, 10))
			[
				SNew(SExpandableArea).HeaderContent()
				[
					SNew(STextBlock).Text(FText::FromString("Select an actor to istantiate"))
				].BodyContent()
						[
							// this is the widget that will be shown
							MakeActorPicker()
						]
			]

			// spline picker
			+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0, 10))
			[
				SNew(SExpandableArea).HeaderContent()
				[
					SNew(STextBlock).Text(FText::FromString("Select a spline to use as path"))
				].BodyContent()
						[
							MakeSplinePicker()
						]
			]

			// number of instances
			+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(20, 10, 20, 0))
			[
				SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().FillWidth(1.0f)
					[
						SNew(STextBlock).Text(FText::FromString("Number of instances"))
					]
					+ SHorizontalBox::Slot().FillWidth(1.0f)
					[
						SNew(SNumericEntryBox<int32>)
							.Value(3)
							.MinSliderValue(2)
							.MaxSliderValue(1000)
							.Value_Lambda([this]() {
								return numberOfInstances;
							})
							.OnValueChanged_Lambda([this](int32 value) {
								numberOfInstances = value;
							})
					]
			]

			//buttons
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(FMargin(0, 15))
			[
				SNew(SButton)
					.Text(FText::FromString("Istantiate"))
					.OnClicked(this, &SSplineInstantiator::OnSpawnButtonClicked)
			]
		];
}

TSharedRef<SWidget> SSplineInstantiator::MakeActorPicker()
{
	return PropertyCustomizationHelpers::MakeActorPickerWithMenu(
		nullptr, // InitialActor  pointer to initial actor
		false, // AllowClear
		FOnShouldFilterActor::CreateLambda([](const AActor* actor){
			// we make check to choose true / false representing if actor needs to be showed
			// in our widget. We are selecting actor to spawn into spline
			// need to be an actor not null and need to be not a spline actor
			if (!actor || actor->FindComponentByClass<USplineComponent>()) return false;
			return true;
		}),
		/* in this lambda i don't do if (actor) because this is sequentially after
		the previous one so the check is already done*/
		FOnActorSelected::CreateLambda([this](AActor* actor) {
			selectedActor = actor;
		}),
		// onClose
		FSimpleDelegate::CreateLambda([&]() -> void {}),
		// onUseSelected
		FSimpleDelegate::CreateLambda([&]() -> void {})
	);
}

TSharedRef<SWidget> SSplineInstantiator::MakeSplinePicker()
{
	return PropertyCustomizationHelpers::MakeActorPickerWithMenu(
		nullptr,
		false,
		FOnShouldFilterActor::CreateLambda([](const AActor* actor) {
			if (!actor) return false;
			//show only actors that have a spline component attached
			if (!actor->FindComponentByClass<USplineComponent>()) return false;
			return true;
		}),
		FOnActorSelected::CreateLambda([this](AActor* actor) {
			// assign the spline component not entire actor
			selectedSpline = actor->FindComponentByClass<USplineComponent>();
		}),
		FSimpleDelegate::CreateLambda([&]() -> void {}),
		FSimpleDelegate::CreateLambda([&]() -> void {})
	);
}
 
FReply SSplineInstantiator::OnSpawnButtonClicked()
{
	//right actors
	if (!selectedActor.IsValid() || !selectedSpline.IsValid()) {
		ShowDialog(EAppMsgType::Ok, TEXT("No actor or spline selected"));
		return FReply::Handled();
	}
	//at least 1 copy
	if (numberOfInstances < 2) {
		ShowDialog(EAppMsgType::Ok, TEXT("Number of instances must be at least 2"));
		return FReply::Handled();
	}
	//get world
	UWorld* world = GEditor->GetEditorWorldContext().World();
	if (!world) {
		ShowDialog(EAppMsgType::Ok, TEXT("No world found"));
		return FReply::Handled();
	}

	AActor* actorToDuplicate = selectedActor.Get();
	int32 wrongDuplications = 0;

	for (int32 i = 0; i < numberOfInstances; i++) {

		// Calculate transform along the spline
		float alpha = (float)i / (float)(numberOfInstances - 1);
		float splineTotalLength = selectedSpline->GetSplineLength();
		float distanceIntoTheSpline = splineTotalLength * alpha;

		//get actual transform
		FVector spawnLocation = selectedSpline->GetLocationAtDistanceAlongSpline(distanceIntoTheSpline, ESplineCoordinateSpace::World);
		FRotator spawnRotation = selectedSpline->GetRotationAtDistanceAlongSpline(distanceIntoTheSpline, ESplineCoordinateSpace::World);
		FVector spawnScale = selectedSpline->GetScaleAtDistanceAlongSpline(distanceIntoTheSpline);

		// Custom name for the new actor (starting from _2)
		FString baseName = selectedActor->GetName();

		// Deselect everything
		GEditor->SelectNone(false, true, false);
		// Select the actor to duplicate
		GEditor->SelectActor(selectedActor.Get(), true, true);
		// Duplicate the actor
		GEditor->edactDuplicateSelected(world->GetCurrentLevel(), false);

		// Get actual actor spawned
		AActor* duplicatedActor = GEditor->GetSelectedActors()->GetTop<AActor>();

		if (duplicatedActor) {
			// set transform
			duplicatedActor->SetActorLocation(spawnLocation);
			duplicatedActor->SetActorRotation(spawnRotation);
			duplicatedActor->SetActorScale3D(spawnScale);
		}
		else
		{
			// error count
			wrongDuplications++;
		}
	}

	if (wrongDuplications > 0) {
		//some duplicated are corrupted
		ShowDialog(EAppMsgType::Ok, FString::Printf(TEXT("%d are corrupted or invalid"), wrongDuplications));
	}
	else
	{
		// all OK
		ShowNotifyInfo(FString::Printf(TEXT("Actor duplicated successfully")));
	}
	return FReply::Handled();
}
