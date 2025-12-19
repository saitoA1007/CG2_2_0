#include"WindowsApp.h"
#include"ImGuiManager.h"

#pragma comment(lib,"winmm.lib")

using namespace GameEngine;

#ifdef USE_IMGUI
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif

LRESULT CALLBACK WindowsApp::WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {

#ifdef USE_IMGUI
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) {
		return true;
	}
#endif

	// メッセージに応じてゲーム固有の処理を行う
	switch (msg) {
		// ウィンドウが破棄された
	case WM_DESTROY:
		// OSに対して、アプリの終了を伝える
		PostQuitMessage(0);
		return 0;

		// キーが押された
	case WM_KEYDOWN:
		// ESCキーが押された場合
		if (wparam == VK_ESCAPE) {
			// ウィンドウを閉じる
			PostQuitMessage(0);
			return 0;
		}
		break;
	}

	// 標準のメッセージ処理を行う
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

void WindowsApp::CreateGameWindow(const std::wstring& title, int32_t kClientWidth, int32_t kClientHeight) {

	// COMの初期化
	CoInitializeEx(0, COINIT_MULTITHREADED);

	/// ウィンドウクラスの作成
	// ウィンドウプロシージャ
	wc_.lpfnWndProc = &WindowProc;
	// ウィンドウクラス名
	wc_.lpszClassName = L"CG2WindowClass";
	// インスタンスハンドル
	wc_.hInstance = GetModuleHandle(nullptr);
	// カーソル
	wc_.hCursor = LoadCursor(nullptr, IDC_ARROW);

	// ウィンドウクラスを登録する
	RegisterClass(&wc_);

	// ウィンドウサイズを表す構造体にクライアント領域を入れる
	wrc_ = { 0,0,kClientWidth,kClientHeight };

	// クライアント領域を元に実際のサイズをwrcを変更してもらう
	AdjustWindowRect(&wrc_, WS_OVERLAPPEDWINDOW, false);

	hwnd_ = CreateWindow(
		wc_.lpszClassName,      // 利用するクラス名
		title.c_str(),          // タイトルバーの文字
		WS_OVERLAPPEDWINDOW/* & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME*/,
		CW_USEDEFAULT,          // 表示X座標(Windowに任せる)
		CW_USEDEFAULT,          // 表示Y座標(WindowOSに任せる)
		wrc_.right - wrc_.left, // ウィンドウ横幅
		wrc_.bottom - wrc_.top, // ウィンドウ縦幅
		nullptr,                // 親ウィンドウハンドル
		nullptr,                // メニューハンドル
		wc_.hInstance,          // インスタンスハンドル
		nullptr);               // オプション

	// ウィンドウモード時のスタイルを保存
	windowedStyle_ = GetWindowLong(hwnd_, GWL_STYLE);

	// ウィンドウを表示する
	ShowWindow(hwnd_, SW_SHOW);

	// システムタイマーの分解能を上げる
	timeBeginPeriod(1);
}

bool WindowsApp::ProcessMessage() {

	// falseならそのまま処理、trueなら終了
	while (PeekMessage(&msg_, nullptr, 0, 0, PM_REMOVE)) {
		if (msg_.message == WM_QUIT) {
			return true;
		}
		TranslateMessage(&msg_);
		DispatchMessage(&msg_);
	}
	return false;
}

void WindowsApp::BreakGameWindow() {
	// COMの終了処理
	CoUninitialize();

	CloseWindow(hwnd_);
}

void WindowsApp::ToggleFullScreen() {
	if (!isFullScreen_) {
		// フルスクリーンモードに切り替え

		// 現在のウィンドウ位置とサイズを保存
		GetWindowRect(hwnd_, &windowedRect_);

		// ウィンドウスタイルを変更
		SetWindowLong(hwnd_, GWL_STYLE, WS_POPUP | WS_VISIBLE);

		// プライマリモニターの解像度を取得
		int screenWidth = GetSystemMetrics(SM_CXSCREEN);
		int screenHeight = GetSystemMetrics(SM_CYSCREEN);

		// ウィンドウを画面全体に配置
		SetWindowPos(
			hwnd_,
			HWND_TOP,
			0, 0,
			screenWidth, screenHeight,
			SWP_FRAMECHANGED | SWP_SHOWWINDOW
		);

		isFullScreen_ = true;
	} else {
		// 通常モードに戻す

		// ウィンドウスタイルを元に戻す
		SetWindowLong(hwnd_, GWL_STYLE, windowedStyle_);

		// 保存しておいた位置とサイズに戻す
		SetWindowPos(
			hwnd_,
			HWND_TOP,
			windowedRect_.left,
			windowedRect_.top,
			windowedRect_.right - windowedRect_.left,
			windowedRect_.bottom - windowedRect_.top,
			SWP_FRAMECHANGED | SWP_SHOWWINDOW
		);

		isFullScreen_ = false;
	}
}