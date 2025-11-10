#pragma once
#include"EditorWindowManager.h"
#include"EditorMenuBar.h"

#include"Windows/SceneWIndow.h"
#include"Windows/AssetWindow.h"
#include"Windows/ConsoleWindow.h"
#include"Windows/HierarchyWindow.h"
#include"Windows/InspectorWindow.h"
#include"Windows/PerformanceWindow.h"

class EditorCore {
public:

	void Initialize();

	void Run();

private:

	std::unique_ptr<EditorWindowManager> windowManager_;

	std::unique_ptr<EditorMenuBar> menuBar_;

private:

	/// <summary>
	/// Dockをするためのスペースを作成する
	/// </summary>
	void BeginDockSpace();

};