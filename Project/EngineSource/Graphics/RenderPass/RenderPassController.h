#pragma once
#include <memory>
#include <vector>
#include<unordered_map>
#include <string>
#include"IRenderPass.h"
#include"RenderPass.h"

#include"RenderTextureManager.h"

namespace GameEngine {

	class RenderPassController final {
	public:
		RenderPassController() = default;
		~RenderPassController() = default;

		void Initialize(RenderTextureManager* renderTextureManager);

		void AddPass(const std::string& name);

		// 描画前に呼び出す(参照する時に切り替えられていなければassertで引っ掛ける)
		void PrePass(const std::string& name);
		void PostPass(const std::string& name);

		//IRenderPass* GetPass(const std::string& name);

		// 最終敵な描画先
		//CD3DX12_GPU_DESCRIPTOR_HANDLE GetFinalOutputSRV() const;

		// 画像を受け取るようにする
		//uint32_t GetPassSrvIndex();

		// 最終的な画面に移す描画処理
		//void EndFrame();

	private:
		RenderPassController(const RenderPassController&) = delete;
		RenderPassController& operator=(const RenderPassController&) = delete;

		RenderTextureManager* renderTextureManager_ = nullptr;
		
		std::unordered_map<std::string, std::unique_ptr<RenderPass>> renderPassList_;

		uint32_t width_ = 0;
		uint32_t height_ = 0;
	};
}

