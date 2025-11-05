#include"PSOManager.h"
#include "LogManager.h"
#include <cassert>

using namespace GameEngine;

void PSOManager::Initialize(ID3D12Device* device, DXC* dxc) {
	LogManager::GetInstance().Log("Initialize PSOManager Start");
	device_ = device;
	dxc_ = dxc;
	// ラスタライザの全パターン生成
	rasterizerBuiler_.Initialize();
	// ブレンドモードの全パターン生成
	blendBuilder_.Initialize();
	// シェーダーコンパイラの初期唖k
	shaderCompiler_.Initialize(dxc);
	LogManager::GetInstance().Log("Initialize PSOManager End");
}

void PSOManager::RegisterPSO(const std::string& name, const CreatePSOData& psoData) {

	// 既に登録されていたら飛ばす
	if (psoList_.find(name) != psoList_.end()) {
		return;
	}

	CreatePSO(name,psoData);

	LogManager::GetInstance().Log("PSO registerd name : " + name);
}

void PSOManager::RegisterPSO(const std::string& name, const CreatePSOData& psoData, RootSignatureBuilder* rootSignature, InputLayoutBuilder* inputLayout) {

    // 既に登録されていたら飛ばす
    if (psoList_.find(name) != psoList_.end()) {
        return;
    }

    // シェーダーをコンパイル
    LogManager::GetInstance().Log("Compiling vertex shader");
    shaderCompiler_.CompileVsShader(psoData.vsPath);

    LogManager::GetInstance().Log("Compiling pixel shader");
    shaderCompiler_.CompilePsShader(psoData.psPath);

    IDxcBlob* vsBlob = shaderCompiler_.GetVertexShaderBlob();
    IDxcBlob* psBlob = shaderCompiler_.GetPixelShaderBlob();

    if (!vsBlob || !psBlob) {
        LogManager::GetInstance().Log("Shader compilation failed for: " + name);
        return;
    }

    // ルートシグネチャが登録されていなければ生成する
    if (rootSignatureList_.find(psoData.rootSigName) == rootSignatureList_.end()) {
        RootSignatureData rootSignatureData;
        rootSignatureData.rootSignature = rootSignature->GetRootSignature();
        rootSignatureData.parameterTypes = rootSignature->GetParameterTypes();
        // RootSignatureを保存
        rootSignatureList_[psoData.rootSigName] = rootSignatureData;
    }    

    // DepthStencilStateの設定
    D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
    depthStencilDesc.DepthEnable = true;
    depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

    // PSO設定
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
    psoDesc.pRootSignature = rootSignatureList_[psoData.rootSigName].rootSignature.Get();
    psoDesc.InputLayout = inputLayout->GetInputLayoutDesc();
    psoDesc.RasterizerState = rasterizerBuiler_.GetRasterizerDesc(DrawModel::FillFront);
    psoDesc.BlendState = blendBuilder_.GetBlendDesc(psoData.blendMode);
    psoDesc.VS = { vsBlob->GetBufferPointer(), vsBlob->GetBufferSize() };
    psoDesc.PS = { psBlob->GetBufferPointer(), psBlob->GetBufferSize() };
    psoDesc.DepthStencilState = depthStencilDesc;
    psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.SampleDesc.Count = 1;
    psoDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

    // PSOの生成
    PSOData pso;
    // リンクするルートシグネチャを保存
    pso.rootSigName = psoData.rootSigName;

    // 実際に生成
    HRESULT hr = device_->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso.graphicsPipelineState));
    assert(SUCCEEDED(hr));

    // PSOを保存
    psoList_[name] = pso;

    LogManager::GetInstance().Log("PSO registerd name : " + name);
}

