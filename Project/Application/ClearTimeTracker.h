#pragma once
#include <chrono>
#include"ClearTime.h"

class ClearTimeTracker {
public:

	ClearTimeTracker();

	// 時間の計測を開始
	void StartMeasureTimes();

	// 時間の計測を終了
	void EndMeasureTimes();

private:

	// 最初の時間
	std::chrono::time_point<std::chrono::high_resolution_clock> startTime_;

	// 最後の時間
	std::chrono::time_point<std::chrono::high_resolution_clock> endTime_;

	// 経過時間を保存
	std::chrono::duration<float> duration_;
};