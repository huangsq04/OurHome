// Fill out your copyright notice in the Description page of Project Settings.

#include "FPostScene3DUIRender.h"
#include "GlobalShader.h"
#include "Shader.h"
#include "RHICommandList.h"
#include "ShaderParameterUtils.h"
#include "RHIStaticStates.h"
#include "PipelineStateCache.h"
#include "PostProcess/SceneRenderTargets.h"
#include "Resources/Version.h"
#include "Scene3DComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Serialization/MemoryLayout.h"

TSharedPtr< class FPostScene3DUIRender, ESPMode::ThreadSafe > FPostScene3DUIRender::FScene3DRenderPtr;

class FScene3DUIVS : public FGlobalShader
{
	DECLARE_EXPORTED_SHADER_TYPE(FScene3DUIVS, Global, /*MYMODULE_API*/);

	FScene3DUIVS() { }
	FScene3DUIVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{
	}

	static bool ShouldCache(EShaderPlatform Platform)
	{
		return true;
	}

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5) || IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::ES3_1);
	}
};
IMPLEMENT_SHADER_TYPE(, FScene3DUIVS, TEXT("/Plugin/Scene3DUI/Private/Scene3DUI.usf"), TEXT("MainVS"), SF_Vertex);

struct FTextureVertex
{
	FVector4	Position;
	FVector2D	UV;
};
class FScene3DUIPS : public FGlobalShader
{
	DECLARE_EXPORTED_SHADER_TYPE(FScene3DUIPS, Global, /*MYMODULE_API*/);

	/** 窗口尺寸 */
	LAYOUT_FIELD(FShaderParameter,SceneSizeParameter);
	/** UI纹理大小 */
	LAYOUT_FIELD(FShaderParameter,UISizeParameter);
	/** UI纹理深度 */
	LAYOUT_FIELD(FShaderParameter,UIDepthParameter);
	/** UI纹理起始坐标 */
	LAYOUT_FIELD(FShaderParameter,UIStartPosParameter);
	/** 场景大小百分比 */
	LAYOUT_FIELD(FShaderParameter,ScreenPercentageParameter);

	/** UI纹理 */
	LAYOUT_FIELD(FShaderResourceParameter,UITextureParameter);
	LAYOUT_FIELD(FShaderResourceParameter,UITextureParameterSampler);

	/** 场景深度纹理 */
	LAYOUT_FIELD(FShaderResourceParameter,SceneDepthTextureParameter);
	LAYOUT_FIELD(FShaderResourceParameter,SceneDepthTextureParameterSampler);


	FScene3DUIPS() { }

