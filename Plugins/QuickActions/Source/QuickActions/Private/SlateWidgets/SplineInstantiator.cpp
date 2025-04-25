
#include "SlateWidgets/SplineInstantiator.h"
#include "PropertyCustomizationHelpers.h"
#include "Components/SplineComponent.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Engine/StaticMeshActor.h"
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
					.Text(FText::FromString("Luca Casamenti"))
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

		// Prepare spawn parameters stom name
		FString baseName = selectedActor->GetName(); // Example: BP_MyCube
		FString customName = FString::Printf(TEXT("%s_%d"), *baseName, i+2); // Example: BP_MyCube_2
		FActorSpawnParameters spawnParams;
		spawnParams.Name = FName(*customName);

		AActor* spawnedActor = nullptr;

		// the actor selected is a static mesh
		AStaticMeshActor* selectedStaticMeshActor = Cast<AStaticMeshActor>(selectedActor.Get());
		if (selectedStaticMeshActor) {
			// get mesh to copy
			UStaticMesh* meshToCopy = selectedStaticMeshActor->GetStaticMeshComponent()->GetStaticMesh();
			// spawn 
			AStaticMeshActor* spawnedStaticMeshActor = world->SpawnActor<AStaticMeshActor>(
				AStaticMeshActor::StaticClass(), spawnLocation, spawnRotation
			);
			// set static mesh and scale
			if (spawnedStaticMeshActor && meshToCopy) {
				spawnedStaticMeshActor->GetStaticMeshComponent()->SetStaticMesh(meshToCopy);
				spawnedStaticMeshActor->SetActorScale3D(spawnScale);
				spawnedActor = spawnedStaticMeshActor;
			}
		}
		// default case ( es: blueprint)
		else
		{
			// spawn actor at location with rotation
			spawnedActor = world->SpawnActor<AActor>(
				selectedActor->GetClass(),
				spawnLocation,
				spawnRotation
			);
			// set scale
			if (spawnedActor) {
				spawnedActor->SetActorScale3D(spawnScale);
			}
		}

		// Place the spawned actor right after the selected one in the World Outliner
		if (spawnedActor && selectedActor.IsValid()) {
			spawnedActor->SetFolderPath(selectedActor->GetFolderPath());
		}
	}
	return FReply::Handled();
}