void PSOManager::LoadFromJson(const std::string& fileName) {

	// 読み込みJSONファイルのフルパスを合成する
	std::string filePath = kDirectoryPath + fileName + ".json";
	// 読み込み用ファイルストリーム
	std::ifstream ifs;
	// ファイルを読み込み用に開く
	ifs.open(filePath);

	// ファイルオープン失敗
	if (ifs.fail()) {
		std::string message = "Failed open data file for load.";
		MessageBoxA(nullptr, message.c_str(), "PSOManager", 0);
		assert(0);
	}

    json root;

    // json文字列からjsonのデータ構造に展開
    ifs >> root;
    // ファイルを閉じる
    ifs.close();

	LogManager::GetInstance().Log("PSO registerd name : " + fileName);
}

void PSOManager::DefaultLoadPSO() {

    LogManager::GetInstance().Log("Loading default PSOs");

    // デフォルトの3Dオブジェクト用PSO
    CreatePSOData default3D;
    default3D.rootSigName = "Default3D";
    default3D.vsPath = L"Resources/Shaders/Object3d.VS.hlsl";
    default3D.psPath = L"Resources/Shaders/Object3d.PS.hlsl";
    default3D.drawMode = DrawModel::FillFront;
    default3D.blendMode = BlendMode::kBlendModeNormal;
    default3D.isDepthEnable = true;
    RootSignatureBuilder rootSigBuilder;
    rootSigBuilder.Initialize(device_);
    rootSigBuilder.AddCBVParameter(0, D3D12_SHADER_VISIBILITY_PIXEL);
    rootSigBuilder.AddCBVParameter(0, D3D12_SHADER_VISIBILITY_VERTEX);
    rootSigBuilder.AddSRVDescriptorTable(0, 1, D3D12_SHADER_VISIBILITY_PIXEL);
    rootSigBuilder.AddCBVParameter(1, D3D12_SHADER_VISIBILITY_PIXEL);
    rootSigBuilder.AddCBVParameter(2, D3D12_SHADER_VISIBILITY_PIXEL);
    rootSigBuilder.AddSampler(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_SHADER_VISIBILITY_PIXEL);
    rootSigBuilder.CreateRootSignature();
    InputLayoutBuilder inputLayoutBuilder;
    inputLayoutBuilder.CreateDefaultObjElement();
    RegisterPSO("Default3D", default3D, &rootSigBuilder, &inputLayoutBuilder);

    // 加算合成用PSO
    default3D.blendMode = BlendMode::kBlendModeAdd;
    RegisterPSO("Additive3D", default3D, &rootSigBuilder, &inputLayoutBuilder);

    // デフォルトのスプライト用PSO
    CreatePSOData defaultSprite;
    defaultSprite.rootSigName = "Default2D";
    defaultSprite.vsPath = L"Resources/Shaders/Sprite.VS.hlsl";
    defaultSprite.psPath = L"Resources/Shaders/Sprite.PS.hlsl";
    defaultSprite.drawMode = DrawModel::None;
    defaultSprite.blendMode = BlendMode::kBlendModeNormal;
    defaultSprite.isDepthEnable = false;
    RegisterPSO("DefaultSprite", defaultSprite);

    // インスタンシング描画用PSO
    CreatePSOData instancing3D;
    instancing3D.rootSigName = "Instancing3D";
    instancing3D.vsPath = L"Resources/Shaders/Particle.VS.hlsl";
    instancing3D.psPath = L"Resources/Shaders/particle.PS.hlsl";
    instancing3D.drawMode = DrawModel::FillFront;
    instancing3D.blendMode = BlendMode::kBlendModeNormal;
    instancing3D.isDepthEnable = true;
    RootSignatureBuilder instancingRootSigBuilder;
    instancingRootSigBuilder.Initialize(device_);
    instancingRootSigBuilder.AddCBVParameter(0, D3D12_SHADER_VISIBILITY_PIXEL);
    instancingRootSigBuilder.AddSRVDescriptorTable(0, 1, D3D12_SHADER_VISIBILITY_VERTEX);
    instancingRootSigBuilder.AddSRVDescriptorTable(0, 1, D3D12_SHADER_VISIBILITY_PIXEL);
    instancingRootSigBuilder.AddSampler(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_SHADER_VISIBILITY_PIXEL);
    instancingRootSigBuilder.CreateRootSignature();
    RegisterPSO("Instancing3D", instancing3D, &instancingRootSigBuilder, &inputLayoutBuilder);

    // インスタンシング描画の加算合成用PSO
    instancing3D.blendMode = BlendMode::kBlendModeAdd;
    RegisterPSO("AdditiveInstancing3D", instancing3D, &instancingRootSigBuilder, &inputLayoutBuilder);

    // グリッド描画用のPSO
    CreatePSOData grid;
    grid.rootSigName = "Grid";
    grid.vsPath = L"Resources/Shaders/Grid.VS.hlsl";
    grid.psPath = L"Resources/Shaders/Grid.PS.hlsl";
    grid.drawMode = DrawModel::None;
    grid.blendMode = BlendMode::kBlendModeNormal;
    grid.isDepthEnable = true;
    RootSignatureBuilder gridRootSigBuilder;
    gridRootSigBuilder.Initialize(device_);
    gridRootSigBuilder.AddCBVParameter(0, D3D12_SHADER_VISIBILITY_VERTEX);
    gridRootSigBuilder.AddCBVParameter(1, D3D12_SHADER_VISIBILITY_PIXEL);
    gridRootSigBuilder.CreateRootSignature();
    InputLayoutBuilder gridInputLayoutBuilder;
    gridInputLayoutBuilder.CreateGridElement();
    RegisterPSO("Grid", grid, &gridRootSigBuilder, &gridInputLayoutBuilder);

    // アニメーション描画用のPSO
    CreatePSOData animation;
    animation.rootSigName = "Animation";
    animation.vsPath = L"Resources/Shaders/SkinningObject3d.VS.hlsl";
    animation.psPath = L"Resources/Shaders/Object3d.PS.hlsl";
    animation.drawMode = DrawModel::None;
    animation.blendMode = BlendMode::kBlendModeNormal;
    animation.isDepthEnable = true;
    RootSignatureBuilder animationRootSigBuilder;
    animationRootSigBuilder.Initialize(device_);
    animationRootSigBuilder.AddCBVParameter(0, D3D12_SHADER_VISIBILITY_PIXEL);
    animationRootSigBuilder.AddCBVParameter(0, D3D12_SHADER_VISIBILITY_VERTEX);
    animationRootSigBuilder.AddSRVDescriptorTable(0, 1, D3D12_SHADER_VISIBILITY_PIXEL);
    animationRootSigBuilder.AddSRVDescriptorTable(0, 1, D3D12_SHADER_VISIBILITY_VERTEX);
    animationRootSigBuilder.AddCBVParameter(1, D3D12_SHADER_VISIBILITY_PIXEL);
    animationRootSigBuilder.AddCBVParameter(2, D3D12_SHADER_VISIBILITY_PIXEL);
    animationRootSigBuilder.AddSampler(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_SHADER_VISIBILITY_PIXEL);
    animationRootSigBuilder.CreateRootSignature();
    InputLayoutBuilder animationInputLayoutBuilder;
    animationInputLayoutBuilder.CreateDefaultAnimationElement();
    RegisterPSO("Animation", animation, &animationRootSigBuilder, &animationInputLayoutBuilder);

    LogManager::GetInstance().Log("Default PSOs loaded");
}

