#pragma once
#include"TextureManager.h"

#include"EditorWindowManager.h"
#include"EditorMenuBar.h"
#include"EditorLayout.h"
#include"EditorToolBar.h"

#include"Windows/SceneWIndow.h"
#include"Windows/AssetWindow.h"
#include"Windows/ConsoleWindow.h"
#include"Windows/HierarchyWindow.h"
#include"Windows/InspectorWindow.h"
#include"Windows/PerformanceWindow.h"

namespace GameEngine {

	class EditorCore {
	public:

		void Initialize(TextureManager* textureManager);

		void Run();

		// 更新状態を取得する
		bool IsActiveUpdate() const;

		bool IsPause() const;

		void Finalize();

	private:

		std::unique_ptr<EditorWindowManager> windowManager_;

		std::unique_ptr<EditorMenuBar> menuBar_;

		std::unique_ptr<EditorLayout> editorLayout_;

		std::unique_ptr<EditorToolBar> editorToolBar_;

	private:

		/// <summary>
		/// Dockをするためのスペースを作成する
		/// </summary>
		void BeginDockSpace();

	};
}
