// Copyright 2023 Pentangle Studio Licensed under the Unlicense License (the «LICENSE»);

#pragma once

#include "Components/TiledImage1D.h"
#include "Slate/SlateBrushAsset.h"
#include "Engine/Texture2D.h"
#include "Engine/Texture2DDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Slate/SlateTextureAtlasInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"

#define LOCTEXT_NAMESPACE "WHIRLPANELS_STiledImage1D"

SLATE_IMPLEMENT_WIDGET(SImage)
void STiledImage1D::PrivateRegisterAttributes(FSlateAttributeInitializer& AttributeInitializer)
{
	SLATE_ADD_MEMBER_ATTRIBUTE_DEFINITION_WITH_NAME(AttributeInitializer, "Image", ImageAttribute, EInvalidateWidgetReason::Layout);
	SLATE_ADD_MEMBER_ATTRIBUTE_DEFINITION_WITH_NAME(AttributeInitializer, "ColorAndOpacity", ColorAndOpacityAttribute, EInvalidateWidgetReason::Paint);
	SLATE_ADD_MEMBER_ATTRIBUTE_DEFINITION_WITH_NAME(AttributeInitializer, "InverseOrder", InverseOrderAttribute, EInvalidateWidgetReason::Paint);
	SLATE_ADD_MEMBER_ATTRIBUTE_DEFINITION_WITH_NAME(AttributeInitializer, "Direction", DirectionAttribute, EInvalidateWidgetReason::Paint);
}

void STiledImage1D::Construct(const FArguments& InArgs)
{
	SetImage(InArgs._Image);
	SetColorAndOpacity(InArgs._ColorAndOpacity);
	SetInverseOrder(InArgs._InverseOrder);
	SetDirection(InArgs._Direction);

	LastAllocatedArea.Reset();
	TilingCount = 0;
	StepCorrection = FVector2D::ZeroVector;
}

FVector2D STiledImage1D::ComputeDesiredSize(float) const
{
	if (TilingCount > 0)
	{
		const FSlateBrush* imageBrush = ImageAttribute.Get();
		
		if (imageBrush && imageBrush->DrawAs == ESlateBrushDrawType::Image)
		{
			const FVector2D imageBrushSize = imageBrush->GetImageSize();
			const FVector2D direction(
				FMath::Max(1, TilingCount * (DirectionAttribute.Get().X + StepCorrection.X))
				, FMath::Max(1, TilingCount * (DirectionAttribute.Get().Y + StepCorrection.Y))
			);
			return imageBrushSize * direction;
		}
	}

	return FVector2D::ZeroVector;
}

int32 STiledImage1D::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	if (!LastAllocatedArea.IsSet() || !AllottedGeometry.GetLocalSize().Equals(LastAllocatedArea.GetValue()))
	{
		LastAllocatedArea = AllottedGeometry.GetLocalSize();

		const_cast<STiledImage1D*>(this)->Invalidate(EInvalidateWidgetReason::Prepass);
	}

	if (TilingCount > 0)
	{
		const FSlateBrush* imageBrush = ImageAttribute.Get();

		if (imageBrush && imageBrush->DrawAs == ESlateBrushDrawType::Image)
		{
			const FVector2D imageBrushSize = imageBrush->GetImageSize();
			const FVector2D step = (DirectionAttribute.Get() + StepCorrection) * imageBrushSize;

			const bool bIsEnabled = ShouldBeEnabled(bParentEnabled);
			const ESlateDrawEffect DrawEffects = bIsEnabled ? ESlateDrawEffect::None : ESlateDrawEffect::DisabledEffect;

			const FLinearColor FinalColorAndOpacity(InWidgetStyle.GetColorAndOpacityTint() * ColorAndOpacityAttribute.Get().GetColor(InWidgetStyle) * imageBrush->GetTint(InWidgetStyle));

			for (int32 i = 0; i < TilingCount; i++)
			{
				const int32 stepIndex = InverseOrderAttribute.Get() ? TilingCount - 1 - i : i;
				FSlateDrawElement::MakeBox(OutDrawElements, LayerId, AllottedGeometry.MakeChild(FVector2D(imageBrushSize), FSlateLayoutTransform(step * stepIndex)).ToPaintGeometry(), imageBrush, DrawEffects, FinalColorAndOpacity);
			}
		}
	}

	return LayerId;
}

