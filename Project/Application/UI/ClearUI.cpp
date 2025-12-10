#define NOMINMAX
#include"ClearUI.h"
#include"GameParamEditor.h"
#include"AudioManager.h"
#include"FPSCounter.h"
#include"EasingManager.h"
using namespace GameEngine;

void ClearUI::Initialize(GameEngine::InputCommand* inputCommand, GameEngine::TextureManager* textureManager) {
	inputCommand_ = inputCommand;

	// クリア文字
	clearTextSprite_ = Sprite::Create({1720.0f,240.0f}, {300.0f,120.0f}, {0.5f,0.5f}, {1.0f,0.95f,0.9f,1.0f});
	clearTextGH_ = textureManager->GetHandleByName("clearText.png");

	// クリア文字の背面用の黒帯
	clearTextBackSprite_ = Sprite::Create({1280.0f,240.0f}, {1280.0f,120.0f}, {0.0f,0.5f}, {0.0f,0.0f,0.0f,0.5f});

	// 操作文字
	guideSprite_ = Sprite::Create({ 640.0f,600.0f }, { 300.0f,80.0f }, { 0.5f,0.5f }, { 1.0f,1.0f,1.0f,0.0f });
	guideGH_ = textureManager->GetHandleByName("spaceText.png");

	// 背景画像
	bgSprite_ = Sprite::Create({ 0.0f,0.0f }, { 1280.0f,720.0f }, { 0.0f,0.0f }, { 1.0f,1.0f,1.0f,0.0f });

	// 選択音声を取得
	selectSH_ = AudioManager::GetInstance().GetHandleByName("Select.png");

	// 数字スプライト(0-9)を作成（共通サイズ・アンカー）
	//for (int i = 0; i < 10; ++i) {
	//	clearTimeNumSprites_[i] = Sprite::Create({ 0.0f, 0.0f }, { 48.0f, 64.0f }, { 0.5f, 0.5f }, {1.0f,1.0f,1.0f,1.0f});
	//}

	// 番号画像を取得
	numbersGH_[0] = textureManager->GetHandleByName("0.png");
	numbersGH_[1] = textureManager->GetHandleByName("1.png");
	numbersGH_[2] = textureManager->GetHandleByName("2.png");
	numbersGH_[3] = textureManager->GetHandleByName("3.png");
	numbersGH_[4] = textureManager->GetHandleByName("4.png");
	numbersGH_[5] = textureManager->GetHandleByName("5.png");
	numbersGH_[6] = textureManager->GetHandleByName("6.png");
	numbersGH_[7] = textureManager->GetHandleByName("7.png");
	numbersGH_[8] = textureManager->GetHandleByName("8.png");
	numbersGH_[9] = textureManager->GetHandleByName("9.png");

	// 時間を表示するデータを作成
	for (size_t i = 0; i < numbersSprites_.size(); ++i) {
		
		if (i == 0) {
			numbersSprites_[i].numSprite_ = Sprite::Create({ 640.0f - 8.0f - 64.0f, 400.0f + 32.0f }, { 48.0f, 64.0f }, { 0.5f, 0.5f }, { 0.0f,0.0f,0.0f,0.0f });
		} else if (i == 1) {
			numbersSprites_[i].numSprite_ = Sprite::Create({ 640.0f - 8.0f - 32.0f, 400.0f + 32.0f }, { 48.0f, 64.0f }, { 0.5f, 0.5f }, { 0.0f,0.0f,0.0f,0.0f });
		} else if (i == 2) {
			numbersSprites_[i].numSprite_ = Sprite::Create({ 640.0f + 8.0f + 32.0f, 400.0f + 32.0f }, { 48.0f, 64.0f }, { 0.5f, 0.5f }, { 0.0f,0.0f,0.0f,0.0f });
		} else if (i == 3) {
			numbersSprites_[i].numSprite_ = Sprite::Create({ 640.0f + 8.0f + 64.0f, 400.0f + 32.0f }, { 48.0f, 64.0f }, { 0.5f, 0.5f }, { 0.0f,0.0f,0.0f,0.0f });
		}

		numbersSprites_[i].number = 0;
	}

	// クリア時間UI
	clearTimeText_ = Sprite::Create({ 640.0f, 316.0f + 32.0f }, { 240.0f, 64.0f }, { 0.5f, 0.5f }, { 1.0f,1.0f,1.0f,0.0f });
	clearTimeTextGH_ = textureManager->GetHandleByName("retryText.png");

	// 桁の中間
	numMiddle_ = Sprite::Create({ 640.0f, 400.0f }, { 48.0f, 64.0f + 32.0f }, { 0.5f, 0.5f }, { 0.0f,0.0f,0.0f,0.0f });
	numMiddleGH_ = textureManager->GetHandleByName("dotto.png");

#ifdef _DEBUG
	RegisterBebugParam();
#endif
	ApplyDebugParam();
}

