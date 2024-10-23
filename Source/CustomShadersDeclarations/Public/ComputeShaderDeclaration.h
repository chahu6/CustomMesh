#pragma once

#include "Engine/TextureRenderTarget2D.h"

struct FWhiteNoiseCSParameters
{
	UTextureRenderTarget2D* RenderTarget = nullptr;

	FIntPoint GetRenderTargetSize() const
	{
		return CachedRenderTargetSize;
	}

	FWhiteNoiseCSParameters() = default;
	FWhiteNoiseCSParameters(UTextureRenderTarget2D* IORenderTarget)
		:RenderTarget(IORenderTarget)
	{
		CachedRenderTargetSize = RenderTarget ? FIntPoint(RenderTarget->SizeX, RenderTarget->SizeY) : FIntPoint::ZeroValue;
	}

private:
	FIntPoint CachedRenderTargetSize;

public:
	uint32 TimeStamp = 0;
};

class CUSTOMSHADERSDECLARATIONS_API FWhiteNoiseCSManager
{
public:
	static FWhiteNoiseCSManager* Get()
	{
		if (!Instance)
		{
			Instance = new FWhiteNoiseCSManager();
		}
		return Instance;
	}

	// 当您想挂接到渲染器并开始执行计算着色器时，请调用此函数。着色器将每帧分派一次。
	void BeginRendering();

	// 停止计算着色器的执行
	void EndRendering();

	// 每当您有新的参数要共享时，请调用此函数。
	void UpdateParameters(FWhiteNoiseCSParameters& DrawParameters);

private:
	FWhiteNoiseCSManager() = default;

	static FWhiteNoiseCSManager* Instance;

	//渲染器将在每一帧执行我们函数的委托句柄
	FDelegateHandle OnPostResolvedSceneColorHandle;

	//缓存着色器管理器参数
	FWhiteNoiseCSParameters CachedParams;

	//我们是否缓存了要传递给着色器的参数
	volatile bool bCachedParamsAreValid;

	//引用着色器将写入其输出的池化渲染目标
	TRefCountPtr<IPooledRenderTarget> ComputeShaderOutput;

public:
	void Execute_RenderThread(FRDGBuilder& GraphBuilder, const FSceneTextures& SceneTextures);
};