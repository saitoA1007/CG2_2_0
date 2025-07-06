#pragma once
#include"BasePostEffect.h"
#include"EngineSource/Core/PSO/GaussianBlurPSO.h"

namespace GameEngine {

    class GaussianBlur : public BasePostEffect {
    public:

        void Initialize(ID3D12Device* device) override {
            // ブラー用のPSO、ルートシグネチャ、定数バッファなどを作成
        }

        ID3D12Resource* Apply(ID3D12GraphicsCommandList* commandList, ID3D12Resource* source) override {
            // 1. PSOとルートシグネチャを設定
            // 2. 定数バッファを更新（ブラーの強度など）
            // 3. 入力テクスチャ(source)をSRVとしてバインド
            // 4. 出力用のレンダーターゲット(m_renderTarget)をRTVとして設定
            // 5. 全画面ポリゴンを描画
            // 6. 出力したリソース(m_renderTarget)を返す
            return m_renderTarget.Get();
        }

        void OnGUI() override {
            // ImGuiなどを使ってブラーの強度などを調整するUIを表示
        }

        void Release() override {
            // 作成したリソースを解放
        }

    private:
        // このエフェクト専用のレンダーターゲット、PSO、ルートシグネチャなど
        Microsoft::WRL::ComPtr<ID3D12Resource> m_renderTarget;

        // ブラーのPSO
        std::unique_ptr<GaussianBlurPSO> gaussianBlurPSO;
    };
}