void ClearUI::Update() {
#ifdef _DEBUG
	//ApplyDebugParam();
#endif

	// 有効化された瞬間にアニメーションをリセットして開始
	if (isActive_ && !prevIsActive_) {
		animTimer_ = 0.0f;
		// 初期値セット
		bgSprite_->color_.w = 0.0f;
		clearTextBackSprite_->position_ = { 1280.0f, clearTextBackSprite_->position_.y };
		clearTextSprite_->position_ = { 1720.0f, clearTextSprite_->position_.y };
		guideSprite_->color_.w = 0.0f;

		// 最後の時間
		endTime_ = std::chrono::high_resolution_clock::now();

		// 経過時間を取得する
		duration_ = endTime_ - startTime_;

		int totalSeconds = static_cast<int>(duration_.count());

		int minutes = totalSeconds / 60;
		int seconds = totalSeconds % 60;

		for (uint32_t i = 0; i < numbersSprites_.size(); ++i) {

			if (i == 0) {
				// 分の 10の位
				int min10 = minutes / 10;
				numbersSprites_[i].number = numbersGH_[min10];
			} else if (i == 1) {
				// 分の  1の位
				int min01 = minutes % 10; 
				numbersSprites_[i].number = numbersGH_[min01];
			} else if (i == 2) {
				// 秒の 10の位
				int sec10 = seconds / 10; 
				numbersSprites_[i].number = numbersGH_[sec10];
			} else if (i == 3) {
				int sec01 = seconds % 10;
				numbersSprites_[i].number = numbersGH_[sec01];
			}
		}		
	}

	prevIsActive_ = isActive_;
	if (!isActive_) { return; }

	// アニメーション進行
	Animate();

	if (inputCommand_->IsCommandActive("Start")) {
		// タイトルに戻る
		if (backTitle_) {
			AudioManager::GetInstance().Play(selectSH_, 0.8f, false);
			backTitle_();
		}
	}

	bgSprite_->Update();
	clearTextBackSprite_->Update();
	clearTextSprite_->Update();
	guideSprite_->Update();
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
}

// 秒数に応じて時間のスプライトを表示する
void ClearUI::ShowTimeSprites(int seconds) {
	clearSeconds_ = std::max(0, seconds);

	//// 桁ごとに数字を配置（右寄せで3桁までを想定）。必要に応じ拡張。
	//int s = clearSeconds_;
	//int digits[3] = { 0, 0, 0 };
	//int digitCount = 0;
	//if (s == 0) {
	//	digits[0] = 0; digitCount = 1;
	//} else {
	//	while (s > 0 && digitCount < 3) {
	//		digits[digitCount++] = s % 10;
	//		s /= 10;
	//	}
	//}
	//
	//// 表示位置のベース（任意の位置。デバッグで可変にしてもよい）
	//Vector2 basePos = { 640.0f, 480.0f };
	//float spacing = 52.0f;
	//
	//// すべて非表示相当のサイズにしておく（alpha 0）
	//for (int i = 0; i < 10; ++i) {
	//	clearTimeNumSprites_[i]->color_.w = 0.0f;
	//}
	//
	//// 必要な桁分を右から並べて表示
	//for (int i = 0; i < digitCount; ++i) {
	//	int d = digits[i];
	//	auto& spr = clearTimeNumSprites_[d];
	//	spr->position_ = { basePos.x - (spacing * i), basePos.y };
	//	spr->color_.w = 1.0f;
	//	spr->Update();
	//}
}

// アニメーション（指定タイムライン）
void ClearUI::Animate() {
	if (!isActive_) return;
	animTimer_ += FpsCounter::deltaTime;
	float t = std::min(animTimer_, animTotal_);

	// 0.0f～2.0f: 背景アルファ 0.0 -> 1.0 (Lerp)
	if (t <= 2.0f) {
		float ratio = std::clamp(t / 2.0f, 0.0f, 1.0f);
		bgSprite_->color_.w = Lerp(0.0f, 1.0f, ratio);
	}

	// 2.5f～3.0f: クリア文字背景 X 1280 -> 0 (EaseOutCubic)
	if (t >= 2.5f && t <= 3.0f) {
		float ratio = std::clamp((t - 2.5f) / 0.5f, 0.0f, 1.0f);
		float x = EaseOutCubic(1280.0f, 0.0f, ratio);
		clearTextBackSprite_->position_.x = x;
	}
	// 2.8f～3.3f: クリア文字 X 1280 -> 640 (EaseOutCubic)
	if (t >= 2.8f && t <= 3.3f) {
		float ratio = std::clamp((t - 2.8f) / 0.5f, 0.0f, 1.0f);
		float x = EaseOutCubic(1720.0f, 640.0f, ratio);
		clearTextSprite_->position_.x = x;
	}

	// 3.0f～4.0f: 操作文字アルファ 0.0 -> 1.0 (Lerp)
	if (t >= 3.0f) {
		float ratio = std::clamp((t - 3.0f) / 1.0f, 0.0f, 1.0f);
		guideSprite_->color_.w = Lerp(0.0f, 1.0f, ratio);

		clearTimeText_->color_.w = Lerp(0.0f, 1.0f, ratio);
		numMiddle_->color_.w = Lerp(0.0f, 1.0f, ratio);

		// 秒数
		for (size_t i = 0; i < numbersSprites_.size(); ++i) {
			numbersSprites_[i].numSprite_->color_.w = Lerp(0.0f, 1.0f, ratio);
		}
	}

	// 更新適用
	bgSprite_->Update();
	clearTextBackSprite_->Update();
	clearTextSprite_->Update();
	guideSprite_->Update();
	for (size_t i = 0; i < numbersSprites_.size(); ++i) {
		numbersSprites_[i].numSprite_->Update();
	}
	clearTimeText_->Update();
	numMiddle_->Update();
}

void ClearUI::StartTime() {
	startTime_ = std::chrono::high_resolution_clock::now();
}