// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/CircularPanel3D.h"

#define LOCTEXT_NAMESPACE "WHIRLPANELS_CircularPanel3D"

void SCircularPanel3D::Construct(const FArguments& InArgs)
{
	Slots.AddSlots(MoveTemp(const_cast<TArray<FSlot::FSlotArguments>&>(InArgs._Slots)));

	SetRadiusA(InArgs._RadiusA);
	SetRadiusB(InArgs._RadiusB);
	SetAlpha(InArgs._Alpha);
	SetBetta(InArgs._Betta);
	SetAngle(InArgs._Angle);
	SetFocusZ(InArgs._FocusZ);
	SetProjectionZ(InArgs._ProjectionZ);
	SetDrawDebugEllipse(InArgs._bDrawDebugEllipse);
}

void SCircularPanel3D::OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const
{
	const FVector2D halfLocalSize = 0.5 * AllottedGeometry.GetLocalSize();

	const int32 NumItems = Slots.Num();

	int NumVisibleItems = 0;

	TMap<TSharedPtr<SWidget>, int32> widgetSlots;

	for (int32 ChildIndex = 0; ChildIndex < NumItems; ++ChildIndex)
	{
		const FSlot& Slot = Slots[ChildIndex];
		const TSharedRef<SWidget>& Widget = Slot.GetWidget();

		widgetSlots.Add(Widget, ChildIndex);

		if (Widget->GetVisibility() != EVisibility::Collapsed)
		{
			NumVisibleItems++;
		}
	}

	if (NumVisibleItems > 0)
	{
		TArray<TSharedPtr<SWidget>> widgets;
		widgets.Reserve(NumVisibleItems);

		TMap<TSharedPtr<SWidget>, FVector> widgetOffsets;
		widgetOffsets.Reserve(NumVisibleItems);

		const float angleRadStep = UE_TWO_PI / NumVisibleItems;
		float angleRad = FMath::DegreesToRadians(Angle);

		for (int32 ChildIndex = 0; ChildIndex < NumItems; ++ChildIndex)
		{
			const FSlot& Slot = Slots[ChildIndex];
			const TSharedRef<SWidget>& Widget = Slot.GetWidget();

			if (Widget->GetVisibility() != EVisibility::Collapsed)
			{
				const FVector offset3D(
					halfLocalSize.X * FMath::Cos(angleRad) * RadiusA,
					halfLocalSize.Y * FMath::Sin(angleRad) * RadiusB, 0);

				const FVector rotatedOffset3D = FVector(RotationMatrix.TransformPosition(offset3D)) + FVector(halfLocalSize, 0);

				widgets.Add(Widget);
				widgetOffsets.Add(Widget, rotatedOffset3D);

				angleRad += angleRadStep;
			}
		}

		widgets.Sort([&widgetOffsets, this](const TSharedPtr<SWidget>& A, const TSharedPtr<SWidget>& B) { return FMath::Abs(widgetOffsets[A].Z - ProjectionZ) < FMath::Abs(widgetOffsets[B].Z - ProjectionZ); });

		for(const TSharedPtr<SWidget>& widget : widgets)
		{
			const FVector2D DesiredSizeOfSlot = widget->GetDesiredSize();

			FVector2D offset;
			offset.X = -DesiredSizeOfSlot.X * Slots[widgetSlots[widget]].GetPivot().X + halfLocalSize.X + (widgetOffsets[widget].X - halfLocalSize.X) * (1 + (ProjectionZ - widgetOffsets[widget].Z) / (widgetOffsets[widget].Z - FocusZ));
			offset.Y = -DesiredSizeOfSlot.Y * Slots[widgetSlots[widget]].GetPivot().Y + halfLocalSize.Y + (widgetOffsets[widget].Y - halfLocalSize.Y) * (1 + (ProjectionZ - widgetOffsets[widget].Z) / (widgetOffsets[widget].Z - FocusZ));

			ArrangedChildren.AddWidget(AllottedGeometry.MakeChild(widget.ToSharedRef(), offset, widget->GetDesiredSize()));
		}
	}
}

