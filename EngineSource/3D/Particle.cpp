#include"Particle.h"

using namespace GameEngine;

ID3D12Device* Particles::device_ = nullptr;
ID3D12GraphicsCommandList* Particles::commandList_ = nullptr;
ParticleCSPSO* Particles::particleCSPSO_ = nullptr;

void Particles::Initialize() {
	// メッシュを作成
	mesh_ = std::make_unique<Mesh>();
	mesh_->CreatePlaneMesh(device_, { 1.0f,1.0f });

	// マテリアルを作成
	material_ = std::make_unique<Material>();
	material_->Initialize({ 1.0f,1.0f,1.0f,1.0f }, { 1.0f,1.0f,1.0f }, 500.0f, false);





}

void Particles::Update() {

}

void Particles::Draw() {

	commandList_->SetComputeRootSignature(particleCSPSO_->GetComputeRootSignature());
	commandList_->SetPipelineState(particleCSPSO_->GetComputePipelineState());

	// UAV,CBVなどをセット
	//commandList_->Dispatch();


	commandList_->SetGraphicsRootSignature(particleCSPSO_->GetRootSignature());
	commandList_->SetPipelineState(particleCSPSO_->GetPipelineState());

	commandList_->DrawInstanced(6, 1024, 0, 0);
}