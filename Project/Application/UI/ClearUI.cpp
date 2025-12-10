#include"ClearUI.h"
#include"GameParamEditor.h"
#include"AudioManager.h"
using namespace GameEngine;

void ClearUI::Initialize(GameEngine::InputCommand* inputCommand, GameEngine::TextureManager* textureManager) {
	inputCommand_ = inputCommand;

	// クリア文字
	clearTextSprite_ = Sprite::Create({0.0f,0.0f}, {100.0f,100.0f}, {0.5f,0.5f}, {1.0f,1.0f,1.0f,1.0f});
	clearTextGH_ = textureManager->GetHandleByName("clearText.png");

	// 操作文字
	guideSprite_ = Sprite::Create({ 0.0f,0.0f }, { 100.0f,100.0f }, { 0.5f,0.5f }, { 1.0f,1.0f,1.0f,1.0f });
	guideGH_ = textureManager->GetHandleByName("spaceText.png");

	// 背景画像
	bgSprite_ = Sprite::Create({ 0.0f,0.0f }, { 1280.0f,720.0f }, { 0.0f,0.0f }, { 0.0f,0.0f,0.0f,0.5f });

	// 選択音声を取得
	selectSH_ = AudioManager::GetInstance().GetHandleByName("Select.png");

#ifdef _DEBUG
	RegisterBebugParam();
#endif
	ApplyDebugParam();
}

void ClearUI::Update() {
#ifdef _DEBUG
	ApplyDebugParam();
#endif

	if (!isActive_) { return; }

	if (inputCommand_->IsCommandActive("Start")) {
		// タイトルに戻る
		if (backTitle_) {
			AudioManager::GetInstance().Play(selectSH_, 0.8f, false);
			backTitle_();
		}
	}
}

void ClearUI::RegisterBebugParam() {
	GameParamEditor::GetInstance()->AddItem(kGroupName_, "ClearTextPos", clearTextSprite_->position_);
	GameParamEditor::GetInstance()->AddItem(kGroupName_, "ClearTextSize", clearTextSprite_->size_);
	GameParamEditor::GetInstance()->AddItem(kGroupName_, "GuidePos", guideSprite_->position_);
	GameParamEditor::GetInstance()->AddItem(kGroupName_, "GuideSize", guideSprite_->size_);
}

void ClearUI::ApplyDebugParam() {
	clearTextSprite_->position_ = GameParamEditor::GetInstance()->GetValue<Vector2>(kGroupName_, "ClearTextPos");
	clearTextSprite_->size_ = GameParamEditor::GetInstance()->GetValue<Vector2>(kGroupName_, "ClearTextSize");
	guideSprite_->position_ = GameParamEditor::GetInstance()->GetValue<Vector2>(kGroupName_, "GuidePos");
	guideSprite_->size_ = GameParamEditor::GetInstance()->GetValue<Vector2>(kGroupName_, "GuideSize");

	clearTextSprite_->Update();
	guideSprite_->Update();
}