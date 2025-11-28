#include"ShaderCompiler.h"
#include <cassert>
using namespace GameEngine;

void ShaderCompiler::Initialize(DXC* dxc) {
	dxc_ = dxc;
}

Microsoft::WRL::ComPtr<IDxcBlob> ShaderCompiler::CompileShader(Type type, const std::wstring& path) {
	Microsoft::WRL::ComPtr<IDxcBlob> shaderBlob;

	// Shaderをコンパイルする
	shaderBlob = dxc_->CompileShader(path,
		compileTypes[static_cast<size_t>(type)].c_str(), dxc_->dxcUtils_.Get(), dxc_->dxcCompiler_.Get(), dxc_->includeHandler_.Get());
	assert(shaderBlob != nullptr);
	return shaderBlob;
}