	FScene3DUIPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{
		SceneSizeParameter.Bind(Initializer.ParameterMap, TEXT("SceneSize"), SPF_Mandatory);
		UISizeParameter.Bind(Initializer.ParameterMap, TEXT("UISize"), SPF_Mandatory);
		UIDepthParameter.Bind(Initializer.ParameterMap, TEXT("UIDepth"), SPF_Mandatory);
		UIStartPosParameter.Bind(Initializer.ParameterMap, TEXT("UIStartPos"), SPF_Mandatory);
		ScreenPercentageParameter.Bind(Initializer.ParameterMap, TEXT("ScreenPercentage"), SPF_Mandatory);

		SceneDepthTextureParameter.Bind(Initializer.ParameterMap, TEXT("DepthTexture"));
		SceneDepthTextureParameterSampler.Bind(Initializer.ParameterMap, TEXT("DepthTextureSampler"));

		UITextureParameter.Bind(Initializer.ParameterMap, TEXT("UITexture"));
		UITextureParameterSampler.Bind(Initializer.ParameterMap, TEXT("UITextureSampler"));

	}

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5) || IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::ES3_1);
	}
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
		OutEnvironment.SetDefine(TEXT("MY_DEFINE"), 1);
	}

	static bool ShouldCache(EShaderPlatform Platform)
	{
		// Could skip compiling for Platform == SP_METAL for example
		return true;
	}

	void DrawUI(FRHICommandList& RHICmdList,
		FPostScene3DUIRender::FBatched3DUI *Param,
		FIntRect &IntRect,
		FIntPoint &SceneSize,
		FTexture2DRHIRef SceneDepthTexture,
		FRHIPixelShader * PixelShader = nullptr,
		bool FlipY = false
	)
	{
		FTexture* TempResource = Param->Texture;
		check(TempResource);
		//检查资源是否已经被释放
		if (TempResource->bGreyScaleFormat || 
			TempResource->bGreyScaleFormat || 
			TempResource->bSRGB || 
			!TempResource->TextureRHI.IsValid())
		{
			return;
		}


		FPlane Plane = Param->Plane;


		FIntPoint UISize = FIntPoint(TempResource->GetSizeX(), TempResource->GetSizeY());

		UISize.X = (float)UISize.X * Param->Scale * SceneSize.X /IntRect.Max.X;
		UISize.Y = (float)UISize.Y * Param->Scale * SceneSize.Y /IntRect.Max.Y;
		FVector2D ScreenPercentage((float)IntRect.Max.X / SceneSize.X, (float)IntRect.Max.Y / SceneSize.Y);

		static const auto ScreenPercentageCVar = IConsoleManager::Get().FindTConsoleVariableDataFloat(TEXT("r.ScreenPercentage"));
		float CurrentValue = ScreenPercentageCVar->GetValueOnAnyThread() / 100.0;
		ScreenPercentage *= CurrentValue;

		float UIDepth = Plane.W;
		float X = 1.0 / SceneSize.X * UISize.X * 2.0;
		float Y = 1.0 / SceneSize.Y * UISize.Y * 2.0;

		float StartY = Plane.Y;
		float StartX = Plane.X - X / 2.0;

		if (FlipY)
		{
			StartY = -StartY;
		}
		//UI的开始坐标
		FVector2D UIStartPos((1.0 + Plane.X - (X / 2.0)) / 2.0, (1.0 - Plane.Y - Y) / 2.0);

		FRHIResourceCreateInfo CreateInfo;

		struct TemplateVertexBuffer : FResourceArrayInterface
		{
			FTextureVertex buffer[4];
			virtual const void* GetResourceData() const override { return static_cast<const void*>(buffer); }
			virtual uint32 GetResourceDataSize() const override { return sizeof(FTextureVertex) * 4; }
			virtual void Discard() override {}
			virtual bool IsStatic() const override { return false; }
			virtual bool GetAllowCPUAccess() const override { return false; }
			virtual void SetAllowCPUAccess(bool bInNeedsCPUAccess) override {}
		};

		TemplateVertexBuffer buffer;
		auto& Vertices = buffer.buffer;

		Vertices[0].Position = FVector4(StartX, StartY + Y, 0.0, 1.0f);
		Vertices[1].Position = FVector4(StartX + X, StartY + Y, 0.0, 1.0f);
		Vertices[2].Position = FVector4(StartX, StartY, 0.0, 1.0f);
		Vertices[3].Position = FVector4(StartX + X, StartY, 0.0, 1.0f);

		if (FlipY)
		{
			UIStartPos.Y = 1.0 - UIStartPos.Y;
			Vertices[0].UV = FVector2D(0, 1);
			Vertices[1].UV = FVector2D(1, 1);
			Vertices[2].UV = FVector2D(0, 0);
			Vertices[3].UV = FVector2D(1, 0);
		}
		else
		{
			Vertices[0].UV = FVector2D(0, 0);
			Vertices[1].UV = FVector2D(1, 0);
			Vertices[2].UV = FVector2D(0, 1);
			Vertices[3].UV = FVector2D(1, 1);
		}
		CreateInfo.ResourceArray = &buffer;
		CreateInfo.DebugName = TEXT("Scene3DUI Vertex Buffer");
		FVertexBufferRHIRef VertexBufferRHI = RHICreateVertexBuffer(sizeof(FTextureVertex) * 4, BUF_Volatile, CreateInfo);

		RHICmdList.SetStreamSource(0, VertexBufferRHI, 0);

		if (SceneDepthTextureParameter.IsBound() || SceneDepthTextureParameterSampler.IsBound())
		{
			SetTextureParameter(
				RHICmdList,
				PixelShader,
				SceneDepthTextureParameter,
				SceneDepthTextureParameterSampler,
				TStaticSamplerState<SF_Point, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI(),
				SceneDepthTexture
			);
		}
		if (UITextureParameter.IsBound() || UITextureParameterSampler.IsBound())
		{
			SetTextureParameter(
				RHICmdList,
				PixelShader,
				UITextureParameter,
				UITextureParameterSampler,
				TStaticSamplerState<SF_Trilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI(),
				TempResource->TextureRHI
			);
		}

		SetShaderValue(RHICmdList, PixelShader, SceneSizeParameter, SceneSize);
		SetShaderValue(RHICmdList, PixelShader, UISizeParameter, UISize);
		SetShaderValue(RHICmdList, PixelShader, UIDepthParameter, UIDepth);
		SetShaderValue(RHICmdList, PixelShader, UIStartPosParameter, UIStartPos);
		SetShaderValue(RHICmdList, PixelShader, ScreenPercentageParameter, ScreenPercentage);
		
		RHICmdList.DrawPrimitive(0, 2, 1);
	}
};