int32 SCircularPanel3D::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	if (bDrawDebugEllipse)
	{
		const FVector2D halfLocalSize = 0.5 * AllottedGeometry.GetLocalSize();

		const ESlateDrawEffect DrawEffects = IsEnabled() ? ESlateDrawEffect::None : ESlateDrawEffect::DisabledEffect;

		const int32 LinePointsNum = 32;

		TArray<FVector2D> LinePoints;
		{
			LinePoints.AddUninitialized(LinePointsNum + 1);

			const float angleRadStep = UE_TWO_PI / LinePointsNum;
			float angleRad = 0;

			for (size_t i = 0; i < LinePointsNum; i++)
			{
				const FVector offset3D(
					halfLocalSize.X * FMath::Cos(angleRad) * RadiusA,
					halfLocalSize.Y * FMath::Sin(angleRad) * RadiusB, 0);

				const FVector rotatedOffset3D = FVector(RotationMatrix.TransformPosition(offset3D)) + FVector(halfLocalSize, 0);

				LinePoints[i].X = halfLocalSize.X + (rotatedOffset3D.X - halfLocalSize.X) * (1 + (ProjectionZ - rotatedOffset3D.Z) / (rotatedOffset3D.Z - FocusZ));
				LinePoints[i].Y = halfLocalSize.Y + (rotatedOffset3D.Y - halfLocalSize.Y) * (1 + (ProjectionZ - rotatedOffset3D.Z) / (rotatedOffset3D.Z - FocusZ));

				angleRad += angleRadStep;
			}

			LinePoints[LinePointsNum] = LinePoints[0];
		}

		FSlateDrawElement::MakeLines(
			OutDrawElements,
			LayerId++,
			AllottedGeometry.ToPaintGeometry(),
			LinePoints,
			DrawEffects,
			FLinearColor(1.0f, 1.0f, 1.0f, 0.5f),
			false
		);
	}

	return SPanel::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
}

void SCircularPanel3D::CalculateRotationMatrix()
{
	const float a = FMath::DegreesToRadians(Alpha);
	const float b = FMath::DegreesToRadians(Betta);

	const float cosA = FMath::Cos(a);
	const float sinA = FMath::Sin(a);
	const float cosB = FMath::Cos(b);
	const float sinB = FMath::Sin(b);

	RotationMatrix.M[0][0] = cosB;
	RotationMatrix.M[0][1] = 0;
	RotationMatrix.M[0][2] = sinB;
	RotationMatrix.M[0][3] = 0;

	RotationMatrix.M[1][0] = sinA * sinB;
	RotationMatrix.M[1][1] = cosA;
	RotationMatrix.M[1][2] = -sinA * cosB;
	RotationMatrix.M[1][3] = 0;

	RotationMatrix.M[2][0] = -cosA * sinB;
	RotationMatrix.M[2][1] = sinA;
	RotationMatrix.M[2][2] = cosA * cosB;
	RotationMatrix.M[2][3] = 0;

	RotationMatrix.M[3][0] = 0;
	RotationMatrix.M[3][1] = 0;
	RotationMatrix.M[3][2] = 0;
	RotationMatrix.M[3][3] = 0;
}



UCircularPanel3DSlot::UCircularPanel3DSlot(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer), Slot(nullptr)
{
	PRAGMA_DISABLE_DEPRECATION_WARNINGS

	Pivot = FVector2D(0.5f);

	PRAGMA_ENABLE_DEPRECATION_WARNINGS
}

FVector2D UCircularPanel3DSlot::GetPivot() const
{
	PRAGMA_DISABLE_DEPRECATION_WARNINGS

	return Slot ? Slot->GetPivot() : Pivot;

	PRAGMA_ENABLE_DEPRECATION_WARNINGS
}

void UCircularPanel3DSlot::SetPivot(FVector2D InPivot)
{
	PRAGMA_DISABLE_DEPRECATION_WARNINGS

	Pivot = InPivot;

	if (Slot)
	{
		Slot->SetPivot(InPivot);
	}

	PRAGMA_ENABLE_DEPRECATION_WARNINGS
}

void UCircularPanel3DSlot::SynchronizeProperties()
{
	PRAGMA_DISABLE_DEPRECATION_WARNINGS

	SetPivot(Pivot);

	PRAGMA_ENABLE_DEPRECATION_WARNINGS
}

void UCircularPanel3DSlot::BuildSlot(TSharedRef<SCircularPanel3D> InCircularPanel)
{
	PRAGMA_DISABLE_DEPRECATION_WARNINGS

	InCircularPanel->AddSlot().Expose(Slot).Pivot(Pivot)
	[
		Content == nullptr ? SNullWidget::NullWidget : Content->TakeWidget()
	];

	PRAGMA_ENABLE_DEPRECATION_WARNINGS
}

