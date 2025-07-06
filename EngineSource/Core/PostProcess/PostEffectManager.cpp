#include"PostEffectManager.h"

void PostEffectManager::Initialize(ID3D12Device* device) {

}

void PostEffectManager::Release() {

}

// エフェクトチェーンにエフェクトを追加
void PostEffectManager::AddEffect(std::unique_ptr<BasePostEffect> effect) {
    postEffects_.push_back(std::move(effect));
}

// エフェクトチェーンを実行
void PostEffectManager::Render(ID3D12GraphicsCommandList* commandList, ID3D12Resource* sceneRenderTarget) {
    if (postEffects_.empty()) {
        // エフェクトがなければ何もしない
        return;
    }

   //ID3D12Resource* currentSource = sceneRenderTarget;
   //int renderTargetIndex = 0;
   //
   //for (size_t i = 0; i < postEffects_.size(); ++i) {
   //    // 出力先レンダーターゲットを取得
   //    ID3D12Resource* currentDest = m_renderTargets[renderTargetIndex].Get();
   //
   //    // **【重要】リソースバリア**
   //    // 入力(SRV)と出力(RTV)のリソース状態を正しく遷移させる
   //    // Transition currentSource from COMMON to PIXEL_SHADER_RESOURCE
   //    // Transition currentDest from COMMON to RENDER_TARGET
   //
   //    // エフェクトを適用
   //    postEffects_[i]->Apply(commandList, currentSource);
   //
   //    // **【重要】リソースバリア**
   //    // 次のループで入力として使えるように状態を戻す
   //    // Transition currentSource from PIXEL_SHADER_RESOURCE to COMMON
   //    // Transition currentDest from RENDER_TARGET to COMMON
   //
   //    // 入力と出力を入れ替え
   //    currentSource = currentDest;
   //    renderTargetIndex = 1 - renderTargetIndex; // 0と1を切り替え
   //}

    // 最終的な結果 (currentSource) をバックバッファにコピーする
    // ...
}

// GUIの表示
void PostEffectManager::OnGUI() {

}