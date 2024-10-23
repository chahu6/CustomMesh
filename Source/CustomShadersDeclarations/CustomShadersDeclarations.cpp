#include "CustomShadersDeclarations.h"
#include "Modules/ModuleManager.h"

IMPLEMENT_GAME_MODULE(FCustomShadersDeclarationsModule, CustomShadersDeclarations);

void FCustomShadersDeclarationsModule::StartupModule()
{
	FString ShaderDirectory = FPaths::Combine(FPaths::ProjectDir(), TEXT("Shaders/Private"));
	AddShaderSourceDirectoryMapping("/CustomShaders", ShaderDirectory);
}

void FCustomShadersDeclarationsModule::ShutdownModule()
{

}
