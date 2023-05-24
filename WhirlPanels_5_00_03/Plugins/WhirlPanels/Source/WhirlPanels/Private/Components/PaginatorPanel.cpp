// Copyright 2023 Pentangle Studio Licensed under the Unlicense License (the «LICENSE»);

#pragma once

#include "Components/PaginatorPanel.h"
#include "Layout/ArrangedChildren.h"
#include "Layout/LayoutUtils.h"

#define LOCTEXT_NAMESPACE "WHIRLPANELS_SPaginatorPanel"

float SPaginatorPanel::RecalcTarget(const TAttribute<int32>& deltaTargetIndex)
{
	if (TargetAlpha == 0)
	{
		TargetIndex = TargetIndex + deltaTargetIndex.Get(0);
	}
	else
	{
		const int newTargetIndex = TargetIndex + deltaTargetIndex.Get(0);
		float newTargetAlpha = (TargetIndex - StartIndex) * TargetAlpha / (TargetIndex - StartIndex + deltaTargetIndex.Get(0));

		TargetIndex = newTargetIndex;
		TargetAlpha = newTargetAlpha;
	}

	return TargetAlpha;
}

void SPaginatorPanel::Construct(const FArguments& InArgs)
{
	Slots.AddSlots(MoveTemp(const_cast<TArray<FSlot::FSlotArguments>&>(InArgs._Slots)));
	SetStartIndex(InArgs._StartIndex);
	SetItemCount(InArgs._ItemCount);
	SetItemSize(InArgs._ItemSize);
	SetDirection(InArgs._Direction);
	SetTargetIndex(InArgs._TargetIndex);
	SetTargetAlpha(InArgs._TargetAlpha);
}

void SPaginatorPanel::OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const
{
	const float deltaIndex = (TargetIndex - StartIndex) * TargetAlpha;
	const float mixedIndex = StartIndex + deltaIndex;
	const int startIndex = FMath::FloorToInt32(mixedIndex);
	const int endIndex = FMath::CeilToInt32(mixedIndex) + ItemCount;

	const FVector2D directionStep = Direction * ItemSize;

	for (int32 SlotIndex = 0; SlotIndex < Slots.Num(); ++SlotIndex)
	{
		if (SlotIndex >= startIndex && SlotIndex < endIndex)
		{
			const FSlot& CurSlot = Slots[SlotIndex];
			const EVisibility ChildVisibility = CurSlot.GetWidget()->GetVisibility();
			if (ArrangedChildren.Accepts(ChildVisibility))
			{
				const FMargin SlotPadding(CurSlot.GetPadding());
				AlignmentArrangeResult XAxisResult = AlignChild<Orient_Horizontal>(ItemSize.X, CurSlot, SlotPadding);
				AlignmentArrangeResult YAxisResult = AlignChild<Orient_Vertical>(ItemSize.Y, CurSlot, SlotPadding);

				ArrangedChildren.AddWidget(ChildVisibility, AllottedGeometry.MakeChild(
					CurSlot.GetWidget(),
					(SlotIndex - StartIndex - deltaIndex) * directionStep + FVector2D(SlotPadding.Left, SlotPadding.Top),
					FVector2D(XAxisResult.Size, YAxisResult.Size)
				));
			}
		}
	}
}

UPaginatorPanelSlot::UPaginatorPanelSlot(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	Slot = nullptr;

	PRAGMA_DISABLE_DEPRECATION_WARNINGS

	Padding = FMargin();
	HorizontalAlignment = HAlign_Fill;
	VerticalAlignment = VAlign_Fill;

	PRAGMA_ENABLE_DEPRECATION_WARNINGS
}

FMargin UPaginatorPanelSlot::GetPadding() const
{
	PRAGMA_DISABLE_DEPRECATION_WARNINGS

	return Slot ? Slot->GetPadding() : Padding;

	PRAGMA_ENABLE_DEPRECATION_WARNINGS
}

void UPaginatorPanelSlot::SetPadding(FMargin InPadding)
{
	PRAGMA_DISABLE_DEPRECATION_WARNINGS

	Padding = InPadding;
	if (Slot)
	{
		Slot->SetPadding(InPadding);
	}

	PRAGMA_ENABLE_DEPRECATION_WARNINGS
}

EHorizontalAlignment UPaginatorPanelSlot::GetHorizontalAlignment() const
{ 
	PRAGMA_DISABLE_DEPRECATION_WARNINGS

	return Slot ? Slot->GetHorizontalAlignment() : HorizontalAlignment.GetValue();

	PRAGMA_ENABLE_DEPRECATION_WARNINGS
}

void UPaginatorPanelSlot::SetHorizontalAlignment(EHorizontalAlignment InHorizontalAlignment)
{
	PRAGMA_DISABLE_DEPRECATION_WARNINGS

	HorizontalAlignment = InHorizontalAlignment;
	if (Slot)
	{
		Slot->SetHorizontalAlignment(InHorizontalAlignment);
	}

	PRAGMA_ENABLE_DEPRECATION_WARNINGS
}

EVerticalAlignment UPaginatorPanelSlot::GetVerticalAlignment() const
{	
	PRAGMA_DISABLE_DEPRECATION_WARNINGS

	return Slot ? Slot->GetVerticalAlignment() : VerticalAlignment.GetValue();

	PRAGMA_ENABLE_DEPRECATION_WARNINGS
}

void UPaginatorPanelSlot::SetVerticalAlignment(EVerticalAlignment InVerticalAlignment)
{
	PRAGMA_DISABLE_DEPRECATION_WARNINGS

	VerticalAlignment = InVerticalAlignment;
	if (Slot)
	{
		Slot->SetVerticalAlignment(InVerticalAlignment);
	}

	PRAGMA_ENABLE_DEPRECATION_WARNINGS
}

