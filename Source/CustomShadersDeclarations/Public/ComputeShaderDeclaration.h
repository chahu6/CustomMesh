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

	// ������ҽӵ���Ⱦ������ʼִ�м�����ɫ��ʱ������ô˺�������ɫ����ÿ֡����һ�Ρ�
	void BeginRendering();

	// ֹͣ������ɫ����ִ��
	void EndRendering();

	// ÿ�������µĲ���Ҫ����ʱ������ô˺�����
	void UpdateParameters(FWhiteNoiseCSParameters& DrawParameters);

private:
	FWhiteNoiseCSManager() = default;

	static FWhiteNoiseCSManager* Instance;

	//��Ⱦ������ÿһִ֡�����Ǻ�����ί�о��
	FDelegateHandle OnPostResolvedSceneColorHandle;

	//������ɫ������������
	FWhiteNoiseCSParameters CachedParams;

	//�����Ƿ񻺴���Ҫ���ݸ���ɫ���Ĳ���
	volatile bool bCachedParamsAreValid;

	//������ɫ����д��������ĳػ���ȾĿ��
	TRefCountPtr<IPooledRenderTarget> ComputeShaderOutput;

public:
	void Execute_RenderThread(FRDGBuilder& GraphBuilder, const FSceneTextures& SceneTextures);
};