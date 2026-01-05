#include"ClearTimeUI.h"

using namespace GameEngine;

ClearTimeUI::ClearTimeUI(GameEngine::TextureManager* textureManager) {

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
			numbersSprites_[i].numSprite_ = Sprite::Create({ basePos_.x - 8.0f - 64.0f - 12.0f, basePos_.y }, { 48.0f, 64.0f }, { 0.5f, 0.5f }, { 1.0f,1.0f,1.0f,1.0f });
		} else if (i == 1) {
			numbersSprites_[i].numSprite_ = Sprite::Create({ basePos_.x - 8.0f - 32.0f, basePos_.y }, { 48.0f, 64.0f }, { 0.5f, 0.5f }, { 1.0f,1.0f,1.0f,1.0f });
		} else if (i == 2) {
			numbersSprites_[i].numSprite_ = Sprite::Create({ basePos_.x + 8.0f + 32.0f, basePos_.y }, { 48.0f, 64.0f }, { 0.5f, 0.5f }, { 1.0f,1.0f,1.0f,1.0f });
		} else if (i == 3) {
			numbersSprites_[i].numSprite_ = Sprite::Create({ basePos_.x + 8.0f + 64.0f + 12.0f, basePos_.y }, { 48.0f, 64.0f }, { 0.5f, 0.5f }, { 1.0f,1.0f,1.0f,1.0f });
		}

		// クリア時間の各桁の番号を取得
		numbersSprites_[i].number = numbersGH_[ClearTime::numbers[i]];
	}

	// 桁の中間
	numMiddle_ = Sprite::Create({ basePos_.x, basePos_.y }, { 48.0f, 64.0f }, { 0.5f, 0.5f }, { 1.0f,1.0f,1.0f,1.0f });
	numMiddleGH_ = textureManager->GetHandleByName("dotto.png");
}