void PSOManager::CreatePSO(const std::string& psoName, const CreatePSOData& psoData) {
	LogManager::GetInstance().Log("PSO create start : " + psoName);

    // シェーダーをコンパイル
    LogManager::GetInstance().Log("Compiling vertex shader");
    shaderCompiler_.CompileVsShader(psoData.vsPath);

    LogManager::GetInstance().Log("Compiling pixel shader");
    shaderCompiler_.CompilePsShader(psoData.psPath);

    IDxcBlob* vsBlob = shaderCompiler_.GetVertexShaderBlob();
    IDxcBlob* psBlob = shaderCompiler_.GetPixelShaderBlob();

    if (!vsBlob || !psBlob) {
        LogManager::GetInstance().Log("Shader compilation failed for: " + psoName);
        return;
    }

    // ルートシグネチャが登録されていなければ生成する
    // RootSignatureの生成
    RootSignatureBuilder rootSigBuilder;
    if (rootSignatureList_.find(psoData.rootSigName) == rootSignatureList_.end()) {
        rootSigBuilder.Initialize(device_);
        rootSigBuilder.CreateRootSignatureFromReflection(dxc_->GetIDxcUtils(), vsBlob, psBlob);
        RootSignatureData rootSignatureData;
        rootSignatureData.rootSignature = rootSigBuilder.GetRootSignature();
        rootSignatureData.parameterTypes = rootSigBuilder.GetParameterTypes();
        // RootSignatureを保存
        rootSignatureList_[psoData.rootSigName] = rootSignatureData;
    }

    // InputLayoutの生成
    InputLayoutBuilder inputLayoutBuilder;
    inputLayoutBuilder.CreateInputLayoutFromReflection(dxc_->GetIDxcUtils(), vsBlob);
    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc = inputLayoutBuilder.GetInputLayoutDesc();

    // DepthStencilStateの設定
    D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
    if (psoData.isDepthEnable) {
        depthStencilDesc.DepthEnable = true;
        depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
        depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    } else {
        depthStencilDesc.DepthEnable = false;
    }

    // PSOの設定
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
    // InputLayout
    psoDesc.InputLayout = inputLayoutDesc;
    // RootSignature
    psoDesc.pRootSignature = rootSignatureList_[psoData.rootSigName].rootSignature.Get();
    // シェーダー
    psoDesc.VS = { vsBlob->GetBufferPointer(), vsBlob->GetBufferSize() };
    psoDesc.PS = { psBlob->GetBufferPointer(), psBlob->GetBufferSize() };
    // BlendState
    psoDesc.BlendState = blendBuilder_.GetBlendDesc(psoData.blendMode);
    // RasterizerState
    psoDesc.RasterizerState = rasterizerBuiler_.GetRasterizerDesc(psoData.drawMode);
    // DepthStencilState
    psoDesc.DepthStencilState = depthStencilDesc;
    psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    psoDesc.SampleDesc.Count = 1;
    psoDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
    // 描画タイプ
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

    // PSOの生成
    PSOData pso;
    // リンクするルートシグネチャを保存
    pso.rootSigName = psoData.rootSigName;

    // 実際に生成
    HRESULT hr = device_->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso.graphicsPipelineState));
    assert(SUCCEEDED(hr));

    // PSOを保存
    psoList_[psoName] = pso;

    LogManager::GetInstance().Log("PSO create end : " + psoName);
}

ID3D12RootSignature* PSOManager::GetRootSignature(const std::string& name) {
	auto it = rootSignatureList_.find(name);
	if (it != rootSignatureList_.end()) {
		return it->second.rootSignature.Get();
	}

	LogManager::GetInstance().Log("RootSignature not found: " + name);
	assert(0);
	return nullptr;
}

ID3D12PipelineState* PSOManager::GetPSO(const std::string& name) {
	auto it = psoList_.find(name);
	if (it != psoList_.end()) {
		return it->second.graphicsPipelineState.Get();
	}

	LogManager::GetInstance().Log("PSO not found: " + name);
	assert(0);
	return nullptr;
}

DrawPsoData PSOManager::GetDrawPsoData(const std::string& PsoName) const {

    auto pso = psoList_.find(PsoName);
    if (pso == psoList_.end()) {
        assert(0);
    }

    auto root = rootSignatureList_.find(pso->second.rootSigName);
    if (root == rootSignatureList_.end()) {
        assert(0);
    }

    DrawPsoData drawData;
    drawData.rootSignature = root->second.rootSignature.Get();
    drawData.graphicsPipelineState = pso->second.graphicsPipelineState.Get();
    return drawData;
}