#include"ShaderCompiler.h"
#include <cassert>
using namespace GameEngine;

void ShaderCompiler::Initialize(DXC* dxc) {
	dxc_ = dxc;
}

void ShaderCompiler::CompileVsShader(const std::wstring& vsPath) {
	// Shaderをコンパイルする
	vertexShaderBlob_ = dxc_->CompileShader(vsPath,
		L"vs_6_0", dxc_->dxcUtils_.Get(), dxc_->dxcCompiler_.Get(), dxc_->includeHandler_.Get());
	assert(vertexShaderBlob_ != nullptr);
}

void ShaderCompiler::CompilePsShader(const std::wstring& psPath) {
	// Shaderをコンパイルする
	pixelShaderBlob_ = dxc_->CompileShader(psPath,
		L"ps_6_0", dxc_->dxcUtils_.Get(), dxc_->dxcCompiler_.Get(), dxc_->includeHandler_.Get());
	assert(pixelShaderBlob_ != nullptr);
}

void ShaderCompiler::CompileCsShader(const std::wstring& csPath) {
	// Shaderをコンパイルする
	csShaderBlob_ = dxc_->CompileShader(csPath,
		L"cs_6_0", dxc_->dxcUtils_.Get(), dxc_->dxcCompiler_.Get(), dxc_->includeHandler_.Get());
	assert(csShaderBlob_ != nullptr);
}

Microsoft::WRL::ComPtr<IDxcBlob> ShaderCompiler::CompileShader(const std::wstring& Path, CompileShaderType type) {
	Microsoft::WRL::ComPtr<IDxcBlob> shaderBlob_;

	// Shaderをコンパイルする
	shaderBlob_ = dxc_->CompileShader(Path,
		paths_[static_cast<size_t>(type)].c_str(), dxc_->dxcUtils_.Get(), dxc_->dxcCompiler_.Get(), dxc_->includeHandler_.Get());
	assert(shaderBlob_ != nullptr);
	return shaderBlob_;
}