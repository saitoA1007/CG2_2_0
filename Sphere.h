#pragma once
#include "Object3d.h"

class Sphere : public RenderableObject {
public:
    Sphere();
    ~Sphere() override;

    void Initialize(ID3D12Device* device, uint32_t subdivision) override;

    void SetTransformationMatrix(const Matrix4x4& worldMatrix);

private:
    TransformationMatrix* transformationMatrixData_ = nullptr;
};