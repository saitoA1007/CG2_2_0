#pragma once
#include<vector>
#include"BasePostEffect.h"
#include <wrl.h>

class PostEffectManager {
public:

    void Initialize(ID3D12Device* device);
    void Release();

    // エフェクトチェーンにエフェクトを追加
    void AddEffect(std::unique_ptr<BasePostEffect> effect);

    // エフェクトチェーンを実行
    void Render(
        ID3D12GraphicsCommandList* commandList,
        ID3D12Resource* sceneRenderTarget // 3Dシーンの最終レンダリング結果
    );

    // GUIの表示
    void OnGUI();

private:

	std::vector<std::unique_ptr<BasePostEffect>> postEffects_;

	// ピンポンレンダリング用のレンダーターゲット(使うか分からない)
	Microsoft::WRL::ComPtr<ID3D12Resource> m_renderTargets[2];
};