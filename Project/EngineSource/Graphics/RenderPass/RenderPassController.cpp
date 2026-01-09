#include"RenderPassController.h"
#include<cassert>
using namespace GameEngine;

void RenderPassController::Initialize(RenderTextureManager* renderTextureManager) {

	// レンダーを取得する
	renderTextureManager_ = renderTextureManager;
}

void RenderPassController::AddPass(const std::string& name) {
	// すでに登録されている場合、早期リターン
	auto getName = renderPassList_.find(name);
	if (getName != renderPassList_.end()) {
		return;
	}

	// レンダーパスを作成
	std::unique_ptr<RenderPass> tmp = std::make_unique<RenderPass>(name);

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