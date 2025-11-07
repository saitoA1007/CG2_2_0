#pragma once
#include"Externals/imgui/imgui.h"
#include"Externals/imgui/imgui_impl_dx12.h"
#include"Externals/imgui/imgui_impl_win32.h"

#include"Externals/imgui/ImGuizmo.h"

#include"WindowsApp.h"
#include"DirectXCommon.h"
#include"SrvManager.h"

#include "Externals/imgui/imgui_node_editor.h"
namespace ed = ax::NodeEditor;

static ed::EditorContext* g_NodeContext = nullptr;

namespace GameEngine {

	class ImGuiManager final {
	public:
		ImGuiManager() = default;
		~ImGuiManager() = default;

		/// <summary>
		/// 初期処理
		/// </summary>
		/// <param name="windowsApp"></param>
		/// <param name="dxCommon"></param>
		void Initialize(WindowsApp* windowsApp, DirectXCommon* dxCommon,SrvManager* srvManager);

		/// <summary>
		/// 更新前処理
		/// </summary>
		void BeginFrame();

		/// <summary>
		/// 更新後処理
		/// </summary>
		void EndFrame();

		/// <summary>
		/// 描画処理
		/// </summary>
		void Draw();

		/// <summary>
		/// 終了処理
		/// </summary>
		void Finalize();

	private:
		ImGuiManager(const ImGuiManager&) = delete;
		const ImGuiManager& operator=(const ImGuiManager&) = delete;

		WindowsApp* windowsApp_ = nullptr;
		DirectXCommon* dxCommon_ = nullptr;
		SrvManager* srvManager_ = nullptr;

		// 固定したいアスペクト比 (16:9)
		const float kTargetAspect = 1280.0f / 720.0f;
	};
}
