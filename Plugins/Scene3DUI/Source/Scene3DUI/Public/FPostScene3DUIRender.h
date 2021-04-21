// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SceneRenderTargetParameters.h"
#include "SceneViewExtension.h"

/**
 * ���ڵ���UI�������ģ��
 */
class SCENE3DUI_API FPostScene3DUIRender : public FSceneViewExtensionBase
{
public:
	/** ��Ⱦ����ṹ�� */
	struct FBatched3DUI
	{
		/** ��Ļ�������� */
		FPlane Plane;

		/** ���� */
		FTexture* Texture;

		/** ���� */
		float Scale;

		/** �������� */
		FVector WorldLocation;

		/** �����Ǹ���� */
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
	 * ������ʾ������� 
	 * @param Resource		   Ҫ��ʾ�Ĳ���
	 * @param Plane            ��Ļ��������
	 * @param Scale            ����
	 * @param WorldLocation    ��������
	 * @param InComponent      ��Ӧ��� 
	 */
	bool SetUIParam(FTexture* Resource, const FPlane &Plane, float Scale, const FVector &WorldLocation, UActorComponent *InComponent);

	void RenderUI(FRHICommandListImmediate& RHICmdList, FIntRect &IntRect, FRHITexture* RenderTargetTexture = nullptr, bool FlipY = false);

	/** This array is sorted during draw-calls */
	mutable TArray<FBatched3DUI> UISprites;

	void SetRenderTarget(UTextureRenderTarget2D* v) { OutputRenderTarget = v; };

	UTextureRenderTarget2D* OutputRenderTarget;


	static TSharedPtr< class FPostScene3DUIRender, ESPMode::ThreadSafe > FScene3DRenderPtr;
};
