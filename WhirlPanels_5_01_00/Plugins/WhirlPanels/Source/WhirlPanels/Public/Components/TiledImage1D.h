// Copyright 2023 Pentangle Studio Licensed under the Unlicense License (the «LICENSE»);

#pragma once

#include "Widgets/SLeafWidget.h"
#include "Components/Widget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Styling/SlateBrush.h"
#include "Engine/StreamableManager.h"
#include "Styling/CoreStyle.h"
#include "TiledImage1D.generated.h"

class USlateBrushAsset;
class ISlateTextureAtlasInterface;
class UTexture2DDynamic;
struct FStreamableHandle;

class STiledImage1D : public SLeafWidget
{
public:

	SLATE_DECLARE_WIDGET(STiledImage1D, SLeafWidget)

	SLATE_BEGIN_ARGS(STiledImage1D)
		: _Image(FCoreStyle::Get().GetDefaultBrush())
		, _ColorAndOpacity(FLinearColor::White)
		, _InverseOrder(false)
		, _Direction(FVector2D(1, 0))
	{
		_Visibility = EVisibility::SelfHitTestInvisible;
	}
		SLATE_ATTRIBUTE(const FSlateBrush*, Image)

		SLATE_ATTRIBUTE(FSlateColor, ColorAndOpacity)

		SLATE_ATTRIBUTE(bool, InverseOrder)

		SLATE_ATTRIBUTE(FVector2D, Direction)

	SLATE_END_ARGS()

	void SetImage(TAttribute<const FSlateBrush*> InImage) { ImageAttribute.Assign(*this, MoveTemp(InImage)); }
	void SetColorAndOpacity(TAttribute<FSlateColor> InColorAndOpacity) { ColorAndOpacityAttribute.Assign(*this, MoveTemp(InColorAndOpacity)); }
	void SetColorAndOpacity(FLinearColor InColorAndOpacity) { ColorAndOpacityAttribute.Set(*this, InColorAndOpacity); }

	void SetInverseOrder(const TAttribute<bool>& inverseOrder) { InverseOrderAttribute.Assign(*this, inverseOrder.Get(false)); }
	void SetDirection(const TAttribute<FVector2D>& direction) { DirectionAttribute.Assign(*this, direction.Get(FVector2D(1, 0))); }

	STiledImage1D() : ImageAttribute(*this), ColorAndOpacityAttribute(*this), InverseOrderAttribute(*this, false), DirectionAttribute(*this, FVector2D(1, 0))
	{
		bHasCustomPrepass = true;

		SetCanTick(false);
		bCanSupportFocus = false;
	}

	void Construct(const FArguments& InArgs);

	virtual FVector2D ComputeDesiredSize(float) const override;

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

protected:

	virtual bool CustomPrepass(float LayoutScaleMultiplier) override;

	TSlateAttributeRef<const FSlateBrush*> GetImageAttribute() const { return TSlateAttributeRef<FSlateBrush const*>(SharedThis(this), ImageAttribute); }
	TSlateAttributeRef<FSlateColor> GetColorAndOpacityAttribute() const { return TSlateAttributeRef<FSlateColor>(SharedThis(this), ColorAndOpacityAttribute); }

	TSlateAttributeRef<bool> GetInverseOrderAttribute() const { return TSlateAttributeRef<bool>(SharedThis(this), InverseOrderAttribute); }
	TSlateAttributeRef<FVector2D> GetDirectionAttribute() const { return TSlateAttributeRef<FVector2D>(SharedThis(this), DirectionAttribute); }

protected:
	
	TSlateAttribute<const FSlateBrush*> ImageAttribute;
	TSlateAttribute<FSlateColor> ColorAndOpacityAttribute;

	TSlateAttribute<bool> InverseOrderAttribute;
	TSlateAttribute<FVector2D> DirectionAttribute;

	mutable TOptional<FVector2D> LastAllocatedArea;
	int32 TilingCount;
	FVector2D StepCorrection;
};

