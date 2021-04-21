// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "Scene3DUI.h"
#include "FPostScene3DUIRender.h"
#include "Interfaces/IPluginManager.h"
#include "ShaderCore.h"
#include "Resources/Version.h"
#include "Engine/GameViewportClient.h"
#include "EngineModule.h"
#include "PostProcess/SceneRenderTargets.h"
#include "LevelSequence.h"
#include "Scene3DComponent.h"
#include "EngineUtils.h" 
#include "GameFramework/CharacterMovementComponent.h"

#define LOCTEXT_NAMESPACE "FScene3DUIModule"

static void	CmdScene3DUIComponent(UWorld* InWorld)
{
	UE_LOG(LogTemp, Warning, TEXT("LoadObject file %s !"), *FString(TEXT("AAAAA")));
	for (TActorIterator<AActor> Itor(InWorld); Itor; ++Itor)
	{
		TInlineComponentArray<UScene3DUIComponent*> Components;
		Itor->GetComponents(Components);
		for (UScene3DUIComponent* Component : Components)
		{
			Component->IsStaticUI = true;
		}
	}

}
FAutoConsoleCommandWithWorld CommandLoadCookAsset(
	TEXT("LoadCookAsset.Scene3DUIComponent"),
	TEXT(""),
	FConsoleCommandWithWorldDelegate::CreateStatic(CmdScene3DUIComponent)
);

static void	CmdMoveComponent(UWorld* InWorld)
{
	UE_LOG(LogTemp, Warning, TEXT("LoadObject file %s !"), *FString(TEXT("AAAAA")));
	for (TActorIterator<AActor> Itor(InWorld); Itor; ++Itor)
	{
		TInlineComponentArray<UCharacterMovementComponent*> Components;
		Itor->GetComponents(Components);
		for (UCharacterMovementComponent* Component : Components)
		{
			Component->SetComponentTickEnabled(false);
		}
	}

}
FAutoConsoleCommandWithWorld CommandMoveComponent(
	TEXT("LoadCookAsset.MoveComponent"),
	TEXT(""),
	FConsoleCommandWithWorldDelegate::CreateStatic(CmdMoveComponent)
);

void PostOpaqueRender(FRHICommandListImmediate& RHICmdList, class FSceneRenderTargets& SceneContext)
{
	FIntRect IntRect;
	FIntPoint IntPoint;
	UGameViewportClient* GameViewport = GEngine->GameViewport;
	if (GEngine->GameViewport != nullptr)
	{
		IntPoint = GameViewport->Viewport->GetSizeXY();
	}
	else
	{
		IntPoint = SceneContext.GetBufferSizeXY();
	}

	IntRect.Max.X = IntPoint.X;
	IntRect.Max.Y = IntPoint.Y;

	if (FPostScene3DUIRender::FScene3DRenderPtr.IsValid())
	{
		//FPostScene3DUIRender::FScene3DRenderPtr = FSceneViewExtensions::NewExtension<FPostScene3DUIRender>();
		FPostScene3DUIRender::FScene3DRenderPtr->RenderUI(RHICmdList, IntRect);
	}

}

void FScene3DUIModule::StartupModule()
{
	FString PluginShaderDir = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("Scene3DUI"))->GetBaseDir(), TEXT("Shaders"));
	AddShaderSourceDirectoryMapping(TEXT("/Plugin/Scene3DUI"), PluginShaderDir);

	//IRendererModule& RendererModule = GetRendererModule();
	//RendererModule.GetResolvedSceneColorCallbacks().AddStatic(&PostOpaqueRender);
}

void FScene3DUIModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}
#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FScene3DUIModule, Scene3DUI)