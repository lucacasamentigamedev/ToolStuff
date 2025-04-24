
#include "SlateWidgets/SplineInstantiator.h"
#include "PropertyCustomizationHelpers.h"
#include "Components/SplineComponent.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "../DebugHeader.h"

void SSplineInstantiator::Construct(const FArguments& InArgs) {
	ChildSlot
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
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
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SExpandableArea).HeaderContent()
				[
					SNew(STextBlock).Text(FText::FromString("Select a spline to use as path"))
				].BodyContent()
						[
							// this is the widget that will be shown
							MakeSplinePicker()
						]
			]
			+ SVerticalBox::Slot().AutoHeight()
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
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
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
	for (int32 i = 0; i < numberOfInstances; i++) {
		// numberOfIstances-1 because we want to have the last point of the spline
		float alpha = (float)i / (float)(numberOfInstances-1);
		float splineTotalLength = selectedSpline->GetSplineLength();
		float distanceIntoTheSpline = splineTotalLength * alpha;
		// get location to spawn
		FVector spawnLocation = selectedSpline->GetLocationAtDistanceAlongSpline(
			distanceIntoTheSpline,
			ESplineCoordinateSpace::World
		);
		// get rotation to spawn
		FRotator spawnRotation = selectedSpline->GetRotationAtDistanceAlongSpline(
			distanceIntoTheSpline,
			ESplineCoordinateSpace::World
		);
		// get scale to spawn
		FVector spawnScale = selectedSpline->GetScaleAtDistanceAlongSpline(
			distanceIntoTheSpline
		);
		// spawn actor at location with rotation
		AActor* spawnedActor = world->SpawnActor<AActor>(
			selectedActor->GetClass(),
			spawnLocation,
			spawnRotation
		);
		// set scale
		if (spawnedActor) {
			spawnedActor->SetActorScale3D(spawnScale);
		}
	}
	return FReply::Handled();
}
