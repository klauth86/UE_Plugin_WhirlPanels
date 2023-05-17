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

#include "Components/SpiralPanel.h"
#include "Layout/ArrangedChildren.h"

#define LOCTEXT_NAMESPACE "WHIRLPANELS_SpiralPanel"

void SSpiralPanel::Construct(const FArguments& InArgs)
{
	Slots.AddSlots(MoveTemp(const_cast<TArray<FSlot::FSlotArguments>&>(InArgs._Slots)));

	SetRadius(InArgs._Radius);
	SetRadiusStep(InArgs._RadiusStep);
	SetAngle(InArgs._Angle);
	SetAngleStep(InArgs._AngleStep);
}

void SSpiralPanel::OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const
{
	const FVector2D halfLocalSize = 0.5f * AllottedGeometry.GetLocalSize();

	float radius = Radius;
	const float radiusStep = RadiusStep;

	float angleRad = FMath::DegreesToRadians(Angle);
	const float angleStepRad = FMath::DegreesToRadians(AngleStep);

	for (int32 ChildIndex = 0; ChildIndex < Slots.Num(); ++ChildIndex)
	{
		const FSlot& Slot = Slots[ChildIndex];
		const TSharedRef<SWidget>& Widget = Slot.GetWidget();

		if (Widget->GetVisibility() != EVisibility::Collapsed)
		{
			const FVector2D DesiredSizeOfSlot = Widget->GetDesiredSize();

			FVector2D offset(
				halfLocalSize.X * (1 + FMath::Cos(angleRad) * radius) - DesiredSizeOfSlot.X * Slot.GetPivot().X,
				halfLocalSize.Y * (1 + FMath::Sin(angleRad) * radius) - DesiredSizeOfSlot.Y * Slot.GetPivot().Y);

			ArrangedChildren.AddWidget(AllottedGeometry.MakeChild(Widget, offset, DesiredSizeOfSlot));

			radius += radiusStep;
			angleRad += angleStepRad;
		}
	}
}



USpiralPanelSlot::USpiralPanelSlot(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer), Slot(nullptr)
{
	PRAGMA_DISABLE_DEPRECATION_WARNINGS

		Pivot = FVector2D(0.5f);

	PRAGMA_ENABLE_DEPRECATION_WARNINGS
}

FVector2D USpiralPanelSlot::GetPivot() const
{
	PRAGMA_DISABLE_DEPRECATION_WARNINGS

		return Slot ? Slot->GetPivot() : Pivot;

	PRAGMA_ENABLE_DEPRECATION_WARNINGS
}

void USpiralPanelSlot::SetPivot(FVector2D InPivot)
{
	PRAGMA_DISABLE_DEPRECATION_WARNINGS

		Pivot = InPivot;

	if (Slot)
	{
		Slot->SetPivot(InPivot);
	}

	PRAGMA_ENABLE_DEPRECATION_WARNINGS
}

void USpiralPanelSlot::SynchronizeProperties()
{
	PRAGMA_DISABLE_DEPRECATION_WARNINGS

		SetPivot(Pivot);

	PRAGMA_ENABLE_DEPRECATION_WARNINGS
}

void USpiralPanelSlot::BuildSlot(TSharedRef<SSpiralPanel> InSpiralPanel)
{
	PRAGMA_DISABLE_DEPRECATION_WARNINGS

		InSpiralPanel->AddSlot().Expose(Slot).Pivot(Pivot)
		[
			Content == nullptr ? SNullWidget::NullWidget : Content->TakeWidget()
		];

	PRAGMA_ENABLE_DEPRECATION_WARNINGS
}

void USpiralPanelSlot::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	Slot = nullptr;
}



USpiralPanel::USpiralPanel(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bIsVariable = false;
	SetVisibilityInternal(ESlateVisibility::SelfHitTestInvisible);

	Radius = 0.25;
	RadiusStep = 0.1;
	Angle = 0;
	AngleStep = 10;
}

void USpiralPanel::SetRadius(float radius)
{
	Radius = radius;

	if (MySpiralPanel.IsValid())
	{
		MySpiralPanel->SetRadius(radius);
	}
}

void USpiralPanel::SetRadiusStep(float radiusStep)
{
	RadiusStep = radiusStep;

	if (MySpiralPanel.IsValid())
	{
		MySpiralPanel->SetRadiusStep(radiusStep);
	}
}

void USpiralPanel::SetAngle(float angle)
{
	Angle = angle;

	if (MySpiralPanel.IsValid())
	{
		MySpiralPanel->SetAngle(angle);
	}
}

void USpiralPanel::SetAngleStep(float angleStep)
{
	AngleStep = angleStep;

	if (MySpiralPanel.IsValid())
	{
		MySpiralPanel->SetAngleStep(angleStep);
	}
}

USpiralPanelSlot* USpiralPanel::AddChildToSpiralPanel(UWidget* Content) { return Cast<USpiralPanelSlot>(Super::AddChild(Content)); }

void USpiralPanel::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	MySpiralPanel->SetRadius(Radius);
	MySpiralPanel->SetRadiusStep(RadiusStep);
	MySpiralPanel->SetAngle(Angle);
	MySpiralPanel->SetAngleStep(AngleStep);
}

void USpiralPanel::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	MySpiralPanel.Reset();
}

#if WITH_EDITOR

const FText USpiralPanel::GetPaletteCategory()
{
	return LOCTEXT("Panel", "Panel");
}

#endif

UClass* USpiralPanel::GetSlotClass() const { return USpiralPanelSlot::StaticClass(); }

void USpiralPanel::OnSlotAdded(UPanelSlot* InSlot)
{
	// Add the child to the live canvas if it already exists
	if (MySpiralPanel.IsValid())
	{
		CastChecked<USpiralPanelSlot>(InSlot)->BuildSlot(MySpiralPanel.ToSharedRef());
	}
}

void USpiralPanel::OnSlotRemoved(UPanelSlot* InSlot)
{
	// Remove the widget from the live slot if it exists.
	if (MySpiralPanel.IsValid() && InSlot->Content)
	{
		TSharedPtr<SWidget> Widget = InSlot->Content->GetCachedWidget();
		if (Widget.IsValid())
		{
			MySpiralPanel->RemoveSlot(Widget.ToSharedRef());
		}
	}
}

TSharedRef<SWidget> USpiralPanel::RebuildWidget()
{
	MySpiralPanel = SNew(SSpiralPanel);

	for (UPanelSlot* PanelSlot : Slots)
	{
		if (USpiralPanelSlot* TypedSlot = Cast<USpiralPanelSlot>(PanelSlot))
		{
			TypedSlot->Parent = this;
			TypedSlot->BuildSlot(MySpiralPanel.ToSharedRef());
		}
	}

	return MySpiralPanel.ToSharedRef();
}

#undef LOCTEXT_NAMESPACE