bool STiledImage1D::CustomPrepass(float LayoutScaleMultiplier)
{
	TilingCount = 0;
	StepCorrection = FVector2D::ZeroVector;

	if (LastAllocatedArea.IsSet())
	{
		const FVector2D lastAllocatedArea = LastAllocatedArea.GetValue();

		if (lastAllocatedArea.X > 0 && lastAllocatedArea.Y > 0)
		{
			const FSlateBrush* imageBrush = ImageAttribute.Get();
			const FVector2D imageBrushSize = imageBrush->GetImageSize();
			
			if (imageBrushSize.X > 0 && imageBrushSize.Y > 0)
			{
				FVector2D step = DirectionAttribute.Get() * imageBrushSize;

				if (step.X > 0 || step.Y > 0)
				{
					FVector2D position = FVector2D::ZeroVector;
					const FVector2D reducedLastAllocatedArea = lastAllocatedArea - imageBrushSize;

					while (position.ComponentwiseAllLessThan(reducedLastAllocatedArea))
					{
						position += step;
						TilingCount++;
					}

					if (TilingCount > 0)
					{
						const FVector2D directionMask(FMath::Sign(DirectionAttribute.Get().X), FMath::Sign(DirectionAttribute.Get().Y));
						FVector2D totalOffset = reducedLastAllocatedArea * directionMask - position;

						if (position.X > reducedLastAllocatedArea.X && position.Y <= reducedLastAllocatedArea.Y) // exceed only X
						{
							step.Y = 0;
							totalOffset.Y = 0;
						}
						else if (position.X <= reducedLastAllocatedArea.X && position.Y > reducedLastAllocatedArea.Y) // exceed only Y
						{
							step.X = 0;
							totalOffset.X = 0;
						}

						if (totalOffset.SizeSquared() > (totalOffset + step).SizeSquared())
						{
							totalOffset += step;
							TilingCount--;
						}

						StepCorrection = totalOffset / TilingCount / imageBrushSize;

						TilingCount++; // Last item
					}
				}
			}
		}
	}

	return true;
}

UTiledImage1D::UTiledImage1D(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	bIsVariable = false;
	SetVisibilityInternal(ESlateVisibility::SelfHitTestInvisible);

	InverseOrder = false;
	Direction = FVector2D(1, 0);
}

void UTiledImage1D::SetColorAndOpacity(FLinearColor InColorAndOpacity) { if (MyTiledImage1D.IsValid()) MyTiledImage1D->SetColorAndOpacity(ColorAndOpacity = InColorAndOpacity); }

void UTiledImage1D::SetOpacity(float InOpacity) { if (MyTiledImage1D.IsValid()) { ColorAndOpacity.A = InOpacity; MyTiledImage1D->SetColorAndOpacity(ColorAndOpacity); } }

void UTiledImage1D::SetBrushTintColor(FSlateColor TintColor)
{
	if (Brush.TintColor != TintColor)
	{
		Brush.TintColor = TintColor;

		if (MyTiledImage1D.IsValid())
		{
			MyTiledImage1D->SetImage(&Brush);
		}
	}
}

void UTiledImage1D::SetBrushResourceObject(UObject* ResourceObject)
{
	if (Brush.GetResourceObject() != ResourceObject)
	{
		Brush.SetResourceObject(ResourceObject);

		if (MyTiledImage1D.IsValid())
		{
			MyTiledImage1D->SetImage(&Brush);
		}
	}
}

void UTiledImage1D::SetBrush(const FSlateBrush& InBrush)
{
	if (Brush != InBrush)
	{
		Brush = InBrush;

		if (MyTiledImage1D.IsValid())
		{
			MyTiledImage1D->SetImage(&Brush);
		}
	}
}

void UTiledImage1D::SetBrushFromAsset(USlateBrushAsset* Asset)
{
	if (!Asset || Brush != Asset->Brush)
	{
		CancelImageStreaming();
		Brush = Asset ? Asset->Brush : FSlateBrush();

		if (MyTiledImage1D.IsValid())
		{
			MyTiledImage1D->SetImage(&Brush);
		}
	}
}

