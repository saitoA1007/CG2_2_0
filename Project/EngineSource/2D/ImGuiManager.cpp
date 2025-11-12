#include"ImGuiManager.h"
using namespace GameEngine;

void ImGuiManager::Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DXGI_SWAP_CHAIN_DESC1 swapChainDesc,
	WindowsApp* windowsApp, SrvManager* srvManager) {

	commandList_ = commandList;
	windowsApp_ = windowsApp;
	srvManager_ = srvManager;

	// ImGuiの初期化。
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.Fonts->Build();

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc_ = {};
	rtvDesc_.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDesc_.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(windowsApp_->GetHwnd());
	ImGui_ImplDX12_Init(device,
		swapChainDesc.BufferCount,
		rtvDesc_.Format,
		srvManager_->GetSRVHeap(),
		srvManager_->GetSRVHeap()->GetCPUDescriptorHandleForHeapStart(),
		srvManager_->GetSRVHeap()->GetGPUDescriptorHandleForHeapStart());

	ed::Config config;
	config.SettingsFile = "node_editor_docked.json";
	g_NodeContext = ed::CreateEditor(&config);
}

void ImGuiManager::BeginFrame() {
	// ImGuiにフレームが始まる旨を伝える
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGuizmo::BeginFrame();

	//ImGui::Begin("NodeEditor");
	//ImGui::Separator();
	//ed::SetCurrentEditor(g_NodeContext);
	//ed::Begin("My Editor", ImVec2(0.0, 0.0f));
	//int uniqueId = 1;
	//// Start drawing nodes.
	//ed::BeginNode(uniqueId++);
	//ImGui::Text("Node A");
	//ed::BeginPin(uniqueId++, ed::PinKind::Input);
	//ImGui::Text("-> In");
	//ed::EndPin();
	//ImGui::SameLine();
	//ed::BeginPin(uniqueId++, ed::PinKind::Output);
	//ImGui::Text("Out ->");
	//ed::EndPin();
	//ed::EndNode();
	//ed::End();
	//ed::SetCurrentEditor(nullptr);
	//ImGui::End();
}

void ImGuiManager::EndFrame() {
	// ImGuiの内部コマンドを生成する
	ImGui::Render();
	//// Imguiの描画用のDescriptorHeapの設定
	ID3D12DescriptorHeap* descriptorHeaps[] = { srvManager_->GetSRVHeap() };
	commandList_->SetDescriptorHeaps(1, descriptorHeaps);
}

void ImGuiManager::Draw() {
	// 実際のcommandListのImGuiの描画コマンドを積む
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList_);
}

void ImGuiManager::Finalize() {
	// ImGuiの終了処理
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}