void UPaginatorPanelSlot::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	PRAGMA_DISABLE_DEPRECATION_WARNINGS

	SetHorizontalAlignment(HorizontalAlignment);
	SetVerticalAlignment(VerticalAlignment);
	SetPadding(Padding);

	PRAGMA_ENABLE_DEPRECATION_WARNINGS
}

void UPaginatorPanelSlot::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	Slot = nullptr;
}

void UPaginatorPanelSlot::BuildSlot(TSharedRef<SPaginatorPanel> InPaginatorPanel)
{
	PRAGMA_DISABLE_DEPRECATION_WARNINGS

	InPaginatorPanel->AddSlot()
		.Expose(Slot)
		.Padding(Padding)
		.HAlign(HorizontalAlignment)
		.VAlign(VerticalAlignment)
		[
			Content == nullptr ? SNullWidget::NullWidget : Content->TakeWidget()
		];

	PRAGMA_ENABLE_DEPRECATION_WARNINGS
}

UPaginatorPanelWidget::UPaginatorPanelWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bIsVariable = false;
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	StartIndex = 0;
	ItemCount = 5;
	ItemSize = FIntPoint(128, 128);
	Direction = FVector2D(1, 0);
	TargetIndex = 0;
	TargetAlpha = 0;
}

void UPaginatorPanelWidget::SetStartIndex(int32 startIndex) { if (MyPaginatorPanel.IsValid()) MyPaginatorPanel->SetStartIndex(StartIndex = startIndex); }

void UPaginatorPanelWidget::SetItemCount(int32 itemCount) { if (MyPaginatorPanel.IsValid()) MyPaginatorPanel->SetItemCount(ItemCount = itemCount); }

void UPaginatorPanelWidget::SetItemSize(const FIntPoint& itemSize) { if (MyPaginatorPanel.IsValid()) MyPaginatorPanel->SetItemSize(ItemSize = itemSize); }

void UPaginatorPanelWidget::SetDirection(const FVector2D& direction) { if (MyPaginatorPanel.IsValid()) MyPaginatorPanel->SetDirection(Direction = direction); }

void UPaginatorPanelWidget::SetTargetIndex(int32 targetIndex) { if (MyPaginatorPanel.IsValid()) MyPaginatorPanel->SetTargetIndex(TargetIndex = targetIndex); }

void UPaginatorPanelWidget::SetTargetAlpha(float targetAlpha) { if (MyPaginatorPanel.IsValid()) MyPaginatorPanel->SetTargetAlpha(TargetAlpha = targetAlpha); }

float UPaginatorPanelWidget::RecalcTarget(int32 deltaTargetIndex)
{
	float recalcedTargetAlpha = MyPaginatorPanel.IsValid() ? MyPaginatorPanel->RecalcTarget(deltaTargetIndex) : INDEX_NONE;
	TargetIndex = MyPaginatorPanel.IsValid() ? MyPaginatorPanel->GetTargetIndex() : TargetIndex;
	return recalcedTargetAlpha;
}

void UPaginatorPanelWidget::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	if (MyPaginatorPanel.IsValid())
	{
		MyPaginatorPanel->SetStartIndex(StartIndex);

		MyPaginatorPanel->SetItemCount(ItemCount);

		MyPaginatorPanel->SetItemSize(ItemSize);

		MyPaginatorPanel->SetDirection(Direction);

		MyPaginatorPanel->SetTargetIndex(TargetIndex);

		MyPaginatorPanel->SetTargetAlpha(TargetAlpha);
	}
}

void UPaginatorPanelWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	MyPaginatorPanel.Reset();
}

UPaginatorPanelSlot* UPaginatorPanelWidget::AddChildToPaginatorPanel(UWidget* Content) { return Cast<UPaginatorPanelSlot>(Super::AddChild(Content)); }

UClass* UPaginatorPanelWidget::GetSlotClass() const { return UPaginatorPanelSlot::StaticClass(); }

void UPaginatorPanelWidget::OnSlotAdded(UPanelSlot* InSlot)
{
	// Add the child to the live canvas if it already exists
	if (MyPaginatorPanel.IsValid())
	{
		CastChecked<UPaginatorPanelSlot>(InSlot)->BuildSlot(MyPaginatorPanel.ToSharedRef());
	}
}

void UPaginatorPanelWidget::OnSlotRemoved(UPanelSlot* InSlot)
{
	// Remove the widget from the live slot if it exists.
	if (MyPaginatorPanel.IsValid() && InSlot->Content)
	{
		TSharedPtr<SWidget> Widget = InSlot->Content->GetCachedWidget();
		if (Widget.IsValid())
		{
			MyPaginatorPanel->RemoveSlot(Widget.ToSharedRef());
		}
	}
}

TSharedRef<SWidget> UPaginatorPanelWidget::RebuildWidget()
{
	MyPaginatorPanel = SNew(SPaginatorPanel)
		.StartIndex(StartIndex)
		.ItemCount(ItemCount)
		.ItemSize(ItemSize)
		.Direction(Direction)
		.TargetIndex(TargetIndex)
		.TargetAlpha(TargetAlpha);

	for (UPanelSlot* PanelSlot : Slots)
	{
		if (UPaginatorPanelSlot* TypedSlot = Cast<UPaginatorPanelSlot>(PanelSlot))
		{
			TypedSlot->Parent = this;
			TypedSlot->BuildSlot(MyPaginatorPanel.ToSharedRef());
		}
	}

	return MyPaginatorPanel.ToSharedRef();
}

#if WITH_EDITOR

const FText UPaginatorPanelWidget::GetPaletteCategory()
{
	return LOCTEXT("Panel", "Panel");
}

#endif

#undef LOCTEXT_NAMESPACE