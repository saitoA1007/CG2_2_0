#pragma once
#include <d3d12.h>
#include <string>
#include <vector>
#include "Externals/DirectXTex/d3dx12.h"

namespace GameEngine {

    struct RenderPassContext {
        ID3D12GraphicsCommandList* commandList = nullptr;
        D3D12_VIEWPORT viewport{};
        D3D12_RECT scissorRect{};
    };

    class IRenderPass {
    public:
        virtual ~IRenderPass() = default;

        /// <summary>
        /// パスの名前を取得
        /// </summary>
        /// <returns></returns>
        virtual const std::string GetName() const = 0;

        /// <summary>
        /// 初期化
        /// </summary>
        virtual void Initialize() = 0;

        /// <summary>
        /// 実行する
        /// </summary>
        virtual void Run() = 0;

        /// <summary>
        /// リサイズ
        /// </summary>
        /// <param name="width"></param>
        /// <param name="height"></param>
        virtual void Resize(uint32_t width, uint32_t height) = 0;

        // 有効フラグ
        void SetEnable(const bool& isEnable) { isEnable_ = isEnable; }
        bool IsEnable() const { return isEnable_; }

        virtual std::string GetOutputRTVName() const = 0;

        virtual void DrawImGui() = 0;

    protected:
        bool isEnable_ = true;
        uint32_t width_ = 0;
        uint32_t height_ = 0;
    };
}

