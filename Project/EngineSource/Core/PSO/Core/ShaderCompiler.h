#pragma once
#include"DXC.h"

namespace GameEngine {

	class ShaderCompiler {
	public:

		void Initialize(DXC* dxc);

		void CompileVsShader(const std::wstring& vsPath);
		void CompilePsShader(const std::wstring& psPath);

		IDxcBlob* GetVertexShaderBlob() const { return vertexShaderBlob_.Get(); }
		IDxcBlob* GetPixelShaderBlob() const { return pixelShaderBlob_.Get(); }

	private:

		DXC* dxc_ = nullptr;

		Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob_;
		Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob_;
	};
}