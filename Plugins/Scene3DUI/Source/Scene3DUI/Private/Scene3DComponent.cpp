// Fill out your copyright notice in the Description page of Project Settings.

#include "Scene3DComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "EngineGlobals.h"
#include "PrimitiveViewRelevance.h"
#include "PrimitiveSceneProxy.h"
#include "Components/LightComponent.h"
#include "Engine/CollisionProfile.h"
#include "UObject/UObjectHash.h"
#include "UObject/UObjectIterator.h"
#include "Engine/Texture2D.h"
#include "SceneManagement.h"
#include "Engine/Light.h"
#include "CoreMinimal.h"
#include "Engine/LevelStreaming.h"
#include "LevelUtils.h"
#include "Engine/TextureRenderTarget2D.h"
#include "DynamicMeshBuilder.h"
#include "PhysicsEngine/BodySetup.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "FPostScene3DUIRender.h"
#include "Resources/Version.h"

DECLARE_CYCLE_STAT(TEXT("UpdateWidget"), STAT_UpdateWidget, STATGROUP_UScene3DUIComponent);
DECLARE_CYCLE_STAT(TEXT("DrawWidgetToRenderTarget"), STAT_DrawWidgetToRenderTarget, STATGROUP_UScene3DUIComponent);

/** Represents a billboard sprite to the scene manager. */
class FScene3DUIProxy : public FPrimitiveSceneProxy
{
public:

	SIZE_T GetTypeHash() const
	{
		static size_t UniquePointer;
		return reinterpret_cast<size_t>(&UniquePointer);
	}

	/** Initialization constructor. */
	FScene3DUIProxy(UScene3DUIComponent* InComponent)
		: FPrimitiveSceneProxy(InComponent)
		, RenderTarget(InComponent->GetRenderTarget())
		, MaterialInstance(InComponent->GetMaterialInstance())
		, BodySetup(InComponent->GetBodySetup())
		, IsSprite(InComponent->bUseSprite)
		, Scale(InComponent->Scale)
		, MinScale(InComponent->MinScale)
		, MaxDepth(InComponent->MaxDepth)
		, MinDepth(InComponent->MinDepth)
		, Component(InComponent)
	{
		bWillEverBeLit = false;

		MaterialRelevance = MaterialInstance->GetRelevance(GetScene().GetFeatureLevel());
	}

	// FPrimitiveSceneProxy interface.
	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override
	{
#if WITH_EDITOR
		const bool bWireframe = ViewFamily.EngineShowFlags.Wireframe;
		auto WireframeMaterialInstance = new FColoredMaterialRenderProxy(
			GEngine->WireframeMaterial ? GEngine->WireframeMaterial->GetRenderProxy() : nullptr,
			FLinearColor(0, 0.5f, 1.f)
		);

		Collector.RegisterOneFrameMaterialProxy(WireframeMaterialInstance);

		FMaterialRenderProxy* ParentMaterialProxy = nullptr;
		if (bWireframe)
		{
			ParentMaterialProxy = WireframeMaterialInstance;
		}
		else
		{
			ParentMaterialProxy = MaterialInstance->GetRenderProxy();
		}
#else

		FMaterialRenderProxy* ParentMaterialProxy = MaterialInstance->GetRenderProxy();
		
#endif
		const FMatrix& ViewportLocalToWorld = GetLocalToWorld();

		if (RenderTarget && FPostScene3DUIRender::FScene3DRenderPtr.IsValid())
		{
			FTextureResource* TextureResource = RenderTarget->Resource;
			if (TextureResource)
			{
				for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
				{

					if (VisibilityMap & (1 << ViewIndex))
					{
						const FSceneView *View = Views[ViewIndex];

						float UniformScale = View->WorldToScreen(GetLocalToWorld().GetOrigin()).W * (4.0f / View->UnscaledViewRect.Width() / View->ViewMatrices.GetProjectionMatrix().M[0][0]);

						if (!IsSprite)
						{
							//观察坐标和旋转有改变就重新设置投影点位置
							//如果不在这里做判断就会产生抖动现象
							FPlane TempPlane = View->Project(GetLocalToWorld().GetOrigin());
							FVector TempPoint = GetLocalToWorld().GetOrigin();
							const FMatrix& ProjMatrix = View->ViewMatrices.GetViewProjectionMatrix();
							float SceneDepth = 1.0f / (TempPlane.W / ProjMatrix.M[3][2] - ProjMatrix.M[2][2] / ProjMatrix.M[3][2]);
							TempPlane.W = SceneDepth;

							float Dev = 0.000008;
							if (
								ViewLocation != View->ViewLocation
								|| ViewRotation != View->ViewRotation
								||  FMath::Abs(ProjectPoint.W - TempPlane.W) > 0.001
								|| (ProjectPoint - TempPlane).SizeSquared() > Dev
								|| (LocalPoint - TempPoint).SizeSquared() > 10.0
								)
							{
								ViewLocation = View->ViewLocation;

								ViewRotation = View->ViewRotation;

								ProjectPoint = TempPlane;
								LocalPoint = TempPoint;
							}

							//小于设置的最大深度值
							if (MaxDepth > ProjectPoint.W && MinDepth < ProjectPoint.W)
							{
								float CurrentScale = Scale - ((Scale - MinScale) / MaxDepth) *  ProjectPoint.W;
								FPostScene3DUIRender::FScene3DRenderPtr->SetUIParam(TextureResource, 
									ProjectPoint, CurrentScale, 
									GetLocalToWorld().GetOrigin(), Component);
							}
						}
						else
						{
							Collector.GetPDI(ViewIndex)->DrawSprite(
								GetLocalToWorld().GetOrigin(),
								RenderTarget->SizeX * UniformScale / 4.0 / Scale,
								RenderTarget->SizeY * UniformScale / 4.0 / Scale,
								TextureResource,
								FColor::White,
								GetDepthPriorityGroup(View),
								0, RenderTarget->SizeX, 0, RenderTarget->SizeY
							);
						}
					}
				}
			}
		}

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
		for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
		{
			if (VisibilityMap & (1 << ViewIndex))
			{
				RenderCollision(BodySetup, Collector, ViewIndex, ViewFamily.EngineShowFlags, GetBounds(), IsSelected());
				RenderBounds(Collector.GetPDI(ViewIndex), ViewFamily.EngineShowFlags, GetBounds(), IsSelected());
			}
		}
#endif
	}

