#pragma once
#include"IEditorWindow.h"
#include"RendererManager.h"

namespace GameEngine {

	class SceneWindow : public IEditorWindow {
	public:

		SceneWindow(RendererManager* rendererManager);

		void Draw() override;
		std::string GetName() const override { return "SceneView"; };

	private:
		GameEngine::RendererManager* rendererManager_ = nullptr;
		// 固定したいアスペクト比 (16:9)
		const float kTargetAspect = 1280.0f / 720.0f;
	};
}
