#pragma once
#include"IEditorWindow.h"

class AssetWindow : public IEditorWindow {
public:
	void Draw() override;
	std::string GetName() const override { return "Asset"; };
};

