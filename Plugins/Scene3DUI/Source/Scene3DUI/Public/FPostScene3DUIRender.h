// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SceneRenderTargetParameters.h"
#include "SceneViewExtension.h"

/**
 * 可遮挡的UI界面后处理模块
 */
class SCENE3DUI_API FPostScene3DUIRender : public FSceneViewExtensionBase
{
public:
	/** 渲染界面结构体 */
	struct FBatched3DUI
	{
		/** 屏幕坐标和深度 */
		FPlane Plane;

		/** 材质 */
		FTexture* Texture;

		/** 缩放 */
		float Scale;

		/** 世界坐标 */
		FVector WorldLocation;

		/** 属于那个组件 */
		UActorComponent *Component;
	};

	/** ISceneViewExtension interface */
	virtual void SetupViewFamily(FSceneViewFamily& InViewFamily) override;
	virtual void SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView) override;
	virtual void BeginRenderViewFamily(FSceneViewFamily& InViewFamily) {}
	virtual void PreRenderView_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneView& InView) override;
	virtual void PreRenderViewFamily_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneViewFamily& InViewFamily) override;
	virtual void PostRenderViewFamily_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneViewFamily& InViewFamily) override;
	virtual bool IsActiveThisFrame(class FViewport* InViewport) const;

	virtual void PostRenderBasePass_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneView& InView) override;

	/** Constructor */
	FPostScene3DUIRender(const FAutoRegister&);

	/** Destructor */
	virtual ~FPostScene3DUIRender();

	/** 
	 * 设置显示界面参数 
	 * @param Resource		   要显示的材质
	 * @param Plane            屏幕坐标和深度
	 * @param Scale            缩放
	 * @param WorldLocation    世界坐标
	 * @param InComponent      对应组件 
	 */
	bool SetUIParam(FTexture* Resource, const FPlane &Plane, float Scale, const FVector &WorldLocation, UActorComponent *InComponent);

	void RenderUI(FRHICommandListImmediate& RHICmdList, FIntRect &IntRect, FRHITexture* RenderTargetTexture = nullptr, bool FlipY = false);

	/** This array is sorted during draw-calls */
	mutable TArray<FBatched3DUI> UISprites;

	void SetRenderTarget(UTextureRenderTarget2D* v) { OutputRenderTarget = v; };

	UTextureRenderTarget2D* OutputRenderTarget;


	static TSharedPtr< class FPostScene3DUIRender, ESPMode::ThreadSafe > FScene3DRenderPtr;
};
