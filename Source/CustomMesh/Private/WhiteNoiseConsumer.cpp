// Fill out your copyright notice in the Description page of Project Settings.


#include "WhiteNoiseConsumer.h"
#include "ComputeShaderDeclaration.h"

AWhiteNoiseConsumer::AWhiteNoiseConsumer()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>("StaticMesh");
	StaticMesh->SetupAttachment(RootComponent);
}

void AWhiteNoiseConsumer::BeginPlay()
{
	Super::BeginPlay();

	FWhiteNoiseCSManager::Get()->BeginRendering();

	UMaterialInstanceDynamic* MID = StaticMesh->CreateAndSetMaterialInstanceDynamic(0);
	MID->SetTextureParameterValue("InputTexture", RenderTarget);
}

void AWhiteNoiseConsumer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	FWhiteNoiseCSManager::Get()->EndRendering();

	Super::EndPlay(EndPlayReason);
}

void AWhiteNoiseConsumer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FWhiteNoiseCSParameters Parameters(RenderTarget);
	TimeStamp++;
	Parameters.TimeStamp = TimeStamp;
	FWhiteNoiseCSManager::Get()->UpdateParameters(Parameters);
}

