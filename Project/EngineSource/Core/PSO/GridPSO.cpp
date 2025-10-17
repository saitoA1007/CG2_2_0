#include"GridPSO.h"
#include<cassert>
#include"LogManager.h"
using namespace GameEngine;

void GridPSO::Initialize(const std::wstring& vsPath, const std::wstring& psPath, ID3D12Device* device, DXC* dxc) {

	// 初期化を開始するログ
	LogManager::GetInstance().Log("GridPSO Class start Initialize");

	// RootSignature作成
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	// RootParameter作成。
	D3D12_ROOT_PARAMETER rootParameters[2] = {};
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // CBVを使う
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX; // VertexShaderで使う
	rootParameters[0].Descriptor.ShaderRegister = 0; // レジスタ番号0を使う
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // CBVを使う
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う
	rootParameters[1].Descriptor.ShaderRegister = 1; // レジスタ番号1を使う
	descriptionRootSignature.pParameters = rootParameters;  // ルートパラメータ配列へのポインタ
	descriptionRootSignature.NumParameters = _countof(rootParameters);  // 配列の長さ
	// Samplerを使用しないので0にする
	descriptionRootSignature.pStaticSamplers = nullptr;
	descriptionRootSignature.NumStaticSamplers = 0;

	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob, errorBlob;

	// GPUに伝える為にシリアライズしてバイナリにする
	HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature,
		D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		LogManager::GetInstance().Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}
	// バイナリを元に生成
	hr = device->CreateRootSignature(0,
		signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(),
		IID_PPV_ARGS(&rootSignature_));
	assert(SUCCEEDED(hr));

	// InputLayout
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[1] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	// Blendstateの設定
	D3D12_BLEND_DESC blendDesc{};
	// 透明度(アルファ値)を適応されるようにする
	blendDesc.RenderTarget[0].BlendEnable = TRUE; // ブレンドを有効化
	blendDesc.RenderTarget[0].LogicOpEnable = FALSE; // 論理演算を無効化
	blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA; // ソースのアルファ値を使用
	blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA; // 1 - ソースアルファ値を使用
	blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD; // 加算ブレンド
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE; // アルファ値のソース
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO; // アルファ値のデスティネーション
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD; // アルファ値の加算ブレンド
	// すべての色要素を書き込む
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	// RasiterzerStateの設定
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	// デバック用グリッドには不要なのでNone
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	// Zクリップを有効
	rasterizerDesc.DepthClipEnable = TRUE;

	// Shaderをコンパイルする
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob, pixelShaderBlob;

	// Shaderをコンパイルする
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

	// PSO設定
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rootSignature_.Get();// RootSignature
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;// InputLayout
	graphicsPipelineStateDesc.BlendState = blendDesc; // BlendState
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc; // RasterizerState
	graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize() };// VertexShader
	graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize() };// PixelShader
	// DepthStencilの設定
	graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	// 書き込むRTVの情報
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	// 使用するトロポジ(形状)のタイプ。三角形
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	// どのように画面に色を打ち込んむかの設定
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	// 実際に生成
	hr = device->CreateGraphicsPipelineState(&graphicsPipelineStateDesc,
		IID_PPV_ARGS(&graphicsPipelineState_));
	assert(SUCCEEDED(hr));

	// 初期化を終了するログ
	LogManager::GetInstance().Log("GridPSO Class End Initialize\n");
}