// Same source file as before, different entry point
IMPLEMENT_SHADER_TYPE(, FScene3DUIPS, TEXT("/Plugin/Scene3DUI/Private/Scene3DUI.usf"), TEXT("MainPS"), SF_Pixel);

void FPostScene3DUIRender::SetupViewFamily(FSceneViewFamily& InViewFamily)
{

}
void FPostScene3DUIRender::SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView)
{

}
void FPostScene3DUIRender::PreRenderView_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneView& InView)
{
	check(IsInRenderingThread());
	SCOPED_DRAW_EVENT(RHICmdList, PreRenderView3DUIRender);
}
void FPostScene3DUIRender::PreRenderViewFamily_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneViewFamily& InViewFamily)
{
	check(IsInRenderingThread());
	SCOPED_DRAW_EVENT(RHICmdList, PreRenderViewFamily3DUIRender);
}
void FPostScene3DUIRender::PostRenderBasePass_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneView& InView)
{
	SCOPED_DRAW_EVENT(RHICmdList, PostBasePass3DUIRender);
	check(IsInRenderingThread());
	if (InView.Family->Views.Num() == 1)
	{
		FIntRect IntRect = InView.Family->Views[0]->UnscaledViewRect;
		//RenderUI(RHICmdList, IntRect);
	}
}
/** 
 * 后渲染线程`
 */
void FPostScene3DUIRender::PostRenderViewFamily_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneViewFamily& InViewFamily)
{
	SCOPED_DRAW_EVENT(RHICmdList, PostScene3DUIRender);
	check(IsInRenderingThread());
	if (InViewFamily.Views.Num() == 1)
	{
		const bool FlipY = RHINeedsToSwitchVerticalAxis(InViewFamily.GetShaderPlatform());
		FRHITexture* RenderTargetTexture = InViewFamily.RenderTarget->GetRenderTargetTexture().GetReference();
		FIntRect IntRect = InViewFamily.Views[0]->UnscaledViewRect;
		RenderUI(RHICmdList, IntRect, RenderTargetTexture, FlipY);
	}

}
bool FPostScene3DUIRender::IsActiveThisFrame(class FViewport* InViewport) const
{
	return true;
}

FPostScene3DUIRender::FPostScene3DUIRender(const FAutoRegister& AutoRegister) :
	FSceneViewExtensionBase(AutoRegister)
{

}
FPostScene3DUIRender::~FPostScene3DUIRender()
{

}
/** 
  * 设置显示界面参数 
  * @param Resource		   要显示的材质
  * @param Plane            屏幕坐标和深度
  * @param Scale            缩放
  * @param WorldLocation    世界坐标
  * @param InComponent      对应组件 
  */
