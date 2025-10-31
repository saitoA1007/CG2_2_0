#include"test.h"
//#include "ConvertString.h"
//#include <format>
//#include <cassert>
//#include "LogManager.h"
//#include <d3d12shader.h>
//
//using namespace GameEngine;
//
//void TrianglePSO::Initialize(const std::wstring& vsPath, const std::wstring& psPath, ID3D12Device* device, DXC* dxc) {
//
//    LogManager::GetInstance().Log("TrianglePSO Class start Initialize (with Shader Reflection)");
//
//    // シェーダーをコンパイル
//    ShaderCompiler shaderCompiler;
//    shaderCompiler.Initialize(dxc);
//    shaderCompiler.CompileVsShader(vsPath);
//    shaderCompiler.CompilePsShader(psPath);
//
//    // ルートシグネチャを作成
//    RootSignatureBuilder rootSigBuilder;
//    rootSigBuilder.Initialize(device);
//    rootSigBuilder.CreateRootSignatureFromReflection(dxc->GetIDxcUtils(),shaderCompiler.GetVertexShaderBlob(), shaderCompiler.GetPixelShaderBlob());
//    rootSignature_ = rootSigBuilder.GetRootSignature();
//    
//    // InputLayoutを作成
//    InputLayoutBuilder inputLayoutBuilder;
//    inputLayoutBuilder.CreateInputLayoutFromReflection(dxc->GetIDxcUtils(),shaderCompiler.GetVertexShaderBlob());
//
//    // ラスタライザを作成
//    RasterizerBuilder rasterizerBuiler;
//    rasterizerBuiler.Initialize();
//
//    // ブレンドモードの設定
//    BlendBuilder blendBuilder;
//    blendBuilder.Initialize();
//
//    // DepthStencilStateの設定
//    D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
//    depthStencilDesc.DepthEnable = true;
//    depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
//    depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
//
//    // PSO設定
//    D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
//    graphicsPipelineStateDesc.pRootSignature = rootSignature_.Get();
//    graphicsPipelineStateDesc.InputLayout = inputLayoutBuilder.GetInputLayoutDesc();
//    graphicsPipelineStateDesc.RasterizerState = rasterizerBuiler.GetRasterizerDesc(DrawModel::FillFront);
//    graphicsPipelineStateDesc.VS = { shaderCompiler.GetVertexShaderBlob()->GetBufferPointer(), shaderCompiler.GetVertexShaderBlob()->GetBufferSize() };
//    graphicsPipelineStateDesc.PS = { shaderCompiler.GetPixelShaderBlob()->GetBufferPointer(), shaderCompiler.GetPixelShaderBlob()->GetBufferSize() };
//    graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
//    graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
//    graphicsPipelineStateDesc.NumRenderTargets = 1;
//    graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
//    graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
//    graphicsPipelineStateDesc.SampleDesc.Count = 1;
//    graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
//   
//    HRESULT hr;
//    for (uint32_t i = 0; i < BlendMode::kCountOfBlendMode; ++i) {
//        graphicsPipelineStateDesc.BlendState = blendBuilder.GetBlendDesc(static_cast<BlendMode>(i));
//        hr = device->CreateGraphicsPipelineState(&graphicsPipelineStateDesc,
//            IID_PPV_ARGS(&graphicsPipelineState_[i]));
//        assert(SUCCEEDED(hr));
//    }
//    
//    // フレーム描画用PSO
//    graphicsPipelineStateDesc.BlendState = blendBuilder.GetBlendDesc(BlendMode::kBlendModeNormal);
//
//    for (uint32_t i = 0; i < DrawModel::kCountOfDrawMode; ++i) {
//        graphicsPipelineStateDesc.RasterizerState = rasterizerBuiler.GetRasterizerDesc(static_cast<DrawModel>(i));
//        hr = device->CreateGraphicsPipelineState(&graphicsPipelineStateDesc,
//            IID_PPV_ARGS(&frameGraphicsPipelineState_[i]));
//        assert(SUCCEEDED(hr));
//    }
//
//    LogManager::GetInstance().Log("TrianglePSO Class End Initialize\n");
//}