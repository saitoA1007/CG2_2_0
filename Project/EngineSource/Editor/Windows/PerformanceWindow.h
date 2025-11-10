#pragma once
#include"IEditorWindow.h"
#include"ImGuiManager.h"

class PerformanceWindow : public IEditorWindow {
public:
	void Draw() override;
	std::string GetName() const override { return "Performance"; };
};