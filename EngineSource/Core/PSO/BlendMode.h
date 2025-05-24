#pragma once

namespace GameEngine {

    enum BlendMode {
        // ブレンドなし
        kBlendModeNone,
        // 通常aブレンド Src * SrcA + Dest + (1-SrcA)
        kBlendModeNormal,
        // 加算 Src * SrcA * Dest * 1
        kBlendModeAdd,
        // 減算 Dest * 1 - Src * SrcA
        kBlendModeSubtract,
        // 乗算 Src * 0 + Dest * Src
        kBlendModeMultily,
        // スクリーン Src * (1-Dest) + Dest * 1
        kBlendModeScreen,

        // 利用禁止
        kCountOfBlendMode,
    };

    enum DrawModel {
        Fill, // 中身を描画
        Frame, // フレームを描画
        FrameBack, // 後ろ側を描画

        kCountOfDrawMode,
    };

}