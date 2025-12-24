#pragma once

// ボスの基本的な状態
enum class BossState {
    In,     // 入りの状態
    Battle, // 戦いの状態
    Out,    // 終わりの状態

    MaxCount // 数
};