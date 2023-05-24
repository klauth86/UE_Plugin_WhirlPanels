// Copyright 2023 Pentangle Studio Licensed under the Unlicense License (the «LICENSE»);

#pragma once

#include "Components/PanelWidget.h"
#include "Components/PanelSlot.h"
#include "Widgets/SPanel.h"
#include "SlotBase.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Layout/Children.h"
#include "PaginatorPanel.generated.h"

class SPaginatorPanel : public SPanel
{
public:

	class FSlot : public TBasicLayoutWidgetSlot<FSlot>
	{
	public:

		FSlot()
			: TBasicLayoutWidgetSlot<FSlot>(HAlign_Fill, VAlign_Fill)
		{}

		SLATE_SLOT_BEGIN_ARGS(FSlot, TBasicLayoutWidgetSlot<FSlot>)
			SLATE_SLOT_END_ARGS()

			void Construct(const FChildren& SlotOwner, FSlotArguments&& InArg)
		{
			TBasicLayoutWidgetSlot<FSlot>::Construct(SlotOwner, MoveTemp(InArg));
		}
	};

	SLATE_BEGIN_ARGS(SPaginatorPanel)
		: _StartIndex(0)
		, _ItemCount(5)
		, _ItemSize(FIntPoint(128, 128))
		, _Direction(FVector2D(1, 0))
		, _TargetIndex(0)
		, _TargetAlpha(0.f)
	{
		_Visibility = EVisibility::SelfHitTestInvisible;
	}

	SLATE_SLOT_ARGUMENT(FSlot, Slots)

		SLATE_ATTRIBUTE(int32, StartIndex)

		SLATE_ATTRIBUTE(int32, ItemCount)

		SLATE_ATTRIBUTE(FIntPoint, ItemSize)

		SLATE_ATTRIBUTE(FVector2D, Direction)

		SLATE_ATTRIBUTE(int32, TargetIndex)

		SLATE_ATTRIBUTE(float, TargetAlpha)

		SLATE_END_ARGS()

		void SetStartIndex(const TAttribute<int32>& startIndex) { StartIndex = startIndex.Get(0); }

	void SetItemCount(const TAttribute<int32>& itemCount) { ItemCount = itemCount.Get(0); }

	void SetItemSize(const TAttribute<FIntPoint>& itemSize) { ItemSize = itemSize.Get(FIntPoint(128, 128)); }

	void SetDirection(const TAttribute<FVector2D>& direction) { Direction = direction.Get(FVector2D(1, 0)); }

	int32 GetTargetIndex() const { return TargetIndex; }

	void SetTargetIndex(const TAttribute<int32>& targetIndex) { TargetIndex = targetIndex.Get(0); }

	void SetTargetAlpha(const TAttribute<float>& targetAlpha) { TargetAlpha = targetAlpha.Get(0.f); }

	float RecalcTarget(const TAttribute<int32>& deltaTargetIndex);

	SPaginatorPanel() : Slots(this), StartIndex(0), ItemCount(5), ItemSize(FIntPoint(128, 128)), Direction(FVector2D(1, 0)), TargetIndex(0), TargetAlpha(0.f) {}

	void Construct(const FArguments& InArgs);

	using FScopedWidgetSlotArguments = TPanelChildren<FSlot>::FScopedWidgetSlotArguments;

	static FSlot::FSlotArguments Slot() { return FSlot::FSlotArguments(MakeUnique<FSlot>()); }
	FScopedWidgetSlotArguments AddSlot() { return FScopedWidgetSlotArguments{ MakeUnique<FSlot>(), Slots, INDEX_NONE }; }
	int32 RemoveSlot(const TSharedRef<SWidget>& SlotWidget) { return Slots.Remove(SlotWidget); }

	virtual void OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const override;
	virtual FVector2D ComputeDesiredSize(float) const override { return ItemCount > 0 ? FVector2D(Direction.X * (ItemCount - 1) + 1, Direction.Y * (ItemCount - 1) + 1) * ItemSize : FVector2D::ZeroVector; }
	virtual FChildren* GetChildren() override { return &Slots; }

protected:

