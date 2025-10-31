#pragma once
#include <d3d12.h>
#include<dxcapi.h>
#include <wrl.h>
#include<unordered_map>
#include<string>

#include"DXC.h"

#include"ShaderCompiler.h"
#include"RootSignatureBuilder.h"
#include"InputLayoutBuilder.h"

// 本来は外から持ってくるもの達
#include"RasterizerBuilder.h"
#include"BlendBuilder.h"

namespace GameEngine {

	class PSOManager {
	public:

		// PSOを作成するためのデータ
		struct PSOData {
			std::wstring vsPath = L"vsPath"; // 頂点シェーダーのパス
			std::wstring psPath = L"psPath"; // ピクセルシェーダーのパス
			DrawModel drawMode = DrawModel::FillFront; // 描画モード
			BlendMode blendMode = BlendMode::kBlendModeNormal; // ブレンドモード
			bool isDepthEnable = true; // 深度の使用
		};

	public:

		// 初期化処理
		void Initialize(ID3D12Device* device, DXC* dxc);

		void RegisterPSO(const std::string& name);

		void LoadFromJson(const std::string& filePath);

		void DefaultLoadPSO();

	private:

		ID3D12Device* device_ = nullptr;
		DXC* dxc_ = nullptr;

		// PSOのリスト
		std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12PipelineState>> psoList_;

		// ルートシグネチャリスト
		std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12RootSignature>> rootSignatureList_;

		// ラスタライザを作成
		RasterizerBuilder rasterizerBuiler_;

		// ブレンドモードの設定
		BlendBuilder blendBuilder_;

	private:

		void CreatePSO(const std::string& name);

		// PSOを作成する
		void CreatePSO(const std::string& psoName, PSOData psoData);

	};
}

