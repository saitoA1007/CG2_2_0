#include"SpritePSO.h"
#include<cassert>
using namespace GameEngine;

void SpritePSO::Initialize(const std::wstring& vsPath, const std::wstring& psPath, ID3D12Device* device, DXC* dxc, LogManager* logManager) {

	// ログを取得
	logManager_ = logManager;

	// 初期化を開始するログ
	if (logManager_) {
		logManager_->Log("SpritePSO Class start Initialize\n");
	}

	// RootSignature作成
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0; // 0から始まる
	descriptorRange[0].NumDescriptors = 1; // 数は1つ
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; // SRVを使う
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; // offsetを自動計算

	// RootParameter作成。PixelShaderのMaterialとVertexShaderのTransform
	D3D12_ROOT_PARAMETER rootParameters[2] = {};
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;  // CBVを使う
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;  // 全てのShaderで使う
	rootParameters[0].Descriptor.ShaderRegister = 0;  // レジスタ番号0
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // DescriptorTableを使う
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う
	rootParameters[1].DescriptorTable.pDescriptorRanges = descriptorRange; // Tableの中身の配列を指定
	rootParameters[1].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange); // Tableで利用する数
	descriptionRootSignature.pParameters = rootParameters;  // ルートパラメータ配列へのポインタ
	descriptionRootSignature.NumParameters = _countof(rootParameters);  // 配列の長さ

	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR; // バイリニアフィルタ
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP; //0-1の範囲外をリピート 
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER; // 比較しない
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX; // ありったけのMIpmapを使う
	staticSamplers[0].ShaderRegister = 0; // レジスタ番号0を使う
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う
	descriptionRootSignature.pStaticSamplers = staticSamplers;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);

	// シリアライズしてバイナリにする
	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob, errorBlob;

	HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature,
		D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		logManager_->Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}
	// バイナリを元に生成
	hr = device->CreateRootSignature(0,
		signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(),
		IID_PPV_ARGS(&rootSignature_));
	assert(SUCCEEDED(hr));

	// InputLayout
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[2] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	// RasiterzerStateの設定
	// RasiterzerStateの設定
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	// 線には不要なのでNone
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	// Zクリップを有効
	rasterizerDesc.DepthClipEnable = TRUE;

	// Shaderをコンパイルする
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob, pixelShaderBlob;

	vertexShaderBlob = dxc->CompileShader(vsPath,
		L"vs_6_0", dxc->dxcUtils_.Get(), dxc->dxcCompiler_.Get(), dxc->includeHandler_.Get());
	assert(vertexShaderBlob != nullptr);

	pixelShaderBlob = dxc->CompileShader(psPath,
		L"ps_6_0", dxc->dxcUtils_.Get(), dxc->dxcCompiler_.Get(), dxc->includeHandler_.Get());
	assert(pixelShaderBlob != nullptr);

	// DepthStencilStateの設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	// Depthの機能を有効化する
	depthStencilDesc.DepthEnable = true;
	// 書き込みします
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	// 比較関数はLessEqual。つまり、近ければ描画される
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	for (uint32_t i = 0; i < BlendMode::kCountOfBlendMode; ++i) {
		// すべての色要素を書き込む
		blendDesc_[i].RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

		// ブレンドモードの有効化
		if (i != kBlendModeNone) {
			blendDesc_[i].RenderTarget[0].BlendEnable = TRUE; // ブレンドを有効化
			blendDesc_[i].RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE; // アルファ値のソース
			blendDesc_[i].RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD; // アルファ値の加算ブレンド
			blendDesc_[i].RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO; // アルファ値のデスティネーション
		}

		switch (i) {

		case kBlendModeNormal:
			blendDesc_[i].RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA; // SrcA
			blendDesc_[i].RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD; // 加算ブレンド
			blendDesc_[i].RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA; // (1-SrcA)
			break;

		case kBlendModeAdd:
			blendDesc_[i].RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA; // SrcA
			blendDesc_[i].RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD; // 加算ブレンド
			blendDesc_[i].RenderTarget[0].DestBlend = D3D12_BLEND_ONE; // (1-SrcA)
			break;

		case kBlendModeSubtract:
			blendDesc_[i].RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA; // SrcA
			blendDesc_[i].RenderTarget[0].BlendOp = D3D12_BLEND_OP_REV_SUBTRACT; // 加算ブレンド
			blendDesc_[i].RenderTarget[0].DestBlend = D3D12_BLEND_ONE; // (1-SrcA)
			break;

		case kBlendModeMultily:
			blendDesc_[i].RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA; // SrcA
			blendDesc_[i].RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD; // 加算ブレンド
			blendDesc_[i].RenderTarget[0].DestBlend = D3D12_BLEND_SRC_COLOR; // (1-SrcA)
			break;

		case kBlendModeScreen:
			blendDesc_[i].RenderTarget[0].SrcBlend = D3D12_BLEND_INV_DEST_COLOR; // SrcA
			blendDesc_[i].RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD; // 加算ブレンド
			blendDesc_[i].RenderTarget[0].DestBlend = D3D12_BLEND_ONE; // (1-SrcA)
			break;
		}
	}

	// PSO設定
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rootSignature_.Get();// RootSignature
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;// InputLayout
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc; // RasterizerState
	graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize() };// VertexShader
	graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize() };// PixelShader
	// DepthStencilの設定
	graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	// 書き込むRTVの情報
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	// 利用するトポロジ（形状）のタイプ。三角形
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	// どのように画面に色を打ち込んむかの設定（気にしなくて良い）
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	for (uint32_t i = 0; i < BlendMode::kCountOfBlendMode; ++i) {

		// 各ブレンドモードを設定
		graphicsPipelineStateDesc.BlendState = blendDesc_[i];

		// 実際に生成
		hr = device->CreateGraphicsPipelineState(&graphicsPipelineStateDesc,
			IID_PPV_ARGS(&graphicsPipelineState_[i]));
		assert(SUCCEEDED(hr));
	}

	// 初期化を終了するログ
	if (logManager_) {
		logManager_->Log("SpritePSO Class End Initialize\n");
	}
}
