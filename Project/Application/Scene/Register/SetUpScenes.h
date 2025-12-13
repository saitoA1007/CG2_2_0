#pragma once
#include"SceneRegistry.h"

#include "Application/Scene/TitleScene.h"

using namespace GameEngine;

/// <summary>
/// シーンを登録する
/// </summary>
/// <param name="factory"></param>
void SetupScenes(SceneRegistry& factory) {

    // 各シーンの登録
    factory.RegisterScene<TitleScene>("Title");

    // 立ち上げ時に起動するシーン
    factory.SetDefaultScene("Title");
}