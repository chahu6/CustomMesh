// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/DeformMeshMeshComponent.h"

struct FDeformMeshVertexFactory : FLocalVertexFactory
{
	DECLARE_VERTEX_FACTORY_TYPE(FDeformMeshVertexFactory);
public:
	//FDeformMeshVertexFactory(ERHIFeatureLevel::Type InFeatureLevel)

};

//IMPLEMENT_VERTEX_FACTORY_TYPE(FDeformMeshVertexFactory, "")