UCLASS()
class WHIRLPANELS_API UTiledImage1D : public UWidget
{
	GENERATED_UCLASS_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Appearance")
		void SetColorAndOpacity(FLinearColor InColorAndOpacity);

	UFUNCTION(BlueprintCallable, Category = "Appearance")
		void SetOpacity(float InOpacity);

	UFUNCTION(BlueprintCallable, Category = "Appearance")
		void SetBrushTintColor(FSlateColor TintColor);

	UFUNCTION(BlueprintCallable, Category = "Appearance")
		void SetBrushResourceObject(UObject* ResourceObject);

	UFUNCTION(BlueprintCallable, Category = "Appearance")
		virtual void SetBrush(const FSlateBrush& InBrush);

	UFUNCTION(BlueprintCallable, Category = "Appearance")
		virtual void SetBrushFromAsset(USlateBrushAsset* Asset);

	UFUNCTION(BlueprintCallable, Category = "Appearance")
		virtual void SetBrushFromTexture(UTexture2D* Texture, bool bMatchSize = false);

	UFUNCTION(BlueprintCallable, Category = "Appearance")
		virtual void SetBrushFromAtlasInterface(TScriptInterface<ISlateTextureAtlasInterface> AtlasRegion, bool bMatchSize = false);

	UFUNCTION(BlueprintCallable, Category = "Appearance")
		virtual void SetBrushFromTextureDynamic(UTexture2DDynamic* Texture, bool bMatchSize = false);

	UFUNCTION(BlueprintCallable, Category = "Appearance")
		virtual void SetBrushFromMaterial(UMaterialInterface* Material);

	UFUNCTION(BlueprintCallable, Category = "Appearance")
		virtual void SetBrushFromSoftTexture(TSoftObjectPtr<UTexture2D> SoftTexture, bool bMatchSize = false);

	UFUNCTION(BlueprintCallable, Category = "Appearance")
		virtual void SetBrushFromSoftMaterial(TSoftObjectPtr<UMaterialInterface> SoftMaterial);

	UFUNCTION(BlueprintCallable, Category = "Appearance")
		UMaterialInstanceDynamic* GetDynamicMaterial();

	UFUNCTION(BlueprintCallable, Category = "Widget")
		void SetInverseOrder(bool inverseOrder);

	UFUNCTION(BlueprintCallable, Category = "Widget")
		void SetDirection(const FVector2D& direction);

	virtual void SynchronizeProperties() override;

	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

#if WITH_EDITOR
	virtual const FText GetPaletteCategory() override;
#endif

protected:

	// UWidget interface
	virtual TSharedRef<SWidget> RebuildWidget() override;
	// End of UWidget interface

	const FSlateBrush* ConvertImage(TAttribute<FSlateBrush> InImageAsset) const;

	void RequestAsyncLoad(TSoftObjectPtr<UObject> SoftObject, TFunction<void()>&& Callback);

	virtual void RequestAsyncLoad(TSoftObjectPtr<UObject> SoftObject, FStreamableDelegate DelegateToCall);

	virtual void CancelImageStreaming();

	virtual void OnImageStreamingStarted(TSoftObjectPtr<UObject> SoftObject);

	virtual void OnImageStreamingComplete(TSoftObjectPtr<UObject> LoadedSoftObject);

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Appearance)
		FSlateBrush Brush;

	UPROPERTY()
		FGetSlateBrush BrushDelegate;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Appearance, meta = (sRGB = "true"))
		FLinearColor ColorAndOpacity;

	UPROPERTY()
		FGetLinearColor ColorAndOpacityDelegate;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Paginator Panel", meta = (UIMin = "0", ClampMin = "0"))
		bool InverseOrder;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Paginator Panel", meta = (UIMin = "0", UIMax = "1", ClampMin = "0", ClampMax = "1"))
		FVector2D Direction;

protected:

	TSharedPtr<STiledImage1D> MyTiledImage1D;

	TSharedPtr<FStreamableHandle> StreamingHandle;

	FSoftObjectPath StreamingObjectPath;

	PROPERTY_BINDING_IMPLEMENTATION(FSlateColor, ColorAndOpacity);
};