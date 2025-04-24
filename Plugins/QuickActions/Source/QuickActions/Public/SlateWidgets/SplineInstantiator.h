#pragma once

#include "Widgets/SCompoundWidget.h"

// there 2 classes can go in forward declaration because they are pointer
// object pointer = same dimensions = can be used in forward declaration
class AActor;
class USplineComponent;

class SSplineInstantiator : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SSplineInstantiator) {}
	SLATE_END_ARGS();

public:
	void Construct(const FArguments& InArgs);

private:
	TSharedRef<SWidget> MakeActorPicker();
	/*
		we use TWeakObjectPtr because if i use a shared pointer i wouuld have a
		shared ref to an object in scene and what if i change scene?
		remain append in memory the object not allowing delete
		so: my plugin does not need to inlfuence the lifecycle of the object
		i don't want to block the delete of the object
	*/
	TWeakObjectPtr<AActor> selectedActor;
	TSharedRef<SWidget> MakeSplinePicker();
	TWeakObjectPtr<USplineComponent> selectedSpline;
	int32 numberOfInstances = 3;
	// callback when the button is clicked
	FReply OnSpawnButtonClicked();
};