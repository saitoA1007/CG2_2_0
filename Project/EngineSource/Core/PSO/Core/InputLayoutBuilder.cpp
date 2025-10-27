#include"InputLayoutBuilder.h"

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