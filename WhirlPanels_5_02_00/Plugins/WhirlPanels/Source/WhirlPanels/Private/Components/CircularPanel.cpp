//This is free and unencumbered software released into the public domain.
//
//Anyone is free to copy, modify, publish, use, compile, sell, or
//distribute this software, either in source code form or as a compiled
//binary, for any purpose, commercial or non - commercial, and by any
//means.
//
//In jurisdictions that recognize copyright laws, the author or authors
//of this software dedicate any and all copyright interest in the
//software to the public domain.We make this dedication for the benefit
//of the public at large and to the detriment of our heirs and
//successors.We intend this dedication to be an overt act of
//relinquishment in perpetuity of all present and future rights to this
//software under copyright law.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
//OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
//ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
//OTHER DEALINGS IN THE SOFTWARE.
//
//For more information, please refer to < https://unlicense.org>

#include "Components/CircularPanel.h"
#include "Layout/ArrangedChildren.h"

#define LOCTEXT_NAMESPACE "WHIRLPANELS_CircularPanel"

void SCircularPanel::Construct(const FArguments& InArgs)
{
	Slots.AddSlots(MoveTemp(const_cast<TArray<FSlot::FSlotArguments>&>(InArgs._Slots)));
	
	SetRadiusA(InArgs._RadiusA);
	SetRadiusB(InArgs._RadiusB);
	SetAngle(InArgs._Angle);
}

void SCircularPanel::OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const
{
	const FVector2D halfLocalSize = 0.5f * AllottedGeometry.GetLocalSize();

	const int32 NumItems = Slots.Num();
	
	int NumVisibleItems = 0;

	for (int32 ChildIndex = 0; ChildIndex < NumItems; ++ChildIndex)
	{
		const FSlot& Slot = Slots[ChildIndex];
		const TSharedRef<SWidget>& Widget = Slot.GetWidget();

		if (Widget->GetVisibility() != EVisibility::Collapsed)
		{
			NumVisibleItems++;
		}
	}

	if (NumVisibleItems > 0)
	{
		const float angleRadStep = UE_TWO_PI / NumVisibleItems;
		float angleRad = FMath::DegreesToRadians(Angle);

		for (int32 ChildIndex = 0; ChildIndex < NumItems; ++ChildIndex)
		{
			const FSlot& Slot = Slots[ChildIndex];
			const TSharedRef<SWidget>& Widget = Slot.GetWidget();

			if (Widget->GetVisibility() != EVisibility::Collapsed)
			{
				const FVector2D DesiredSizeOfSlot = Widget->GetDesiredSize();

				FVector2D offset(
					halfLocalSize.X * (1 + FMath::Cos(angleRad) * RadiusA) - DesiredSizeOfSlot.X * Slot.GetPivot().X,
					halfLocalSize.Y * (1 + FMath::Sin(angleRad) * RadiusB) - DesiredSizeOfSlot.Y * Slot.GetPivot().Y);

				ArrangedChildren.AddWidget(AllottedGeometry.MakeChild(Widget, offset, DesiredSizeOfSlot));

				angleRad += angleRadStep;
			}
		}
	}
}



UCircularPanelSlot::UCircularPanelSlot(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer), Slot(nullptr)
{
	PRAGMA_DISABLE_DEPRECATION_WARNINGS

	Pivot = FVector2D(0.5f);

	PRAGMA_ENABLE_DEPRECATION_WARNINGS
}

FVector2D UCircularPanelSlot::GetPivot() const
{
	PRAGMA_DISABLE_DEPRECATION_WARNINGS

	return Slot ? Slot->GetPivot() : Pivot;
	
	PRAGMA_ENABLE_DEPRECATION_WARNINGS
}

void UCircularPanelSlot::SetPivot(FVector2D InPivot)
{
	PRAGMA_DISABLE_DEPRECATION_WARNINGS

	Pivot = InPivot;

	if (Slot)
	{
		Slot->SetPivot(InPivot);
	}

	PRAGMA_ENABLE_DEPRECATION_WARNINGS
}

void UCircularPanelSlot::SynchronizeProperties()
{
	PRAGMA_DISABLE_DEPRECATION_WARNINGS

	SetPivot(Pivot);

	PRAGMA_ENABLE_DEPRECATION_WARNINGS
}

void UCircularPanelSlot::BuildSlot(TSharedRef<SCircularPanel> InCircularPanel)
{
	PRAGMA_DISABLE_DEPRECATION_WARNINGS

	InCircularPanel->AddSlot().Expose(Slot).Pivot(Pivot)
		[
			Content == nullptr ? SNullWidget::NullWidget : Content->TakeWidget()
		];

	PRAGMA_ENABLE_DEPRECATION_WARNINGS
}

void UCircularPanelSlot::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	Slot = nullptr;
}



UCircularPanel::UCircularPanel(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bIsVariable = false;
	SetVisibilityInternal(ESlateVisibility::SelfHitTestInvisible);

	RadiusA = 0.5;
	RadiusB = 0.5;
	Angle = 0;
}

void UCircularPanel::SetRadiusA(float radiusA)
{
	RadiusA = radiusA;

	if (MyCircularPanel.IsValid())
	{
		MyCircularPanel->SetRadiusA(radiusA);
	}
}

void UCircularPanel::SetRadiusB(float radiusB)
{
	RadiusB = radiusB;

	if (MyCircularPanel.IsValid())
	{
		MyCircularPanel->SetRadiusB(radiusB);
	}
}

void UCircularPanel::SetAngle(float angle)
{
	Angle = angle;

	if (MyCircularPanel.IsValid())
	{
		MyCircularPanel->SetAngle(angle);
	}
}

UCircularPanelSlot* UCircularPanel::AddChildToCircularPanel(UWidget* Content) { return Cast<UCircularPanelSlot>(Super::AddChild(Content)); }

void UCircularPanel::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	MyCircularPanel->SetRadiusA(RadiusA);
	MyCircularPanel->SetRadiusB(RadiusB);
	MyCircularPanel->SetAngle(Angle);
}

void UCircularPanel::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	MyCircularPanel.Reset();
}

#if WITH_EDITOR

const FText UCircularPanel::GetPaletteCategory()
{
	return LOCTEXT("Panel", "Panel");
}

#endif

UClass* UCircularPanel::GetSlotClass() const { return UCircularPanelSlot::StaticClass(); }

void UCircularPanel::OnSlotAdded(UPanelSlot* InSlot)
{
	// Add the child to the live canvas if it already exists
	if (MyCircularPanel.IsValid())
	{
		CastChecked<UCircularPanelSlot>(InSlot)->BuildSlot(MyCircularPanel.ToSharedRef());
	}
}

void UCircularPanel::OnSlotRemoved(UPanelSlot* InSlot)
{
	// Remove the widget from the live slot if it exists.
	if (MyCircularPanel.IsValid() && InSlot->Content)
	{
		TSharedPtr<SWidget> Widget = InSlot->Content->GetCachedWidget();
		if (Widget.IsValid())
		{
			MyCircularPanel->RemoveSlot(Widget.ToSharedRef());
		}
	}
}

TSharedRef<SWidget> UCircularPanel::RebuildWidget()
{
	MyCircularPanel = SNew(SCircularPanel);

	for (UPanelSlot* PanelSlot : Slots)
	{
		if (UCircularPanelSlot* TypedSlot = Cast<UCircularPanelSlot>(PanelSlot))
		{
			TypedSlot->Parent = this;
			TypedSlot->BuildSlot(MyCircularPanel.ToSharedRef());
		}
	}

	return MyCircularPanel.ToSharedRef();
}

#undef LOCTEXT_NAMESPACE