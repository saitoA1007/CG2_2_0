#pragma once
#include <Windows.h>
#include<cstdint>
#include"WindowsAPI.h"
#include<cassert>
#include<format>
#include<iostream>
#include"Log.h"
#include"Vector4.h"
#include"ConvertString.h"

namespace Engine {

	/// <summary>
	/// エンジンの初期化
	/// </summary>
	void Initialize(const std::wstring& title = L"LE2A_05_サイトウ_アオイ", const int32_t kClientWidth, const int32_t kClientHeight);

	/// <summary>
	/// エンジンの更新
	/// </summary>
	void Update();

	/// <summary>
	/// エンジンの終了
	/// </summary>
	void Finalize();
}
