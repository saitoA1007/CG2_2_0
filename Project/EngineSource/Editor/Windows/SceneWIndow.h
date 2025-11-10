#pragma once
#include"IEditorWindow.h"

class SceneWindow : public IEditorWindow {
public:
	void Draw() override;
	std::string GetName() const override { return "SceneView"; };
};