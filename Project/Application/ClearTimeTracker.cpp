#include"ClearTimeTracker.h"

ClearTimeTracker::ClearTimeTracker() {

}

void ClearTimeTracker::StartMeasureTimes() {
	startTime_ = std::chrono::high_resolution_clock::now();
}

void ClearTimeTracker::EndMeasureTimes() {
	// 最後の時間
	endTime_ = std::chrono::high_resolution_clock::now();

	// クリア時間を取得する
	duration_ = endTime_ - startTime_;
	ClearTime::totalSeconds = static_cast<int>(duration_.count());
	// 時間から各桁の番号を取得
	ClearTime::CreateNumbers();
}