// Copyright 2023 Pentangle Studio Licensed under the Unlicense License (the «LICENSE»);

#pragma once

#include "Widgets/SPanel.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "SlotBase.h"
#include "Layout/Children.h"
#include "Components/PanelSlot.h"
#include "Components/PanelWidget.h"
#include "SpiralPanel.generated.h"

class SSpiralPanel : public SPanel
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

	protected:

		FVector2D Pivot;
	};

	SLATE_BEGIN_ARGS(SSpiralPanel)
		: _Radius(0)
		, _RadiusStep(0)
		, _Angle(0)
		, _AngleStep(0)
	{
		_Visibility = EVisibility::SelfHitTestInvisible;
	}

	SLATE_SLOT_ARGUMENT(FSlot, Slots)

		SLATE_ATTRIBUTE(float, Radius)

		SLATE_ATTRIBUTE(float, RadiusStep)

		SLATE_ATTRIBUTE(float, Angle)

		SLATE_ATTRIBUTE(float, AngleStep)

	SLATE_END_ARGS()

	void SetRadius(const TAttribute<float>& radius) { Radius = radius.Get(0); }

	void SetRadiusStep(const TAttribute<float>& radiusStep) { RadiusStep = radiusStep.Get(0); }

	void SetAngle(const TAttribute<float>& angle) { Angle = angle.Get(0); }

	void SetAngleStep(const TAttribute<float>& angleStep) { AngleStep = angleStep.Get(0); }

	SSpiralPanel() : Slots(this) {}

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

	float Radius;
	float RadiusStep;
	float Angle;
	float AngleStep;
};

UCLASS()
class WHIRLPANELS_API USpiralPanelSlot : public UPanelSlot
{
	GENERATED_UCLASS_BODY()

private:

	SSpiralPanel::FSlot* Slot;

public:

	UE_DEPRECATED(5.0, "Direct access to OffsetAttr is now deprecated. Use the getter or setter.")
		FVector2D Pivot;

public:

	FVector2D GetPivot() const;

	UFUNCTION(BlueprintCallable, Category = "Layout|Border Slot")
		void SetPivot(FVector2D InPivot);

	virtual void SynchronizeProperties() override;

	virtual void BuildSlot(TSharedRef<SSpiralPanel> InSpiralPanel);

	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
};

UCLASS()
class WHIRLPANELS_API USpiralPanel : public UPanelWidget
{
	GENERATED_UCLASS_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Widget")
		void SetRadius(float radius);

	UFUNCTION(BlueprintCallable, Category = "Widget")
		void SetRadiusStep(float radiusStep);

	UFUNCTION(BlueprintCallable, Category = "Widget")
		void SetAngle(float angle);

	UFUNCTION(BlueprintCallable, Category = "Widget")
		void SetAngleStep(float angleStep);

	UFUNCTION(BlueprintCallable, Category = "Widget")
		USpiralPanelSlot* AddChildToSpiralPanel(UWidget* Content);

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

	TSharedPtr<SSpiralPanel> MySpiralPanel;

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Circular Panel", meta = (UIMin = 0, UIMax = 8, ClampMin = 0, ClampMax = 8))
		float Radius;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Circular Panel", meta = (UIMin = 0, UIMax = 8, ClampMin = 0, ClampMax = 8))
		float RadiusStep;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Circular Panel")
		float Angle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Circular Panel")
		float AngleStep;
};