#pragma once
#include <Windows.h>
#include <cstdint>
#include <string>

namespace GameEngine {

	class WindowsApp final {
	public:
		WindowsApp() = default;
		~WindowsApp() = default;

		// ウィンドウサイズ
		static const int32_t kWindowWidth = 1280;
		static const int32_t kWindowHeight = 720;

	public:

		/// <summary>
		/// ウィンドウプロシージャ
		/// </summary>
		/// <param name="hwnd">ウィンドウハンドル</param>
		/// <param name="msg"></param>
		/// <param name="wparam"></param>
		/// <param name="lparam"></param>
		/// <returns></returns>
		static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

		/// <summary>
		/// ウィンドウの作成
		/// </summary>
		/// <param name="title">ウィンドウタイトル</param>
		/// <param name="kClientWidth">ウィンドウのクライアント領域の横幅</param>
		/// <param name="kClientHeight">ウィンドウのクライアント領域の縦幅</param>
		void CreateGameWindow(const std::wstring& title, int32_t clientWidth = kWindowWidth, int32_t clientHeight = kWindowHeight);

		/// <summary>
		/// メッセージ処理
		/// </summary>
		bool ProcessMessage();

		/// <summary>
		/// ウィンドウの破棄
		/// </summary>
		void BreakGameWindow();

		/// <summary>
		/// ウィンドウハンドルの取得
		/// </summary>
		/// <returns></returns>
		HWND GetHwnd() const { return hwnd_; }

		HINSTANCE GetHInstance() const { return wc_.hInstance; }

		/// <summary>
		/// フルスクリーンモードと通常モードを切り替える
		/// </summary>
		void ToggleFullScreen();

		/// <summary>
		/// 現在フルスクリーンモードかどうかを取得
		/// </summary>
		/// <returns>フルスクリーンならtrue</returns>
		bool IsFullScreen() const { return isFullScreen_; }

	private:
		//WindowsApp() = default;
		//~WindowsApp() = default;
		WindowsApp(const WindowsApp&) = delete;
		WindowsApp& operator=(const WindowsApp&) = delete;

		// ウィンドウクラス
		WNDCLASS wc_{};
		// ウィンドウハンドル
		HWND hwnd_ = nullptr;
		RECT wrc_{};

		MSG msg_{};

		// フルスクリーン関連
		bool isFullScreen_ = false;
		RECT windowedRect_{};  // ウィンドウモード時の位置とサイズを保存
		LONG windowedStyle_ = 0;  // ウィンドウモード時のスタイルを保存
	};
}