void UTiledImage1D::SetBrushFromTexture(UTexture2D* Texture, bool bMatchSize)
{
	CancelImageStreaming();

	if (Brush.GetResourceObject() != Texture)
	{
		Brush.SetResourceObject(Texture);

		if (Texture) // Since this texture is used as UI, don't allow it affected by budget.
		{
			Texture->bForceMiplevelsToBeResident = true;
			Texture->bIgnoreStreamingMipBias = true;
		}

		if (bMatchSize)
		{
			if (Texture)
			{
				Brush.ImageSize.X = Texture->GetSizeX();
				Brush.ImageSize.Y = Texture->GetSizeY();
			}
			else
			{
				Brush.ImageSize = FVector2D(0, 0);
			}
		}

		if (MyTiledImage1D.IsValid())
		{
			MyTiledImage1D->SetImage(&Brush);
		}
	}
}

void UTiledImage1D::SetBrushFromAtlasInterface(TScriptInterface<ISlateTextureAtlasInterface> AtlasRegion, bool bMatchSize)
{
	if (Brush.GetResourceObject() != AtlasRegion.GetObject())
	{
		CancelImageStreaming();
		Brush.SetResourceObject(AtlasRegion.GetObject());

		if (bMatchSize)
		{
			if (AtlasRegion)
			{
				FSlateAtlasData AtlasData = AtlasRegion->GetSlateAtlasData();
				Brush.ImageSize = AtlasData.GetSourceDimensions();
			}
			else
			{
				Brush.ImageSize = FVector2D(0, 0);
			}
		}

		if (MyTiledImage1D.IsValid())
		{
			MyTiledImage1D->SetImage(&Brush);
		}
	}
}

void UTiledImage1D::SetBrushFromTextureDynamic(UTexture2DDynamic* Texture, bool bMatchSize)
{
	if (Brush.GetResourceObject() != Texture)
	{
		CancelImageStreaming();
		Brush.SetResourceObject(Texture);

		if (bMatchSize && Texture)
		{
			Brush.ImageSize.X = Texture->SizeX;
			Brush.ImageSize.Y = Texture->SizeY;
		}

		if (MyTiledImage1D.IsValid())
		{
			MyTiledImage1D->SetImage(&Brush);
		}
	}
}

void UTiledImage1D::SetBrushFromMaterial(UMaterialInterface* Material)
{
	if (Brush.GetResourceObject() != Material)
	{
		CancelImageStreaming();
		Brush.SetResourceObject(Material);

		//TODO UMG Check if the material can be used with the UI

		if (MyTiledImage1D.IsValid())
		{
			MyTiledImage1D->SetImage(&Brush);
		}
	}
}

void UTiledImage1D::SetBrushFromSoftTexture(TSoftObjectPtr<UTexture2D> SoftTexture, bool bMatchSize)
{
	TWeakObjectPtr<UTiledImage1D> WeakThis(this); // using weak ptr in case 'this' has gone out of scope by the time this lambda is called

	RequestAsyncLoad(SoftTexture,
		[WeakThis, SoftTexture, bMatchSize]()
		{
			if (UTiledImage1D* StrongThis = WeakThis.Get())
			{
				ensureMsgf(SoftTexture.Get(), TEXT("Failed to load %s"), *SoftTexture.ToSoftObjectPath().ToString());
				StrongThis->SetBrushFromTexture(SoftTexture.Get(), bMatchSize);
			}
		}
	);
}

void UTiledImage1D::SetBrushFromSoftMaterial(TSoftObjectPtr<UMaterialInterface> SoftMaterial)
{
	TWeakObjectPtr<UTiledImage1D> WeakThis(this); // using weak ptr in case 'this' has gone out of scope by the time this lambda is called

	RequestAsyncLoad(SoftMaterial,
		[WeakThis, SoftMaterial]()
		{
			if (UTiledImage1D* StrongThis = WeakThis.Get())
			{
				ensureMsgf(SoftMaterial.Get(), TEXT("Failed to load %s"), *SoftMaterial.ToSoftObjectPath().ToString());
				StrongThis->SetBrushFromMaterial(SoftMaterial.Get());
			}
		}
	);
}

UMaterialInstanceDynamic* UTiledImage1D::GetDynamicMaterial()
{
	UMaterialInterface* Material = NULL;

	UObject* Resource = Brush.GetResourceObject();
	Material = Cast<UMaterialInterface>(Resource);

	if (Material)
	{
		UMaterialInstanceDynamic* DynamicMaterial = Cast<UMaterialInstanceDynamic>(Material);

		if (!DynamicMaterial)
		{
			DynamicMaterial = UMaterialInstanceDynamic::Create(Material, this);
			Brush.SetResourceObject(DynamicMaterial);

			if (MyTiledImage1D.IsValid())
			{
				MyTiledImage1D->SetImage(&Brush);
			}
		}

		return DynamicMaterial;
	}

	//TODO UMG can we do something for textures?  General purpose dynamic material for them?

	return NULL;
}