	TPanelChildren<FSlot> Slots;
	int32 StartIndex;
	int32 ItemCount;
	FIntPoint ItemSize;
	FVector2D Direction;
	int32 TargetIndex;
	float TargetAlpha;
};

UCLASS()
class WHIRLPANELS_API UPaginatorPanelSlot : public UPanelSlot
{
	GENERATED_UCLASS_BODY()

protected:

	SPaginatorPanel::FSlot* Slot;

public:

	FMargin GetPadding() const;

	UFUNCTION(BlueprintCallable, Category = "Layout|Border Slot")
		void SetPadding(FMargin InPadding);

	EHorizontalAlignment GetHorizontalAlignment() const;

	UFUNCTION(BlueprintCallable, Category = "Layout|Grid Slot")
		void SetHorizontalAlignment(EHorizontalAlignment InHorizontalAlignment);

	EVerticalAlignment GetVerticalAlignment() const;

	UFUNCTION(BlueprintCallable, Category = "Layout|Grid Slot")
		void SetVerticalAlignment(EVerticalAlignment InVerticalAlignment);

	virtual void SynchronizeProperties() override;

	virtual void BuildSlot(TSharedRef<SPaginatorPanel> InPaginatorPanel);

	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

protected:

	/** The padding area between the slot and the content it contains. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Layout|Overlay Slot")
		FMargin Padding;

	/** The alignment of the object horizontally. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Layout|Overlay Slot")
		TEnumAsByte<EHorizontalAlignment> HorizontalAlignment;

	/** The alignment of the object vertically. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Layout|Overlay Slot")
		TEnumAsByte<EVerticalAlignment> VerticalAlignment;
};

UCLASS()
class WHIRLPANELS_API UPaginatorPanelWidget : public UPanelWidget
{
	GENERATED_UCLASS_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Widget")
		void SetStartIndex(int32 startIndex);

	UFUNCTION(BlueprintCallable, Category = "Widget")
		void SetItemCount(int32 itemCount);

	UFUNCTION(BlueprintCallable, Category = "Widget")
		void SetItemSize(const FIntPoint& itemSize);

	UFUNCTION(BlueprintCallable, Category = "Widget")
		void SetDirection(const FVector2D& direction);

	UFUNCTION(BlueprintCallable, Category = "Widget")
		void SetTargetIndex(int32 targetIndex);

	UFUNCTION(BlueprintCallable, Category = "Widget")
		void SetTargetAlpha(float targetAlpha);

	UFUNCTION(BlueprintCallable, Category = "Widget")
		float RecalcTarget(int32 deltaTargetIndex);

	virtual void SynchronizeProperties() override;

	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

	UFUNCTION(BlueprintCallable, Category = "Widget")
		UPaginatorPanelSlot* AddChildToPaginatorPanel(UWidget* Content);

#if WITH_EDITOR
	virtual const FText GetPaletteCategory() override;
#endif

protected:

	// UPanelWidget
	virtual UClass* GetSlotClass() const override;
	virtual void OnSlotAdded(UPanelSlot* Slot) override;
	virtual void OnSlotRemoved(UPanelSlot* Slot) override;
	// End UPanelWidget

protected:

	TSharedPtr<SPaginatorPanel> MyPaginatorPanel;

protected:
	// UWidget interface
	virtual TSharedRef<SWidget> RebuildWidget() override;
	// End of UWidget interface

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Paginator Panel", meta = (UIMin = "0", ClampMin = "0"))
		int32 StartIndex;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Paginator Panel", meta = (UIMin = "0", ClampMin = "0"))
		int32 ItemCount;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Paginator Panel", meta = (UIMin = "0", ClampMin = "0"))
		FIntPoint ItemSize;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Paginator Panel", meta = (UIMin = "0", UIMax = "1", ClampMin = "0", ClampMax = "1"))
		FVector2D Direction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Paginator Panel")
		int32 TargetIndex;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Paginator Panel", meta = (UIMin = "0", UIMax = "1", ClampMin = "0", ClampMax = "1"))
		float TargetAlpha;
};