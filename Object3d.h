#pragma once
#include <d3d12.h>
#include "VertexData.h"
#include "Material.h"
#include "Math/Matrix4x4.h"
#include <wrl.h>

// オブジェクトの基底クラス
class RenderableObject {
public:
    
    virtual ~RenderableObject();

    virtual void Initialize(ID3D12Device* device, uint32_t subdivision)=0;
    virtual void Draw(ID3D12GraphicsCommandList* commandList, ID3D12Resource* directionalLightResource,D3D12_GPU_DESCRIPTOR_HANDLE* textureSrvHandlesGPU);
    virtual void Release();

protected:
//private:

    RenderableObject() = default;
    RenderableObject(RenderableObject&) = delete;
    RenderableObject& operator=(RenderableObject&) = delete;

    Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
    Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;
    Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
    Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource_;

    D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
    D3D12_INDEX_BUFFER_VIEW indexBufferView_{};

    uint32_t totalVertices_ = 0;
    uint32_t totalIndices_ = 0;
};