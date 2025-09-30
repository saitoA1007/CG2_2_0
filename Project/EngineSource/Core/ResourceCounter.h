#pragma once
#include<cstdint>

namespace GameEngine {

	// リソースの数を管理
	enum class ResourceCount {
		// ImGuiのSRVの総数
		kMaxImGuiCount,
		kDepthCount,
		// ポストエフェクト用のSRVの総数
		kStartOffScreenCount,
		kMaxOffScreenCount = 16,

		// テクスチャの総数
		kStartTextureCount,
		kMaxTextureCount = kMaxOffScreenCount + 128,

		// 複数モデル描画用のSRVの総数
		kStartModelCount,
		kMaxModelCount = kMaxTextureCount + 128,

		// SRVのリソースの総数
		kMaxResourceCount,
	};
}