// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Widgets/SPanel.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "SlotBase.h"
#include "Layout/Children.h"
#include "Components/PanelSlot.h"
#include "Components/PanelWidget.h"
#include "CircularPanel.generated.h"

class SCircularPanel : public SPanel
{
public:

	class FSlot : public TSlotBase<FSlot>
	{
	public:
		using TSlotBase<FSlot>::TSlotBase;

		SLATE_SLOT_BEGIN_ARGS(FSlot, TSlotBase<FSlot>)

			SLATE_ATTRIBUTE(FVector2D, Pivot)

		SLATE_SLOT_END_ARGS()

			FSlot()
			: TSlotBase<FSlot>()
			, Pivot(FVector2D(0.5, 0.5))
		{}

		void Construct(const FChildren& SlotOwner, FSlotArguments&& InArg)
		{
			TSlotBase<FSlot>::Construct(SlotOwner, MoveTemp(InArg));
			
			if (InArg._Pivot.IsSet())
			{
				SetPivot(InArg._Pivot.Get());
			}
		}

		void SetPivot(const FVector2D& Value) { Pivot = Value; }
		const FVector2D& GetPivot() const { return Pivot; }

		float RadiusA;
		float RadiusB;
		float SelectionAlpha;

	protected:

		FVector2D Pivot;
	};

	SLATE_BEGIN_ARGS(SCircularPanel)
		: _RadiusA(0)
		, _RadiusB(0)
		, _Angle(0)
	{
		_Visibility = EVisibility::SelfHitTestInvisible;
	}

	SLATE_SLOT_ARGUMENT(FSlot, Slots)

		SLATE_ATTRIBUTE(float, RadiusA)

		SLATE_ATTRIBUTE(float, RadiusB)

		SLATE_ATTRIBUTE(float, Angle)

	SLATE_END_ARGS()

	void SetRadiusA(const TAttribute<float>& radiusA) { RadiusA = radiusA.Get(0); }

	void SetRadiusB(const TAttribute<float>& radiusB) { RadiusB = radiusB.Get(0); }

	void SetAngle(const TAttribute<float>& angle) { Angle = angle.Get(0); }

	SCircularPanel() : Slots(this) {}

	void Construct(const FArguments& InArgs);

	using FScopedWidgetSlotArguments = TPanelChildren<FSlot>::FScopedWidgetSlotArguments;

	static FSlot::FSlotArguments Slot() { return FSlot::FSlotArguments(MakeUnique<FSlot>()); }
	FScopedWidgetSlotArguments AddSlot() { return FScopedWidgetSlotArguments{ MakeUnique<FSlot>(), Slots, INDEX_NONE }; }
	int32 RemoveSlot(const TSharedRef<SWidget>& SlotWidget) { return Slots.Remove(SlotWidget); }

	virtual void OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const override;
	virtual FVector2D ComputeDesiredSize(float) const override { return FVector2D::ZeroVector; }
	virtual FChildren* GetChildren() override { return &Slots; }

protected:

	TPanelChildren<FSlot> Slots;

	float RadiusA;
	float RadiusB;
	float Angle;
};

UCLASS()
class WHIRLPANELS_API UCircularPanelSlot : public UPanelSlot
{
	GENERATED_UCLASS_BODY()

private:

	SCircularPanel::FSlot* Slot;

public:

	UE_DEPRECATED(5.1, "Direct access to Pivot is deprecated. Please use the getter or setter.")
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Getter, Setter, BlueprintSetter = "SetPivot", Category = "Layout|Circular Panel Slot")
		FVector2D Pivot;

public:

	FVector2D GetPivot() const;

	UFUNCTION(BlueprintCallable, Category = "Layout|Border Slot")
		void SetPivot(FVector2D InPivot);

	virtual void SynchronizeProperties() override;

	virtual void BuildSlot(TSharedRef<SCircularPanel> InCircularPanel);

	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
};

UCLASS()
class WHIRLPANELS_API UCircularPanel : public UPanelWidget
{
	GENERATED_UCLASS_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Widget")
		void SetRadiusA(float radiusA);

	UFUNCTION(BlueprintCallable, Category = "Widget")
		void SetRadiusB(float radiusB);

	UFUNCTION(BlueprintCallable, Category = "Widget")
		void SetAngle(float angle);

	UFUNCTION(BlueprintCallable, Category = "Widget")
		UCircularPanelSlot* AddChildToCircularPanel(UWidget* Content);

	virtual void SynchronizeProperties() override;

	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

#if WITH_EDITOR
	virtual const FText GetPaletteCategory() override;
#endif

public:

	virtual UClass* GetSlotClass() const override;

	virtual void OnSlotAdded(UPanelSlot* Slot) override;

	virtual void OnSlotRemoved(UPanelSlot* Slot) override;

protected:

	virtual TSharedRef<SWidget> RebuildWidget() override;

protected:

	TSharedPtr<SCircularPanel> MyCircularPanel;

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Circular Panel", meta = (UIMin = 0, UIMax = 8, ClampMin = 0, ClampMax = 8))
		float RadiusA;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Circular Panel", meta = (UIMin = 0, UIMax = 8, ClampMin = 0, ClampMax = 8))
		float RadiusB;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Circular Panel")
		float Angle;
};