	void RenderCollision(UBodySetup* InBodySetup, FMeshElementCollector& Collector, int32 ViewIndex, const FEngineShowFlags& EngineShowFlags, const FBoxSphereBounds& InBounds, bool bRenderInEditor) const
	{
	}


	
	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
	{
		bool bVisible = true;

		FPrimitiveViewRelevance Result;

		MaterialRelevance.SetPrimitiveViewRelevance(Result);

		if (View->bIsReflectionCapture || View->bIsPlanarReflection || View->bIsSceneCapture)
		{
			Result.bDrawRelevance = false;
			Result.bDynamicRelevance = false;
		}
		else
		{
			Result.bDrawRelevance = IsShown(View) && bVisible && View->Family->EngineShowFlags.WidgetComponents;
			Result.bDynamicRelevance = true;
		}
		Result.bShadowRelevance = false;
		Result.bEditorPrimitiveRelevance = false;

		return Result;
	}

	virtual void GetLightRelevance(const FLightSceneProxy* LightSceneProxy, bool& bDynamic, bool& bRelevant, bool& bLightMapped, bool& bShadowMapped) const override
	{
		bDynamic = false;
		bRelevant = false;
		bLightMapped = false;
		bShadowMapped = false;
	}


	virtual bool CanBeOccluded() const override
	{
		return !MaterialRelevance.bDisableDepthTest;
	}

	virtual uint32 GetMemoryFootprint(void) const override { return(sizeof(*this) + GetAllocatedSize()); }

	uint32 GetAllocatedSize(void) const { return(FPrimitiveSceneProxy::GetAllocatedSize()); }


private:

	UTextureRenderTarget2D* RenderTarget;

	UMaterialInstanceDynamic* MaterialInstance;

	FMaterialRelevance MaterialRelevance;

	UBodySetup* BodySetup;

	bool IsSprite;

	float Scale;

	float MinScale;

	float MaxDepth;

	float MinDepth;

	mutable FPlane ProjectPoint;
	
	mutable FVector LocalPoint;

	mutable FVector	ViewLocation;

	mutable FRotator ViewRotation;

	UScene3DUIComponent* Component;
};

