#pragma once
#include<chrono>
#include<thread>

class FrameRateController {
public:

	// FPS固定初期化
	void InitializeFixFPS();

	// FPS固定更新
	void UpdateFixFPS();

private:
	// 記録時間(FPS固定用)
	std::chrono::steady_clock::time_point reference_;
};