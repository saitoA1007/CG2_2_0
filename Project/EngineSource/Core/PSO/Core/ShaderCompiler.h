#pragma once
#include"DXC.h"

namespace GameEngine {

	class ShaderCompiler {
	public:

		// コンパイルするシェーダータイプ
		enum class Type {
			VS,
			PS,
			Cs,

			Count
		};

		/// <summary>
		/// 初期化
		/// </summary>
		/// <param name="dxc"></param>
		void Initialize(DXC* dxc);

		// コンパイルするシェーダー
		Microsoft::WRL::ComPtr<IDxcBlob> CompileShader(Type type, const std::wstring& path);

	private:

		DXC* dxc_ = nullptr;

		// コンパイルタイプ
		std::wstring compileTypes[static_cast<size_t>(Type::Count)] = {
			L"vs_6_0",
			L"ps_6_0",
			L"cs_6_0"
		};
	};
}