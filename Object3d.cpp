#include"Object3d.h"

RenderableObject::~RenderableObject() {
    Release();
}

void RenderableObject::Draw(ID3D12GraphicsCommandList* commandList, ID3D12Resource* directionalLightResource,D3D12_GPU_DESCRIPTOR_HANDLE* textureSrvHandlesGPU) {
    commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);
    commandList->IASetIndexBuffer(&indexBufferView_);
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
    commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixResource_->GetGPUVirtualAddress());
    commandList->SetGraphicsRootDescriptorTable(2, *textureSrvHandlesGPU);
    commandList->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());
    commandList->DrawIndexedInstanced(totalIndices_, 1, 0, 0, 0);
}

void RenderableObject::Release() {
    if (vertexResource_) vertexResource_.Reset();
    if (indexResource_) indexResource_.Reset();
    if (materialResource_) materialResource_.Reset();
    if (transformationMatrixResource_) transformationMatrixResource_.Reset();
}