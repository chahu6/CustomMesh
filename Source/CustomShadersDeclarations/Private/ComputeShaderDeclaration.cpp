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
	//�����������ɫ��ʹ�ýṹ��Ϊ�����
	SHADER_USE_PARAMETER_STRUCT(FWhiteNoiseCS, FGlobalShader);
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_UAV(RWTexture2D<float>, OutputTexture)
		SHADER_PARAMETER(FVector2f, Dimensions)
		SHADER_PARAMETER(UINT, TimeStamp)
	END_SHADER_PARAMETER_STRUCT()

public:
	// ��Ӹ���ɫ���ı��뻷������������mobile��ʵ�֣���������в���������ϡ�
	//Called by the engine to determine which permutations to compile for this shader
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}

	// �涨�����߳���(numthreads)��
	//Modifies the compilations environment of the shader
	static inline void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);

		//����������ʹ���������һЩԤ���������塣�����������Ǹ���NUM_THREAD_PER_GROUP_DIMENSION��ֵʱ�����ǾͲ���ͬʱ����C++��HLSL����
		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_X"), NUM_THREADS_PER_GROUP_DIMENSION);
		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_Y"), NUM_THREADS_PER_GROUP_DIMENSION);
		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_Z"), 1);
	}
};

//�⽫�������洴����ɫ���Լ���ɫ����ڵ��λ�á�
//ShaderType ShaderPath��ɫ��������Type
// ���У���һ����Ϊ����ɫ��������ƣ��ڶ�����Ϊ����·��������Ϊ��ɫ�����ͣ���CS(compute shader)��PS(pixel shader)��VS(vertex shader)��
IMPLEMENT_GLOBAL_SHADER(FWhiteNoiseCS, "/CustomShaders/WhiteNoiseCS.usf", "MainComputeShader", SF_Compute);

FWhiteNoiseCSManager* FWhiteNoiseCSManager::Instance = nullptr;

void FWhiteNoiseCSManager::BeginRendering()
{
	//�������Ѿ���ʼ��������Ч��������ִ���κβ���
	if (OnPostResolvedSceneColorHandle.IsValid())
	{
		return;
	}
	bCachedParamsAreValid = false;
	//��ȡ��Ⱦ��ģ�飬�������ǵ���Ŀ��ӵ��ص��У��Ա��ڳ�����Ⱦ��ɺ�ÿһ֡������ִ����
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

//ͨ���ṩ��ɫ��������ʹ�õ�parameters�ṹ��ʵ�������²���
void FWhiteNoiseCSManager::UpdateParameters(FWhiteNoiseCSParameters& DrawParameters)
{
	CachedParams = DrawParameters;
	bCachedParamsAreValid = true;
}

/// <summary>
/// ������ɫ�����Ͳ����ṹ��ʵ������ʹ�û������ɫ�������������ṹ�����
/// ��ȫ����ɫ��ӳ���л�ȡ����ɫ�����͵�����
/// ʹ�ò����ṹʵ��������ɫ��
/// </summary>
void FWhiteNoiseCSManager::Execute_RenderThread(FRDGBuilder& GraphBuilder, const FSceneTextures& SceneTextures)
{
	// ���û��Ҫʹ�õĻ��������������
	// ���cachedParams��û���ṩ��ȾĿ�꣬������
	if (!(bCachedParamsAreValid && CachedParams.RenderTarget))
	{
		return;
	}
	
	// ��Ⱦ�̶߳���
	check(IsInRenderingThread());

	FRHICommandListImmediate& RHICmdList = GraphBuilder.RHICmdList;

	//�����ȾĿ����Ч����ͨ���ṩ����������ȾĿ����л�ȡԪ��
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

	// �������ǰ�󶨵���ȾĿ��
	// UnbindRenderTargets(RHICmdList);

	//ָ����Դת���������ں󳡾���Ⱦ��ִ�д˲�����������ǽ�������ΪGraphics to Compute
	//RHICmdList.Transition(FRHITransitionInfo(ComputeShaderOutput->GetRenderTargetItem().UAV, ERHIAccess::UAVCompute));

	//�ÿͻ����ṩ�Ļ������������ɫ�������ṹ
	FWhiteNoiseCS::FParameters PassParameters;
	PassParameters.OutputTexture = ComputeShaderOutput->GetRenderTargetItem().UAV;
	PassParameters.Dimensions = FVector2f(CachedParams.GetRenderTargetSize().X, CachedParams.GetRenderTargetSize().Y);
	PassParameters.TimeStamp = CachedParams.TimeStamp;

	//��ȫ����ɫ��ӳ���л�ȡ����ɫ�����͵�����
	TShaderMapRef<FWhiteNoiseCS> WhiteNoiseCS(GetGlobalShaderMap(GMaxRHIFeatureLevel));

	//���ɼ�����ɫ��
	FComputeShaderUtils::Dispatch(RHICmdList, WhiteNoiseCS, PassParameters,
		FIntVector(FMath::DivideAndRoundUp(CachedParams.GetRenderTargetSize().X, NUM_THREADS_PER_GROUP_DIMENSION),
			FMath::DivideAndRoundUp(CachedParams.GetRenderTargetSize().Y, NUM_THREADS_PER_GROUP_DIMENSION), 1));

	//����ɫ����������Ƶ��ͻ����ṩ����ȾĿ��
	RHICmdList.CopyTexture(ComputeShaderOutput->GetRenderTargetItem().ShaderResourceTexture, CachedParams.RenderTarget->GetRenderTargetResource()->TextureRHI, FRHICopyTextureInfo());
}
