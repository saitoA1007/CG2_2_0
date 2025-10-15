#include"ImGuiManager.h"
using namespace GameEngine;

void ImGuiManager::Initialize(WindowsApp* windowsApp, DirectXCommon* dxCommon, SrvManager* srvManager) {

	windowsApp_ = windowsApp;
	dxCommon_ = dxCommon;
	srvManager_ = srvManager;

	// ImGuiの初期化。
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.Fonts->Build();

	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(windowsApp_->GetHwnd());
	ImGui_ImplDX12_Init(dxCommon_->GetDevice(),
		dxCommon_->GetSwapChainDesc().BufferCount,
		dxCommon_->GetRTVDesc().Format,
		srvManager_->GetSRVHeap(),
		srvManager_->GetSRVHeap()->GetCPUDescriptorHandleForHeapStart(),
		srvManager_->GetSRVHeap()->GetGPUDescriptorHandleForHeapStart());
}

void ImGuiManager::BeginFrame() {
	// ImGuiにフレームが始まる旨を伝える
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGuizmo::BeginFrame();

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	
	ImGui::Begin("DockSpace Window", nullptr, window_flags);
	ImGui::PopStyleVar(2);
	
	ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
	ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
	
	ImGui::End();
}

void ImGuiManager::EndFrame() {

	// 描画した結果を移す
	ImGui::Begin("Scene");
	ImVec2 sceneWindowSize = ImGui::GetContentRegionAvail();
	D3D12_GPU_DESCRIPTOR_HANDLE& srvHandle = dxCommon_->GetSRVHandle();
	ImGui::Image((ImTextureID)srvHandle.ptr, sceneWindowSize);
	ImGui::End();

	// ImGuiの内部コマンドを生成する
	ImGui::Render();
	//// Imguiの描画用のDescriptorHeapの設定
	ID3D12DescriptorHeap* descriptorHeaps[] = { srvManager_->GetSRVHeap() };
	dxCommon_->GetCommandList()->SetDescriptorHeaps(1, descriptorHeaps);
}

void ImGuiManager::Draw() {
	// 実際のcommandListのImGuiの描画コマンドを積む
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dxCommon_->GetCommandList());
}

void ImGuiManager::Finalize() {
	// ImGuiの終了処理
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}