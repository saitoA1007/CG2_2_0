#pragma once
#include <d3d12.h>
#include <string>
#include <vector>
#include "Externals/DirectXTex/d3dx12.h"
#include"RenderTexture.h"

namespace GameEngine {

    struct RenderPassContext {
        ID3D12GraphicsCommandList* commandList = nullptr;
        bool isDepth = false;
    };

    class RenderPass {
    public:

        RenderPass(const std::string& name, RenderPassContext* context, RenderTexture* renderTexture);

        // 描画前処理
        void PrePass();

        // 描画後処理
        void PostPass();

        // srvIndexを取得
        uint32_t GetSrvIndex()const;

        CD3DX12_GPU_DESCRIPTOR_HANDLE GetSrvHandle();

        // リサイズ
        //void Resize(uint32_t width, uint32_t height);

        // 名前を取得
        const std::string GetName() const { return name_; }

    private:

        RenderTexture* renderTexture_ = nullptr;

        ID3D12GraphicsCommandList* commandList_ = nullptr;
        D3D12_VIEWPORT viewport_{};
        D3D12_RECT scissorRect_{};

        // パスの名前
        std::string name_;

        bool isEnable_ = true;

        bool isDepth_ = false;
    };
}