#pragma once
#include <d3d12.h>
#include<dxcapi.h>
#include <wrl.h>
#include"EngineSource/Common/LogManager.h"

namespace GameEngine {

	class DXC final {
	public:

		/// <summary>
		/// 初期化
		/// </summary>
		void Initialize(LogManager* logManager);

		Microsoft::WRL::ComPtr<IDxcBlob> CompileShader(
			// CompilerするShaderファイルへのパス
			const std::wstring& filePath,
			// Compilerに使用するProfile
			const wchar_t* profile,
			// 初期化で生成したものを3つ
			IDxcUtils* dxcUtils,
			IDxcCompiler3* dxcCompiler,
			IDxcIncludeHandler* includeHandler);

		Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils_;
		Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler_;
		Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler_;
	private:

		// ログ
		LogManager* logManager_;
	};
}