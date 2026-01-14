#include"RenderTexture.h"

using namespace GameEngine;

void RenderTexture::Initialize(RenderTextureContext& context) {
	rtvIndex_ = context.rtvIndex;
	srvIndex_ = context.srvIndex;
	width_ = context.width;
	height_ = context.height;
	resource_ = context.resource;
	rtvHandle_ = context.rtvHandle;
	srvGpuHandle_ = context.srvGpuHandle;

	isDepth_ = context.isDepth;

	if (context.isDepth) {
		depthResource_ = context.dsvResource;
		dsvIndex_ = context.dsvIndex;
		dsvHaveSrvIndex_ = context.dsvHaveSrvIndex;
		dsvHandle_ = context.dsvHandle;
	}
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

	// 深度が設定されていなければ早期リターン
	if (!isDepth_) { return; }

	D3D12_RESOURCE_BARRIER depthbarrier{};
	depthbarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	depthbarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	depthbarrier.Transition.pResource = depthResource_;
	depthbarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	depthbarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	depthbarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_DEPTH_WRITE;
	commandList->ResourceBarrier(1, &depthbarrier);
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

	// 深度が設定されていなければ早期リターン
	if (!isDepth_) { return; }

	D3D12_RESOURCE_BARRIER depthbarrier{};
	depthbarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	depthbarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	depthbarrier.Transition.pResource = depthResource_;
	depthbarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	depthbarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_DEPTH_WRITE;
	depthbarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	commandList->ResourceBarrier(1, &depthbarrier);
}
