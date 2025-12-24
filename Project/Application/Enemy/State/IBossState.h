#pragma once

class IBossState {
public:
    virtual ~IBossState() = default;
    virtual void Enter() = 0;
    virtual void Update() = 0;
    virtual void Exit() = 0;
};