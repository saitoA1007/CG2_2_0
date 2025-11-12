#pragma once
#include"SceneState.h"
#include"SceneChangeRequest.h"

namespace GameEngine {

	class SceneMenuBar {
	public:

		SceneMenuBar(SceneChangeRequest* request);

		void Run();

	private:
		SceneState sceneState_ = SceneState::GE;
		SceneChangeRequest* sceneChangeRequest_ = nullptr;
		static inline const char* sceneNames_[] = { "Title", "Game", "GE" };
	};
}