UScene3DUIComponent::UScene3DUIComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetUsingAbsoluteScale(true);
	bHiddenInGame = false;
	//bGenerateOverlapEvents = false;
	bUseEditorCompositing = true;
	bDrawAtDesiredSize = true;
	bUseSprite = false;
	Scale = 0.5;
	MinScale = 0.5;
	MaxDepth = 1.0;
	MinDepth = 0.0;

	//禁止反射平面会渲染
	bVisibleInReflectionCaptures = false;
	BoundsScale = 10.0;
	IsStaticUI = false;
}
FPrimitiveSceneProxy* UScene3DUIComponent::CreateSceneProxy()
{
	if (!FPostScene3DUIRender::FScene3DRenderPtr.IsValid())
	{
		FPostScene3DUIRender::FScene3DRenderPtr = FSceneViewExtensions::NewExtension<FPostScene3DUIRender>();
	}

	float SpriteScale = 1.0f;
#if WITH_EDITOR
	if (GetOwner())
	{
		SpriteScale = GetOwner()->SpriteScale;
	}
#endif

	if (MaterialInstance)
	{
		MaterialInstance = nullptr;
	}

	if (Space == EWidgetSpace::Screen)
	{
		return nullptr;
	}
	if (!IsInGameThread())
	{
		return nullptr;
	}
	//if (WidgetRenderer && CurrentSlateWidget.IsValid())
	if (WidgetRenderer)
	{
		UMaterialInterface* BaseMaterial = GetMaterial(0);
		MaterialInstance = UMaterialInstanceDynamic::Create(BaseMaterial, this);

		UpdateMaterialInstanceParameters();

		RequestRedraw();
		LastWidgetRenderTime = 0;
		WidgetRenderDeltaTime = 0;

		return new FScene3DUIProxy(this);
	}

#if WITH_EDITOR
	// make something so we can see this component in the editor
	class FWidgetBoxProxy : public FPrimitiveSceneProxy
	{
	public:
		SIZE_T GetTypeHash() const
		{
			static size_t UniquePointer;
			return reinterpret_cast<size_t>(&UniquePointer);
		}

		FWidgetBoxProxy(const UWidgetComponent* InComponent)
			: FPrimitiveSceneProxy(InComponent)
			, BoxExtents(1.f, InComponent->GetDrawSize().X / 2.0f, InComponent->GetDrawSize().Y / 2.0f)
		{
			bWillEverBeLit = false;
		}

		virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override
		{
			QUICK_SCOPE_CYCLE_COUNTER(STAT_BoxSceneProxy_GetDynamicMeshElements);

			const FMatrix& LocalToWorld = GetLocalToWorld();

			for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
			{
				if (VisibilityMap & (1 << ViewIndex))
				{
					const FSceneView* View = Views[ViewIndex];

					const FLinearColor DrawColor = GetViewSelectionColor(FColor::White, *View, IsSelected(), IsHovered(), false, IsIndividuallySelected());

					FPrimitiveDrawInterface* PDI = Collector.GetPDI(ViewIndex);
					DrawOrientedWireBox(PDI, LocalToWorld.GetOrigin(), LocalToWorld.GetScaledAxis(EAxis::X), LocalToWorld.GetScaledAxis(EAxis::Y), LocalToWorld.GetScaledAxis(EAxis::Z), BoxExtents, DrawColor, SDPG_World);
				}
			}
		}

		virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
		{
			FPrimitiveViewRelevance Result;
			if (!View->bIsGameView)
			{
				// Should we draw this because collision drawing is enabled, and we have collision
				const bool bShowForCollision = View->Family->EngineShowFlags.Collision && IsCollisionEnabled();
				Result.bDrawRelevance = IsShown(View) || bShowForCollision;
				Result.bDynamicRelevance = true;
				Result.bShadowRelevance = IsShadowCast(View);
				Result.bEditorPrimitiveRelevance = UseEditorCompositing(View);
			}
			return Result;
		}
		virtual uint32 GetMemoryFootprint(void) const override { return(sizeof(*this) + GetAllocatedSize()); }
		uint32 GetAllocatedSize(void) const { return(FPrimitiveSceneProxy::GetAllocatedSize()); }

	private:
		const FVector	BoxExtents;
	};

	return new FWidgetBoxProxy(this);
#else
	return nullptr;
#endif
}

void UScene3DUIComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	//因为如果界面设置了折叠导致渲染大小变成零后，就无法再渲染这个到目标纹理。
	DrawSize.X = 220;
	DrawSize.Y = 250;

#if !UE_SERVER
	if (!IsRunningDedicatedServer())
	{

		

		//if (!IsValid(GetWidget()) && !SlateWidget.IsValid())
		if (!IsValid(GetWidget()))
		{
			return;
		}

		WidgetRenderDeltaTime += DeltaTime;
		if (WidgetRenderDeltaTime > MaxWidgetRenderDeltaTime && !IsStaticUI)
		{
			WidgetRenderDeltaTime = 0.0;
		}

		if (Space != EWidgetSpace::Screen)
		{
			if ((ShouldDrawWidget() && WidgetRenderDeltaTime == 0.0 && !bIsPassiveUpdate) || bIsExecutedUpdateUITexture)
			{
				{
					SCOPE_CYCLE_COUNTER(STAT_UpdateWidget);
					UpdateWidget();
				}

				{
					SCOPE_CYCLE_COUNTER(STAT_DrawWidgetToRenderTarget);
					const float DeltaTimeFromLastDraw = LastWidgetRenderTime == 0 ? 0 : (GetCurrentTime() - LastWidgetRenderTime);
					DrawWidgetToRenderTarget(DeltaTimeFromLastDraw);
				}
			}
		}
	}
#endif // !UE_SERVER

	// 持续更新定时器
	{
		UpdateUITexture_Duration -= DeltaTime;

		if (UpdateUITexture_Duration <= 0) bIsExecutedUpdateUITexture = false;
	}
}
bool UScene3DUIComponent::ShouldDrawWidget() const
{
	return !IsStaticUI || LastWidgetRenderTime == 0.0;
}

/**
 * 因为边界太小会导致UI在遮挡查询中被剔除，所以增大盒子大小避免闪烁。
 */
FBoxSphereBounds UScene3DUIComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	FBoxSphereBounds BoxSphereBounds  = Super::CalcBounds(LocalToWorld);
	BoxSphereBounds.BoxExtent = BoxSphereBounds.BoxExtent * 2.0f;
	return BoxSphereBounds;
}
void UScene3DUIComponent::UpdateUITexture(float Duration)
{
	bIsExecutedUpdateUITexture = true;
	UpdateUITexture_Duration = FMath::Max(UpdateUITexture_Duration, Duration);
}

void UScene3DUIComponent::BeginDestroy()
{
	Super::BeginDestroy();
}
