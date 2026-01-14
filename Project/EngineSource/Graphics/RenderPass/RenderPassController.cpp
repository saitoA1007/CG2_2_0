#include"RenderPassController.h"
#include<cassert>
using namespace GameEngine;

void RenderPassController::Initialize(RenderTextureManager* renderTextureManager, ID3D12GraphicsCommandList* commandList) {

	commandList_ = commandList;

	// レンダーを取得する
	renderTextureManager_ = renderTextureManager;
}

void RenderPassController::AddPass(const std::string& name, bool isDepth) {
	// すでに登録されている場合、早期リターン
	auto getName = renderPassList_.find(name);
	if (getName != renderPassList_.end()) {
		return;
	}

	// renderTextureを作成
	renderTextureManager_->Create(name, isDepth);
	RenderTexture* renderTex = renderTextureManager_->GetRenderTexture(name);

	// 生成リソースを作成
	RenderPassContext context;
	context.commandList = commandList_;
	context.isDepth = isDepth;

	// レンダーパスを作成
	std::unique_ptr<RenderPass> tmp = std::make_unique<RenderPass>(name, &context, renderTex);

	// 登録
	renderPassList_[name] = std::move(tmp);
}

void RenderPassController::PrePass(const std::string& name) {
	// 登録されていなければエラー
	auto render = renderPassList_.find(name);
	if (render == renderPassList_.end()) {
		assert(false && "Not found RenderPass");
	}

	// 描画前処理
	render->second->PrePass();
}

void RenderPassController::PostPass(const std::string& name) {
	// 登録されていなければエラー
	auto render = renderPassList_.find(name);
	if (render == renderPassList_.end()) {
		assert(false && "Not found RenderPass");
	}

	// 描画後処理
	render->second->PostPass();
}

void RenderPassController::SetEndPass(const std::string& name) {
	// 登録されていなければエラー
	auto render = renderPassList_.find(name);
	if (render == renderPassList_.end()) {
		assert(false && "Not found RenderPass");
	}

	// 最終的な描画先を設定
	resultPassName_ = name;
	resultSrvHandle_ = render->second->GetSrvHandle();
}

CD3DX12_GPU_DESCRIPTOR_HANDLE RenderPassController::GetFinalOutputSRV() {
	return resultSrvHandle_;
}