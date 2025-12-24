#include"RenderTexture.h"

using namespace GameEngine;

void RenderTexture::Initialize(RenderTextureContext& context) {
	rtvIndex_ = context.rtvIndex;
	srvIndex_ = context.srvIndex;
	width_ = context.width;
	height_ = context.height;
	resource_ = context.resource;
	srvGpuHandle_ = context.srvGpuHandle;
}

void RenderTexture::TransitionToRenderTarget(ID3D12GraphicsCommandList* commandList) {
	if (currentState_ == D3D12_RESOURCE_STATE_RENDER_TARGET) {
		return;
	}

	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = resource_;
	barrier.Transition.StateBefore = currentState_;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

	commandList->ResourceBarrier(1, &barrier);
	currentState_ = D3D12_RESOURCE_STATE_RENDER_TARGET;
}

void RenderTexture::TransitionToShaderResource(ID3D12GraphicsCommandList* commandList) {
	if (currentState_ == D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE) {
		return;
	}

	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = resource_;
	barrier.Transition.StateBefore = currentState_;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

	commandList->ResourceBarrier(1, &barrier);
	currentState_ = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
}
