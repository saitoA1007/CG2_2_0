#include"InputLayoutBuilder.h"
#include <d3d12shader.h>
#include<cassert>
using namespace GameEngine;

void InputLayoutBuilder::CreateInputElement(const std::string& name, uint32_t index, uint32_t slotIndex, DXGI_FORMAT format) {
	// InputLayout
	D3D12_INPUT_ELEMENT_DESC inputElementDescs = {};
	inputElementDescs.SemanticName = name.c_str();
	inputElementDescs.SemanticIndex = index;
	inputElementDescs.InputSlot = slotIndex;
	inputElementDescs.Format = format;
	inputElementDescs.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	// 格納する
	inputElementDescs_.push_back(std::move(inputElementDescs));
}

void InputLayoutBuilder::CreateInputLayoutDesc() {
	inputLayoutDesc_.pInputElementDescs = inputElementDescs_.data();
	inputLayoutDesc_.NumElements = static_cast<UINT>(inputElementDescs_.size());
}

void InputLayoutBuilder::Reset() {
	inputElementDescs_.clear();
}

void InputLayoutBuilder::CreateDefaultObjElement() {
	CreateInputElement("POSITION", 0,0, DXGI_FORMAT_R32G32B32A32_FLOAT);
	CreateInputElement("TEXCOORD", 0, 0, DXGI_FORMAT_R32G32_FLOAT);
	CreateInputElement("NORMAL", 0, 0, DXGI_FORMAT_R32G32B32_FLOAT);
	CreateInputLayoutDesc();
}

void InputLayoutBuilder::CreateDefaultSpriteElement() {
	CreateInputElement("POSITION", 0, 0, DXGI_FORMAT_R32G32B32A32_FLOAT);
	CreateInputElement("TEXCOORD", 0, 0, DXGI_FORMAT_R32G32_FLOAT);
	CreateInputLayoutDesc();
}

void InputLayoutBuilder::CreateDefaultLineElement() {
	CreateInputElement("POSITION", 0, 0, DXGI_FORMAT_R32G32B32A32_FLOAT);
	CreateInputElement("COLOR", 0, 0, DXGI_FORMAT_R32G32B32A32_FLOAT);
	CreateInputLayoutDesc();
}

void InputLayoutBuilder::CreateDefaultAnimationElement() {
	CreateInputElement("POSITION", 0, 0, DXGI_FORMAT_R32G32B32A32_FLOAT);
	CreateInputElement("TEXCOORD", 0, 0, DXGI_FORMAT_R32G32_FLOAT);
	CreateInputElement("NORMAL", 0, 0, DXGI_FORMAT_R32G32B32_FLOAT);
	CreateInputElement("WEIGHT", 0, 1, DXGI_FORMAT_R32G32B32A32_FLOAT);
	CreateInputElement("INDEX", 0, 1, DXGI_FORMAT_R32G32B32A32_SINT);
	CreateInputLayoutDesc();
}

void InputLayoutBuilder::CreateInputLayoutFromReflection(IDxcUtils* utils,IDxcBlob* vsBlob) {
    DxcBuffer reflectionBuffer{};
    reflectionBuffer.Ptr = vsBlob->GetBufferPointer();
    reflectionBuffer.Size = vsBlob->GetBufferSize();
    reflectionBuffer.Encoding = DXC_CP_ACP;

    Microsoft::WRL::ComPtr<ID3D12ShaderReflection> reflection;
    HRESULT hr = utils->CreateReflection(&reflectionBuffer, IID_PPV_ARGS(&reflection));
    assert(SUCCEEDED(hr));

    D3D12_SHADER_DESC shaderDesc{};
    reflection->GetDesc(&shaderDesc);

    inputElementDescs_.clear();
    semanticNames_.clear();

    // 入力パラメータを解析
    for (UINT i = 0; i < shaderDesc.InputParameters; ++i) {
        D3D12_SIGNATURE_PARAMETER_DESC paramDesc{};
        reflection->GetInputParameterDesc(i, &paramDesc);

        D3D12_INPUT_ELEMENT_DESC elementDesc{};

        // セマンティック名を保存
        semanticNames_.push_back(paramDesc.SemanticName);
        elementDesc.SemanticName = semanticNames_.back().c_str();
        elementDesc.SemanticIndex = paramDesc.SemanticIndex;
        elementDesc.InputSlot = 0; // デフォルトはスロット0
        elementDesc.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

        // コンポーネントタイプとマスクからフォーマットを決定
        if (paramDesc.Mask == 1) {
            if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) { 
                elementDesc.Format = DXGI_FORMAT_R32_UINT;
            } else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) { 
                elementDesc.Format = DXGI_FORMAT_R32_SINT;
            } else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) {
                elementDesc.Format = DXGI_FORMAT_R32_FLOAT;
            };
        } else if (paramDesc.Mask <= 3) {
            if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) { 
                elementDesc.Format = DXGI_FORMAT_R32G32_UINT;
            }
            else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) { 
                elementDesc.Format = DXGI_FORMAT_R32G32_SINT;
            }
            else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) { 
                elementDesc.Format = DXGI_FORMAT_R32G32_FLOAT; 
            }
        } else if (paramDesc.Mask <= 7) {
            if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) {
                elementDesc.Format = DXGI_FORMAT_R32G32B32_UINT; 
            }
            else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) { 
                elementDesc.Format = DXGI_FORMAT_R32G32B32_SINT;
            }
            else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) {
                elementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
            }
        } else if (paramDesc.Mask <= 15) {
            if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) {
                elementDesc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
            }
            else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) { 
                elementDesc.Format = DXGI_FORMAT_R32G32B32A32_SINT;
            }
            else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) {
                elementDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
            }
        }

        inputElementDescs_.push_back(elementDesc);
    }

    // 入力レイアウトDescを作成
    CreateInputLayoutDesc();
}