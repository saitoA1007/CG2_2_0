#pragma once
#include"IEditorWindow.h"

class ConsoleWindow : public IEditorWindow {
public:
	void Draw() override;
	std::string GetName() const override { return "Console"; };
};