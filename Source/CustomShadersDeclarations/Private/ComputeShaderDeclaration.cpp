#include "ComputeShaderDeclaration.h"
#include "GlobalShader.h"
#include "ShaderParameterStruct.h"
#include "RenderTargetPool.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"

#define NUM_THREADS_PER_GROUP_DIMENSION 32

class FWhiteNoiseCS : public FGlobalShader
{
public:
	// declare this class as a global shader
	DECLARE_GLOBAL_SHADER(FWhiteNoiseCS)
	//告诉引擎此着色器使用结构作为其参数
	SHADER_USE_PARAMETER_STRUCT(FWhiteNoiseCS, FGlobalShader);
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_UAV(RWTexture2D<float>, OutputTexture)
		SHADER_PARAMETER(FVector2f, Dimensions)
		SHADER_PARAMETER(UINT, TimeStamp)
	END_SHADER_PARAMETER_STRUCT()

public:
	// 添加该着色器的编译环境，若是想在mobile端实现，请读者自行查阅相关资料。
	//Called by the engine to determine which permutations to compile for this shader
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}

	// 规定组内线程数(numthreads)：
	//Modifies the compilations environment of the shader
	static inline void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);

		//我们在这里使用它来添加一些预处理器定义。这样，当我们更改NUM_THREAD_PER_GROUP_DIMENSION的值时，我们就不必同时更改C++和HLSL代码
		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_X"), NUM_THREADS_PER_GROUP_DIMENSION);
		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_Y"), NUM_THREADS_PER_GROUP_DIMENSION);
		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_Z"), 1);
	}
};

//这将告诉引擎创建着色器以及着色器入口点的位置。
//ShaderType ShaderPath着色器函数名Type
// 其中，第一参数为该着色器类的名称，第二参数为虚拟路径，第三为着色器类型，有CS(compute shader)，PS(pixel shader)，VS(vertex shader)。
IMPLEMENT_GLOBAL_SHADER(FWhiteNoiseCS, "/CustomShaders/WhiteNoiseCS.usf", "MainComputeShader", SF_Compute);

FWhiteNoiseCSManager* FWhiteNoiseCSManager::Instance = nullptr;

void FWhiteNoiseCSManager::BeginRendering()
{
	//如果句柄已经初始化并且有效，则无需执行任何操作
	if (OnPostResolvedSceneColorHandle.IsValid())
	{
		return;
	}
	bCachedParamsAreValid = false;
	//获取渲染器模块，并将我们的条目添加到回调中，以便在场景渲染完成后每一帧都可以执行它
	const FName RendererModuleName("Renderer");
	if (IRendererModule* RendererModule = FModuleManager::GetModulePtr<IRendererModule>(RendererModuleName))
	{
		OnPostResolvedSceneColorHandle = RendererModule->GetResolvedSceneColorCallbacks().AddRaw(this, &FWhiteNoiseCSManager::Execute_RenderThread);
	}
}

void FWhiteNoiseCSManager::EndRendering()
{
	if (!OnPostResolvedSceneColorHandle.IsValid())
	{
		return;
	}

	const FName RendererModuleName("Renderer");
	if (IRendererModule* RendererModule = FModuleManager::GetModulePtr<IRendererModule>(RendererModuleName))
	{
		RendererModule->GetResolvedSceneColorCallbacks().Remove(OnPostResolvedSceneColorHandle);
	}

	OnPostResolvedSceneColorHandle.Reset();
}

//通过提供着色器管理器使用的parameters结构的实例来更新参数
void FWhiteNoiseCSManager::UpdateParameters(FWhiteNoiseCSParameters& DrawParameters)
{
	CachedParams = DrawParameters;
	bCachedParamsAreValid = true;
}

/// <summary>
/// 创建着色器类型参数结构的实例，并使用缓存的着色器管理器参数结构填充它
/// 从全局着色器映射中获取对着色器类型的引用
/// 使用参数结构实例分派着色器
/// </summary>
void FWhiteNoiseCSManager::Execute_RenderThread(FRDGBuilder& GraphBuilder, const FSceneTextures& SceneTextures)
{
	// 如果没有要使用的缓存参数，请跳过
	// 如果cachedParams中没有提供渲染目标，请跳过
	if (!(bCachedParamsAreValid && CachedParams.RenderTarget))
	{
		return;
	}
	
	// 渲染线程断言
	check(IsInRenderingThread());

	FRHICommandListImmediate& RHICmdList = GraphBuilder.RHICmdList;

	//如果渲染目标无效，请通过提供描述符从渲染目标池中获取元素
	if (!ComputeShaderOutput.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Not Valid"));
		FPooledRenderTargetDesc ComputeShaderOutputDesc(FPooledRenderTargetDesc::Create2DDesc(CachedParams.GetRenderTargetSize(), 
			CachedParams.RenderTarget->GetRenderTargetResource()->TextureRHI->GetFormat(), 
			FClearValueBinding::None, 
			TexCreate_None, 
			TexCreate_ShaderResource | TexCreate_UAV,
			false
		));
		ComputeShaderOutputDesc.DebugName = TEXT("WhiteNoiseCS_Output_RenderTarget");
		GRenderTargetPool.FindFreeElement(RHICmdList, ComputeShaderOutputDesc, ComputeShaderOutput, TEXT("WhiteNoiseCS_Output_RenderTarget"));
	}

	// 解除绑定以前绑定的渲染目标
	// UnbindRenderTargets(RHICmdList);

	//指定资源转换，我们在后场景渲染中执行此操作，因此我们将其设置为Graphics to Compute
	//RHICmdList.Transition(FRHITransitionInfo(ComputeShaderOutput->GetRenderTargetItem().UAV, ERHIAccess::UAVCompute));

	//用客户端提供的缓存数据填充着色器参数结构
	FWhiteNoiseCS::FParameters PassParameters;
	PassParameters.OutputTexture = ComputeShaderOutput->GetRenderTargetItem().UAV;
	PassParameters.Dimensions = FVector2f(CachedParams.GetRenderTargetSize().X, CachedParams.GetRenderTargetSize().Y);
	PassParameters.TimeStamp = CachedParams.TimeStamp;

	//从全局着色器映射中获取对着色器类型的引用
	TShaderMapRef<FWhiteNoiseCS> WhiteNoiseCS(GetGlobalShaderMap(GMaxRHIFeatureLevel));

	//分派计算着色器
	FComputeShaderUtils::Dispatch(RHICmdList, WhiteNoiseCS, PassParameters,
		FIntVector(FMath::DivideAndRoundUp(CachedParams.GetRenderTargetSize().X, NUM_THREADS_PER_GROUP_DIMENSION),
			FMath::DivideAndRoundUp(CachedParams.GetRenderTargetSize().Y, NUM_THREADS_PER_GROUP_DIMENSION), 1));

	//将着色器的输出复制到客户端提供的渲染目标
	RHICmdList.CopyTexture(ComputeShaderOutput->GetRenderTargetItem().ShaderResourceTexture, CachedParams.RenderTarget->GetRenderTargetResource()->TextureRHI, FRHICopyTextureInfo());
}
