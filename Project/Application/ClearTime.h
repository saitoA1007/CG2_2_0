#pragma once

// クリア時間
class ClearTime {
public:

	// 桁
	enum class Digit {
		Min10, // 分の10の位
		Min01, // 分の1の位
		Sec10, // 秒の10の位
		Sec01, // 秒の1の位

		MaxCount
	};

	static inline int totalSeconds = 0;

	// 桁
	static inline int numbers[static_cast<size_t>(Digit::MaxCount)] = {};

	// 各桁の番号を取得
	static void CreateNumbers() {
		int minutes = totalSeconds / 60;
		int seconds = totalSeconds % 60;

		for (size_t i = 0; i < static_cast<size_t>(Digit::MaxCount); ++i) {

			if (i == static_cast<size_t>(Digit::Min10)) {
				// 分の10の位
				numbers[i] = minutes / 10;
			} else if (i == static_cast<size_t>(Digit::Min01)) {
				// 分の1の位
				numbers[i] = minutes % 10;
			} else if (i == static_cast<size_t>(Digit::Sec10)) {
				// 秒の10の位
				numbers[i] = seconds / 10;
			} else if (i == static_cast<size_t>(Digit::Sec01)) {
				// 秒の1の位
				numbers[i] = seconds % 10;
			}
		}

	}
};
