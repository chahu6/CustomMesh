// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WhiteNoiseConsumer.generated.h"

class UTextureRenderTarget2D;

UCLASS()
class CUSTOMMESH_API AWhiteNoiseConsumer : public AActor
{
	GENERATED_BODY()
	
public:	
	AWhiteNoiseConsumer();

protected:
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason);

public:	
	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> StaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ShaderDemo)
	TObjectPtr<UTextureRenderTarget2D> RenderTarget;

private:
	uint32 TimeStamp = 0;
};
