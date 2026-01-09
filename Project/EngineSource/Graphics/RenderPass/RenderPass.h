#pragma once
#include <d3d12.h>
#include <string>
#include <vector>
#include "Externals/DirectXTex/d3dx12.h"
#include"RenderTexture.h"

namespace GameEngine {

    struct RenderPassContext {
        ID3D12GraphicsCommandList* commandList = nullptr;
        D3D12_VIEWPORT viewport{};
        D3D12_RECT scissorRect{};
    };

    class RenderPass {
    public:

        RenderPass(const std::string& name, RenderPass* renderPass);

        // 描画前処理
        void PrePass();

        // 描画後処理
        void PostPass();

        // srvIndexを取得
        uint32_t GetSrvIndex()const;

        // リサイズ
        //void Resize(uint32_t width, uint32_t height);

        // 名前を取得
        const std::string GetName() const { return name_; }

    private:
        // レンダーパス
        RenderPass* renderPass_ = nullptr;

        // パスの名前
        std::string name_;

        bool isEnable_ = true;
        uint32_t width_ = 0;
        uint32_t height_ = 0;
    };
}