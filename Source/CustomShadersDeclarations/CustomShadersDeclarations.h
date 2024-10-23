#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

class CUSTOMSHADERSDECLARATIONS_API FCustomShadersDeclarationsModule : public IModuleInterface
{
public:
	static inline FCustomShadersDeclarationsModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FCustomShadersDeclarationsModule>("CustomShadersDeclarations");
	}

	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("CustomShadersDeclarations");
	}

public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};