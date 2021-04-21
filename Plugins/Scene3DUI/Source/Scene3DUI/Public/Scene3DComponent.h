// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "Scene3DComponent.generated.h"

DECLARE_STATS_GROUP(TEXT("UScene3DUIComponent"), STATGROUP_UScene3DUIComponent, STATCAT_Advanced);

/**
 *  可遮挡的UI界面组件
 */
UCLASS(Blueprintable, ClassGroup = "UserInterface", hidecategories = (Object, Activation, "Components|Activation", Sockets, Base, Lighting, LOD, Mesh), editinlinenew, meta = (BlueprintSpawnableComponent))
class SCENE3DUI_API UScene3DUIComponent : public UWidgetComponent
{
	GENERATED_UCLASS_BODY()

	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const;

	virtual bool ShouldDrawWidget() const override;

	virtual void BeginDestroy() override;
public:

	/** 更新UI */
	UFUNCTION(BlueprintCallable, Category = "Scene3DUI")
	void UpdateUITexture(float Duration = 0.5f);

	UPROPERTY(EditAnywhere, Category = "Scene3DUI")
	bool bUseSprite;

	/** 材质缩放比例,默认为0.5 */
	UPROPERTY(EditAnywhere, Category = "Scene3DUI")
	float Scale;

	/** 材质最小缩放比例 默认为 0,5 */
	UPROPERTY(EditAnywhere, Category = "Scene3DUI")
	float MinScale;

	/** 最大的显示深度 */
	UPROPERTY(EditAnywhere, Category = "Scene3DUI")
	float MaxDepth;

	/** 最小的显示深度 */
	UPROPERTY(EditAnywhere, Category = "Scene3DUI")
	float MinDepth;

	/** 静态UI */
	UPROPERTY(EditAnywhere, Category = "Scene3DUI")
	bool IsStaticUI;

	/** 渲染UI间隔 */
	UPROPERTY(EditAnywhere, Category = "Scene3DUI")
	float MaxWidgetRenderDeltaTime = 0.5;

	/** 被动更新时,MaxWidgetRenderDeltaTime无效,需要手动调用UpdateUITexture()更新 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scene3DUI")
	bool bIsPassiveUpdate = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scene3DUI")
	UTextureRenderTarget2D* OutputRenderTarget;

	float WidgetRenderDeltaTime;

private:
	bool bIsExecutedUpdateUITexture = false;

	float UpdateUITexture_Duration = 0.0f;
};