void UCircularPanel3DSlot::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	Slot = nullptr;
}



UCircularPanel3D::UCircularPanel3D(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bIsVariable = false;
	SetVisibilityInternal(ESlateVisibility::SelfHitTestInvisible);

	RadiusA = 0.5;
	RadiusB = 0.5;
	Alpha = 0; 
	Betta = 0; 
	Angle = 0;
	FocusZ = -100000;
	ProjectionZ = 4000;
	bDrawDebugEllipse = 1;
}

void UCircularPanel3D::SetRadiusA(float radiusA)
{
	RadiusA = radiusA;

	if (MyCircularPanel.IsValid())
	{
		MyCircularPanel->SetRadiusA(radiusA);
	}
}

void UCircularPanel3D::SetRadiusB(float radiusB)
{
	RadiusB = radiusB;

	if (MyCircularPanel.IsValid())
	{
		MyCircularPanel->SetRadiusB(radiusB);
	}
}

void UCircularPanel3D::SetAlpha(float alpha)
{
	Alpha = alpha;

	if (MyCircularPanel.IsValid())
	{
		MyCircularPanel->SetAlpha(alpha);
	}
}

void UCircularPanel3D::SetBetta(float betta)
{
	Betta = betta;

	if (MyCircularPanel.IsValid())
	{
		MyCircularPanel->SetBetta(betta);
	}
}

void UCircularPanel3D::SetAngle(float angle)
{
	Angle = angle;

	if (MyCircularPanel.IsValid())
	{
		MyCircularPanel->SetAngle(angle);
	}
}

void UCircularPanel3D::SetFocusZ(float focusZ)
{
	FocusZ = focusZ;

	if (MyCircularPanel.IsValid())
	{
		MyCircularPanel->SetFocusZ(focusZ);
	}
}

void UCircularPanel3D::SetProjectionZ(float projectionZ)
{
	ProjectionZ = projectionZ;

	if (MyCircularPanel.IsValid())
	{
		MyCircularPanel->SetProjectionZ(projectionZ);
	}
}

void UCircularPanel3D::SetDrawDebugEllipse(bool drawDebugEllipse)
{
	bDrawDebugEllipse = drawDebugEllipse;

	if (MyCircularPanel.IsValid())
	{
		MyCircularPanel->SetDrawDebugEllipse(drawDebugEllipse);
	}
}

UCircularPanel3DSlot* UCircularPanel3D::AddChildToCircularPanel3D(UWidget* Content) { return Cast<UCircularPanel3DSlot>(Super::AddChild(Content)); }

void UCircularPanel3D::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	MyCircularPanel->SetRadiusA(RadiusA);
	MyCircularPanel->SetRadiusB(RadiusB);
	MyCircularPanel->SetAlpha(Alpha); 
	MyCircularPanel->SetBetta(Betta); 
	MyCircularPanel->SetAngle(Angle);
	MyCircularPanel->SetFocusZ(FocusZ);
	MyCircularPanel->SetProjectionZ(ProjectionZ);
	MyCircularPanel->SetDrawDebugEllipse(bDrawDebugEllipse);
}

void UCircularPanel3D::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	MyCircularPanel.Reset();
}

#if WITH_EDITOR

const FText UCircularPanel3D::GetPaletteCategory()
{
	return LOCTEXT("Panel", "Panel");
}

#endif

UClass* UCircularPanel3D::GetSlotClass() const { return UCircularPanel3DSlot::StaticClass(); }

void UCircularPanel3D::OnSlotAdded(UPanelSlot* InSlot)
{
	// Add the child to the live canvas if it already exists
	if (MyCircularPanel.IsValid())
	{
		CastChecked<UCircularPanel3DSlot>(InSlot)->BuildSlot(MyCircularPanel.ToSharedRef());
	}
}

void UCircularPanel3D::OnSlotRemoved(UPanelSlot* InSlot)
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

TSharedRef<SWidget> UCircularPanel3D::RebuildWidget()
{
	MyCircularPanel = SNew(SCircularPanel3D);

	for (UPanelSlot* PanelSlot : Slots)
	{
		if (UCircularPanel3DSlot* TypedSlot = Cast<UCircularPanel3DSlot>(PanelSlot))
		{
			TypedSlot->Parent = this;
			TypedSlot->BuildSlot(MyCircularPanel.ToSharedRef());
		}
	}

	return MyCircularPanel.ToSharedRef();
}

#undef LOCTEXT_NAMESPACE