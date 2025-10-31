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

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc_ = {};
	rtvDesc_.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDesc_.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(windowsApp_->GetHwnd());
	ImGui_ImplDX12_Init(dxCommon_->GetDevice(),
		dxCommon_->GetSwapChainDesc().BufferCount,
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
	//ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
	ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

	if (ImGui::BeginMenuBar()) {
		if (ImGui::BeginMenu("View")) {
			ImGui::Text("NoneParameter");
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
	
	ImGui::End();

	ImGui::Begin("NodeEditor");
	ImGui::Separator();
	ed::SetCurrentEditor(g_NodeContext);
	ed::Begin("My Editor", ImVec2(0.0, 0.0f));
	int uniqueId = 1;
	// Start drawing nodes.
	ed::BeginNode(uniqueId++);
	ImGui::Text("Node A");
	ed::BeginPin(uniqueId++, ed::PinKind::Input);
	ImGui::Text("-> In");
	ed::EndPin();
	ImGui::SameLine();
	ed::BeginPin(uniqueId++, ed::PinKind::Output);
	ImGui::Text("Out ->");
	ed::EndPin();
	ed::EndNode();
	ed::End();
	ed::SetCurrentEditor(nullptr);
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