void UTiledImage1D::SetInverseOrder(bool inverseOrder) { if (MyTiledImage1D.IsValid()) MyTiledImage1D->SetInverseOrder(InverseOrder = inverseOrder); }

void UTiledImage1D::SetDirection(const FVector2D& direction) { if (MyTiledImage1D.IsValid()) MyTiledImage1D->SetDirection(Direction = direction); }

void UTiledImage1D::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	TAttribute<FSlateColor> ColorAndOpacityBinding = PROPERTY_BINDING(FSlateColor, ColorAndOpacity);
	TAttribute<const FSlateBrush*> ImageBinding = OPTIONAL_BINDING_CONVERT(FSlateBrush, Brush, const FSlateBrush*, ConvertImage);

	if (MyTiledImage1D.IsValid())
	{
		MyTiledImage1D->SetImage(ImageBinding);
		MyTiledImage1D->SetColorAndOpacity(ColorAndOpacityBinding);
		MyTiledImage1D->SetInverseOrder(InverseOrder);
		MyTiledImage1D->SetDirection(Direction);
	}
}

void UTiledImage1D::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	MyTiledImage1D.Reset();
}

#if WITH_EDITOR

const FText UTiledImage1D::GetPaletteCategory()
{
	return LOCTEXT("Common", "Common");
}

#endif

TSharedRef<SWidget> UTiledImage1D::RebuildWidget()
{
	MyTiledImage1D = SNew(STiledImage1D)
		.InverseOrder(InverseOrder)
		.Direction(Direction);

	return MyTiledImage1D.ToSharedRef();
}

const FSlateBrush* UTiledImage1D::ConvertImage(TAttribute<FSlateBrush> InImageAsset) const
{
	UTiledImage1D* MutableThis = const_cast<UTiledImage1D*>(this);
	MutableThis->Brush = InImageAsset.Get();

	return &Brush;
}

void UTiledImage1D::RequestAsyncLoad(TSoftObjectPtr<UObject> SoftObject, TFunction<void()>&& Callback)
{
	RequestAsyncLoad(SoftObject, FStreamableDelegate::CreateLambda(MoveTemp(Callback)));
}

void UTiledImage1D::RequestAsyncLoad(TSoftObjectPtr<UObject> SoftObject, FStreamableDelegate DelegateToCall)
{
	CancelImageStreaming();

	if (UObject* StrongObject = SoftObject.Get())
	{
		DelegateToCall.ExecuteIfBound();
		return;  // No streaming was needed, complete immediately.
	}

	OnImageStreamingStarted(SoftObject);

	TWeakObjectPtr<UTiledImage1D> WeakThis(this);
	StreamingObjectPath = SoftObject.ToSoftObjectPath();
	StreamingHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
		StreamingObjectPath,
		[WeakThis, DelegateToCall, SoftObject]()
		{
			if (UTiledImage1D* StrongThis = WeakThis.Get())
			{
				// If the object paths don't match, then this delegate was interrupted, but had already been queued for a callback
				// so ignore everything and abort.
				if (StrongThis->StreamingObjectPath != SoftObject.ToSoftObjectPath())
				{
					return; // Abort!
				}

				// Call the delegate to do whatever is needed, probably set the new image.
				DelegateToCall.ExecuteIfBound();

				// Note that the streaming has completed.
				StrongThis->OnImageStreamingComplete(SoftObject);
			}
		},
		FStreamableManager::AsyncLoadHighPriority);
}

void UTiledImage1D::CancelImageStreaming()
{
	if (StreamingHandle.IsValid())
	{
		StreamingHandle->CancelHandle();
		StreamingHandle.Reset();
	}

	StreamingObjectPath.Reset();
}

void UTiledImage1D::OnImageStreamingStarted(TSoftObjectPtr<UObject> SoftObject)
{
	// No-Op
}

void UTiledImage1D::OnImageStreamingComplete(TSoftObjectPtr<UObject> LoadedSoftObject)
{
	// No-Op
}

#undef LOCTEXT_NAMESPACE