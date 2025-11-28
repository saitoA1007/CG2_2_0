#pragma once
#include"DXC.h"

namespace GameEngine {

	class ShaderCompiler {
	public:

		// 読み込むシェーダー
		enum class CompileShaderType {
			VS,
			PS,
			CS,

			Count
		};

		void Initialize(DXC* dxc);

		void CompileVsShader(const std::wstring& vsPath);
		void CompilePsShader(const std::wstring& psPath);

		void CompileCsShader(const std::wstring& csPath);

		IDxcBlob* GetVertexShaderBlob() const { return vertexShaderBlob_.Get(); }
		IDxcBlob* GetPixelShaderBlob() const { return pixelShaderBlob_.Get(); }

		IDxcBlob* GetCsShaderBlob() const { return csShaderBlob_.Get(); }

		Microsoft::WRL::ComPtr<IDxcBlob> CompileShader(const std::wstring& Path, CompileShaderType type);

	private:

		DXC* dxc_ = nullptr;

		Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob_;
		Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob_;

		Microsoft::WRL::ComPtr<IDxcBlob> csShaderBlob_;

		std::wstring paths_[static_cast<size_t>(CompileShaderType::Count)] = {
			L"vs_6_0",
			L"ps_6_0",
			L"cs_6_0"
		};
	};
}