bool FPostScene3DUIRender::SetUIParam(FTexture* Resource, const FPlane &Plane, float Scale, const FVector &WorldLocation, UActorComponent *InComponent)
{
	FBatched3DUI* UI = new(UISprites) FBatched3DUI;
	UI->Plane = Plane;
	UI->Texture = Resource;
	UI->Scale = Scale;
	UI->WorldLocation = WorldLocation;
	UI->Component = InComponent;
	return true;
}

void FPostScene3DUIRender::RenderUI(FRHICommandListImmediate& RHICmdList, FIntRect &IntRect, FRHITexture* RenderTargetTexture, bool FlipY )
{
	if (UISprites.Num() == 0)
	{
		return;
	}

	struct FCompareDepth
	{
		FORCEINLINE bool operator()(const FBatched3DUI& A, const FBatched3DUI& B) const
		{
			return (A.Plane.W > B.Plane.W) ? false : true;
		}
	};
	if (RenderTargetTexture == nullptr)
	{
		return;
	}

	//UI深度排序
	UISprites.Sort(FCompareDepth());
	FSceneRenderTargets& SceneContext = FSceneRenderTargets::Get(RHICmdList);
	FTexture2DRHIRef SceneDepthTexture = SceneContext.GetSceneDepthSurface();

	FIntPoint SceneSize = SceneDepthTexture->GetTexture2D()->GetSizeXY();

	FRHIRenderPassInfo RPInfo(RenderTargetTexture, ERenderTargetActions::Load_Store);
	TransitionRenderPassTargets(RHICmdList, RPInfo);
	RHICmdList.BeginRenderPass(RPInfo, TEXT("Render3DUI"));

	const auto featureLevel = GMaxRHIFeatureLevel;
	auto ShaderMap = GetGlobalShaderMap(featureLevel);
	TShaderMapRef<FScene3DUIVS> UIVS(ShaderMap);
	TShaderMapRef<FScene3DUIPS> UIPS(ShaderMap);

	FGraphicsPipelineStateInitializer GraphicsPSOInit;
	RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
	//纹理混合状态
	GraphicsPSOInit.BlendState = TStaticBlendState<CW_RGBA, BO_Add, BF_SourceAlpha, BF_InverseSourceAlpha, BO_Add, BF_One, BF_Zero>::GetRHI();
	//GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
	GraphicsPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();
	GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();

	static const FName RendererModuleName("Renderer");
	IRendererModule* RendererModule = FModuleManager::GetModulePtr<IRendererModule>(RendererModuleName);

	{
		FVertexDeclarationElementList FDE;
		FVertexElement tem{ 0, 0, EVertexElementType::VET_Float4, 0, sizeof(FTextureVertex), 0 };
		FDE.Add(tem);
		FVertexElement tem2{ 0, sizeof(FVector4), EVertexElementType::VET_Float2, 1, sizeof(FTextureVertex), 0 };
		FDE.Add(tem2);
		auto VD = RHICreateVertexDeclaration(FDE);
		GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = VD;
	}

	GraphicsPSOInit.BoundShaderState.VertexShaderRHI = UIVS.GetVertexShader();

	GraphicsPSOInit.BoundShaderState.PixelShaderRHI = UIPS.GetPixelShader();
	GraphicsPSOInit.PrimitiveType = PT_TriangleStrip;

	SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);

	for (int i = 0; i < UISprites.Num(); ++i)
	{
		if (!UISprites[i].Component->IsBeingDestroyed() && 
			UISprites[i].Component->IsRegistered() &&
			UISprites[i].Component->IsRenderStateCreated() &&
			Cast<UScene3DUIComponent>(UISprites[i].Component) != nullptr)
		{
			UIPS->DrawUI(RHICmdList, &UISprites[i], IntRect, SceneSize,
				SceneDepthTexture, UIPS.GetPixelShader(), FlipY);
		}
	}
	RHICmdList.EndRenderPass();

	UISprites.Empty();
}
