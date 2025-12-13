#pragma once
#include <memory>
#include <vector>
#include <string>
#include"IRenderPass.h"

namespace GameEngine {

	class RenderPassController final {
	public:
		RenderPassController() = default;
		~RenderPassController() = default;

		void Initialize();

		void AddPass(std::unique_ptr<IRenderPass> pass);

		//IRenderPass* GetPass(const std::string& name);

		//void AllDraw();

		// 最終敵な描画先
		//CD3DX12_GPU_DESCRIPTOR_HANDLE GetFinalOutputSRV() const;

		//void DrawImGui();

	private:
		RenderPassController(const RenderPassController&) = delete;
		RenderPassController& operator=(const RenderPassController&) = delete;

		uint32_t width_ = 0;
		uint32_t height_ = 0;
	};
}

