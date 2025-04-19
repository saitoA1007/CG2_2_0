
#include<cstdint>
#include<d3d12.h>
#include<dxgi1_6.h>
#include<cassert>
#include<format>
#include<iostream>
#include"Log.h"
#include"Vector4.h"
#include"ConvertString.h"
#include"CompileShader.h"
#include"CreateBufferResource.h"
#include"DescriptorHeap.h"
#include"Math.h"
#include"VertexData.h"
#include"DepthStencilTexture.h"
#include"DescriptorHandle.h"
#include"Material.h"
#include"DirectionalLight.h"
#include"CrashHandler.h"
#include"Camera.h"
#include<dxgidebug.h>
#include<dxcapi.h>
#include <string>
#include <vector>

#include"ResorceLeakChecker.h"

#include"WindowsAPI.h"
#include"DirectXCommon.h"
#include"Sphere.h"
#include"ImGuiManager.h"
#include"TextureManager.h"

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"dxcompiler.lib")

// windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	
	// 誰も補足しなかった場合に(Unhandled)、補足する関数を登録
	SetUnhandledExceptionFilter(ExportDump);

	// COMの初期化
	CoInitializeEx(0, COINIT_MULTITHREADED);

	/// ログを出力するための準備
	// ログディレクトリがなければ作成
	if (!std::filesystem::exists("logs")) {
		std::filesystem::create_directory("logs");
	}
	// 現在時刻を取得(UTC時刻)
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	// ログファイルの名前にコンマ何秒はいらないので、削って秒にする
	std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds>
		nowSeconds = std::chrono::time_point_cast<std::chrono::seconds>(now);
	// 日本時間(PCの設定時間)に変換
	std::chrono::zoned_time localTime{ std::chrono::current_zone(),nowSeconds };
	// formatを使って年月日_時分秒の文字列に変換
	std::string dateString = std::format("{:%Y%m%d_%H%M%S}", localTime);
	// 時刻を使ってファイル名を決定
	std::string logFilePath = std::string("logs/") + dateString + ".log";
	// ファイルを作って書き込み準備
	std::ofstream logStream(logFilePath);


	// ウィンドウの作成
	WindowsApp* windowsApp = WindowsApp::GetInstance();
	windowsApp->CreateGameWindow(L"CG2",1280,720);

	// リソースチェックのデバック
	D3DResourceLeakChecker leakCheck;

	// DirectXCommonの初期化
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();
	dxCommon->Initialize(windowsApp->GetHwnd(), windowsApp->kWindowWidth, windowsApp->kWindowHeight, logStream);

	// ImGuiの初期化
	ImGuiManager* imGuiManager = ImGuiManager::GetInstance();
	imGuiManager->Initialize(windowsApp, dxCommon);

	// テクスチャの初期化
	TextureManager::GetInstance()->Initialize();
	// 画像をロード
	uint32_t uvCheckerGH = TextureManager::Load("Resources/uvChecker.png", logStream);


	// dxcCompilerを初期化
	IDxcUtils* dxcUtils = nullptr;
	IDxcCompiler3* dxcCompiler = nullptr;
	HRESULT hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
	assert(SUCCEEDED(hr));
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
	assert(SUCCEEDED(hr));

	// 現時点でincludeはしないが、includeに対応するための設定を行っておく
	IDxcIncludeHandler* includeHandler = nullptr;
	hr = dxcUtils->CreateDefaultIncludeHandler(&includeHandler);
	assert(SUCCEEDED(hr));


	// RootSignature作成
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0; // 0から始まる
	descriptorRange[0].NumDescriptors = 1; // 数は1つ
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; // SRVを使う
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; // offsetを自動計算

	// RootParameter作成。PixelShaderのMaterialとVertexShaderのTransform
	D3D12_ROOT_PARAMETER rootParameters[4] = {};
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;  // CBVを使う
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;  // PixelShaderで使う
	rootParameters[0].Descriptor.ShaderRegister = 0;  // レジスタ番号0
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;  // CBVを使う
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;  // VertexShaderで使う
	rootParameters[1].Descriptor.ShaderRegister = 0;  // レジスタ番号0を使う
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // DescriptorTableを使う
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う
	rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange; // Tableの中身の配列を指定
	rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange); // Tableで利用する数
	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // CBVを使う
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShdaderで使う 
	rootParameters[3].Descriptor.ShaderRegister = 1; // レジスタ番号1を使う
	descriptionRootSignature.pParameters = rootParameters;  // ルートパラメータ配列へのポインタ
	descriptionRootSignature.NumParameters = _countof(rootParameters);  // 配列の長さ

	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR; // バイリニアフィルタ
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP; //0-1の範囲外をリピート 
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER; // 比較しない
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX; // ありったけのMIpmapを使う
	staticSamplers[0].ShaderRegister = 0; // レジスタ番号0を使う
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う 
	descriptionRootSignature.pStaticSamplers = staticSamplers;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);

	// シリアライズしてバイナリにする
	ID3DBlob* signatureBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	hr = D3D12SerializeRootSignature(&descriptionRootSignature,
		D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		Log(logStream,reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}
	// バイナリを元に生成
	ID3D12RootSignature* rootSignature = nullptr;
	hr = dxCommon->GetDevice()->CreateRootSignature(0,
		signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(),
		IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));

	// InputLayout
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[2].SemanticName = "NORMAL";
	inputElementDescs[2].SemanticIndex = 0;
	inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	// Blendstateの設定
	D3D12_BLEND_DESC blendDesc{};
	// すべての色要素を書き込む
	blendDesc.RenderTarget[0].RenderTargetWriteMask =
		D3D12_COLOR_WRITE_ENABLE_ALL;

	// RasiterzerStateの設定
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	// 裏面(時計回り)を表示しない
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	// 三角形の中を塗りつぶす
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	// Shaderをコンパイルする
	IDxcBlob* vertexShaderBlob = CompileShader(L"Object3d.VS.hlsl",
		L"vs_6_0", dxcUtils, dxcCompiler, includeHandler, logStream);
	assert(vertexShaderBlob != nullptr);

	IDxcBlob* pixelShaderBlob = CompileShader(L"Object3d.PS.hlsl",
		L"ps_6_0", dxcUtils, dxcCompiler, includeHandler, logStream);
	assert(pixelShaderBlob != nullptr);

	// DepthStencilStateの設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	// Depthの機能を有効化する
	depthStencilDesc.DepthEnable = true;
	// 書き込みします
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	// 比較関数はLessEqual。つまり、近ければ描画される
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	// PSO設定
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rootSignature;// RootSignature
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;// InputLayout
	graphicsPipelineStateDesc.BlendState = blendDesc; // BlendState
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc; // RasterizerState
	graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(),
	vertexShaderBlob->GetBufferSize() };// VertexShader
	graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(),
	pixelShaderBlob->GetBufferSize() };// PixelShader
	// DepthStencilの設定
	graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	
	// 書き込むRTVの情報
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	// 利用するトポロジ（形状）のタイプ。三角形
	graphicsPipelineStateDesc.PrimitiveTopologyType =
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	// どのように画面に色を打ち込んむかの設定（気にしなくて良い）
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	// 実際に生成
	ID3D12PipelineState* graphicsPipelineState = nullptr;
	hr = dxCommon->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc,
		IID_PPV_ARGS(&graphicsPipelineState));
	assert(SUCCEEDED(hr));

	//==========================================================================
	// 球の作成
	//==========================================================================

	// 球
	Sphere sphere;
	sphere.Initialize(dxCommon->GetDevice(),16);

	//===================================================================================
	// 2d用のspriteを作成
	//===================================================================================

	//// Sprite用の頂点リソースを作る
	//ID3D12Resource* vertexResourceSprite = CreateBufferResource(dxCommon->GetDevice(), sizeof(VertexData) * 4);
	//
	//// 頂点バッファビューを作成する
	//D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSprite{};
	//// リソースの先頭のアドレスから使う
	//vertexBufferViewSprite.BufferLocation = vertexResourceSprite->GetGPUVirtualAddress();
	//// 使用するリソースのサイズは頂点4つ分のサイズ
	//vertexBufferViewSprite.SizeInBytes = sizeof(VertexData) * 4;
	//// 1頂点当たりのサイズ
	//vertexBufferViewSprite.StrideInBytes = sizeof(VertexData);
	//
	//VertexData* vertexDataSprite = nullptr;
	//vertexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataSprite));
	//// 頂点インデックス
	//vertexDataSprite[0].position = { 0.0f,360.0f,0.0f,1.0f }; // 左下
	//vertexDataSprite[0].texcoord = { 0.0f,1.0f };
	//vertexDataSprite[0].normal = { 0.0f,0.0f,-1.0f };
	//vertexDataSprite[1].position = { 0.0f,0.0f,0.0f,1.0f }; // 左上
	//vertexDataSprite[1].texcoord = { 0.0f,0.0f };
	//vertexDataSprite[1].normal = { 0.0f,0.0f,-1.0f };
	//vertexDataSprite[2].position = { 640.0f,360.0f,0.0f,1.0f }; // 右下
	//vertexDataSprite[2].texcoord = { 1.0f,1.0f };
	//vertexDataSprite[2].normal = { 0.0f,0.0f,-1.0f };
	//vertexDataSprite[3].position = { 640.0f,0.0f,0.0f,1.0f }; // 左上
	//vertexDataSprite[3].texcoord = { 1.0f,0.0f };
	//vertexDataSprite[3].normal = { 0.0f,0.0f,-1.0f };
	//
	//// Sprite用の頂点インデックスのリソースを作る
	//ID3D12Resource* indexResourceSprite = CreateBufferResource(dxCommon->GetDevice(), sizeof(uint32_t) * 6);
	//// 頂点バッファビューを作成する
	//D3D12_INDEX_BUFFER_VIEW indexBufferViewSprite{};
	//// リソースの先頭のアドレスから使う
	//indexBufferViewSprite.BufferLocation = indexResourceSprite->GetGPUVirtualAddress();
	//// 使用するリソースのサイズはインデックス6つ分のサイズ
	//indexBufferViewSprite.SizeInBytes = sizeof(uint32_t) * 6;
	//// インデックスはuint32_tとする
	//indexBufferViewSprite.Format = DXGI_FORMAT_R32_UINT;
	//
	//// インデックスリソースにデータを書き込む
	//uint32_t* indexDataSprite = nullptr;
	//indexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&indexDataSprite));
	//// 三角形
	//indexDataSprite[0] = 0;  indexDataSprite[1] = 1;  indexDataSprite[2] = 2;
	//// 三角形2
	//indexDataSprite[3] = 1;  indexDataSprite[4] = 3;  indexDataSprite[5] = 2;
	//
	//// Sprite用のマテリアルリソースを作る
	//ID3D12Resource* materialResourceSprite = CreateBufferResource(dxCommon->GetDevice(), sizeof(Material));
	//// マテリアルにデータを書き込む
	//Material* materialDataSprite = nullptr;
	//// 書き込むためのアドレスを取得
	//materialResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&materialDataSprite));
	//// 白色に設定
	//materialDataSprite->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	//// SpriteはLightingしないのでfalseに設定する
	//materialDataSprite->enableLighting = false;
	//// UVTransform行列を初期化
	//materialDataSprite->uvTransform = MakeIdentity4x4();
	//
	//// Sprite用のTransformationMatrix用のリソースを作る。TransformationMatrix 1つ分のサイズを用意する
	//ID3D12Resource* transformationMatirxResourceSprite = CreateBufferResource(dxCommon->GetDevice(), sizeof(TransformationMatrix));
	//// データを書き込む
	//TransformationMatrix* transformationMatrixDataSprite = nullptr;
	//// 書き込むためのアドレスを取得
	//transformationMatirxResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixDataSprite));
	//// 単位行列を書き込んでおく
	//transformationMatrixDataSprite->WVP = MakeIdentity4x4();
	//transformationMatrixDataSprite->World = MakeIdentity4x4();

	//=======================================================================
	// 平行光源の設定
	//=======================================================================

	// 平行光源のリソースを作る。
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource = CreateBufferResource(dxCommon->GetDevice(), sizeof(DirectionalLight));
	// 平行光源のデータを作る
	DirectionalLight* directionalLightData = nullptr;
	// 書き込むためのアドレスを取得
	directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));
	// デフォルト値を設定
	directionalLightData->color = { 1.0f,1.0f,1.0f,1.0f };// 色
	directionalLightData->direction = { 0.0f,-1.0f,0.f };// 方向
	directionalLightData->intensity = 1.0f;// 輝度

	//=========================================================================
	// 宣言と初期化
	//=========================================================================

	// カメラ
	Camera camera;
	camera.Initialize({ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,-10.0f} }, windowsApp->kWindowWidth, windowsApp->kWindowHeight);

	// 球用
	Transform transform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
	Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	sphere.SetTransformationMatrix(camera.MakeWVPMatrix(worldMatrix));

	//// Sprite用のWorldViewProjectionMatrixを作る
	//Transform transformSprite{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
	//Matrix4x4 worldMatrixSprite = MakeAffineMatrix(transformSprite.scale, transformSprite.rotate, transformSprite.translate);
	//Matrix4x4 viewMatrixSprite = MakeIdentity4x4();
	//Matrix4x4 worldViewProjectionMatrixSprite = Multiply(worldMatrixSprite, Multiply(viewMatrixSprite, camera.GetOrthographicMatrix()));
	//transformationMatrixDataSprite->WVP = worldViewProjectionMatrixSprite;

	//// UVTransform用の変数
	//Transform uvTransformSprite{
	//	{1.0f,1.0f,1.0f},
	//	{0.0f,0.0f,0.0f},
	//	{0.0f,0.0f,0.0f}
	//};
	//Matrix4x4 uvTransformMatrix = MakeAffineMatrix(uvTransformSprite.scale, { 0.0f,0.0f,uvTransformSprite.rotate.z }, uvTransformSprite.translate);
	//materialDataSprite->uvTransform = uvTransformMatrix;

	// テクスチャを切り替えるフラグ
	bool useMonsterBall = true;

	//===========================================================================
	// メインループ
	//===========================================================================

	// ウィンドウのxボタンが押されるまでループ
	while (true) {
		if (windowsApp->ProcessMessage()) {
			break;
		}

		//=====================================================
		// 更新処理
		//=====================================================

		// ImGuiにフレームが始まる旨を伝える
		imGuiManager->BeginFrame();

		// 画像を動かすimGui
		ImGui::Begin("DebugWindow");
		//ImGui::SliderFloat3("transformSprite", &transformSprite.translate.x, 0.0f, 720.0f);
		ImGui::Checkbox("useMonsterBall", &useMonsterBall);

		ImGui::ColorEdit3("Light_Color", &directionalLightData->color.x); // 色を変更
		ImGui::SliderFloat3("Light_Direction", &directionalLightData->direction.x, -1.0f, 1.0f); // 方向を変更
		// 光源の方向を正規化
		directionalLightData->direction = Normalize(directionalLightData->direction);
		ImGui::SliderFloat("Light_Intensity", &directionalLightData->intensity, 0.0f, 10.0f); // 強度を変更

		//ImGui::DragFloat2("UVTranslate", &uvTransformSprite.translate.x, 0.01f, -10.0f, 10.0f);
		//ImGui::DragFloat2("UVScale", &uvTransformSprite.scale.x, 0.01f, -10.0f, 10.0f);
		//ImGui::SliderAngle("UVRotate", &uvTransformSprite.rotate.z);
		ImGui::End();

		// 球の処理
		transform.rotate.y += 0.03f;
		worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
		sphere.SetTransformationMatrix(camera.MakeWVPMatrix(worldMatrix));

		// 2d画像の処理
		//worldMatrixSprite = MakeAffineMatrix(transformSprite.scale, transformSprite.rotate, transformSprite.translate);
		//worldViewProjectionMatrixSprite = Multiply(worldMatrixSprite, Multiply(viewMatrixSprite, camera.GetOrthographicMatrix()));
		//transformationMatrixDataSprite->WVP = worldViewProjectionMatrixSprite;

		// UVTransform
		//uvTransformMatrix = MakeAffineMatrix(uvTransformSprite.scale, { 0.0f,0.0f,uvTransformSprite.rotate.z }, uvTransformSprite.translate);
		//materialDataSprite->uvTransform = uvTransformMatrix;

		// ImGuiの受付終了
		imGuiManager->EndFrame();

		//====================================================
		// 描画処理
		//====================================================

		// 描画前処理
		dxCommon->PreDraw(rootSignature, graphicsPipelineState);

		// 球の描画
		sphere.Draw(dxCommon->GetCommandList(), directionalLightResource.Get(), &TextureManager::GetTextureSrvHandlesGPU(uvCheckerGH));

		//// Spriteの描画。変更が必要なものだけ変更する
		//dxCommon->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferViewSprite);
		//dxCommon->GetCommandList()->IASetIndexBuffer(&indexBufferViewSprite);// IBVを設定
		//// マテリアルCBufferの場所を設定
		//dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResourceSprite->GetGPUVirtualAddress());
		//// SpriteがuvCheckerを描画するようにする
		//dxCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureSrvHandlesGPU[0]);
		//// TransformationMatrixCBufferの場所を設定
		//dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatirxResourceSprite->GetGPUVirtualAddress());
		//// 描画！(DrawCall/ドローコール) 6個のインデックスを使用し1つのインスタンスを描画。その他は当面0で良い
		//dxCommon->GetCommandList()->DrawIndexedInstanced(6, 1, 0, 0, 0);

		// ImGuiの描画処理
		imGuiManager->Draw();
		// 描画後処理
		dxCommon->PostDraw();
	}

	// ImGuiの解放処理
	imGuiManager->Finalize();

	// COMの終了処理
	CoUninitialize();

	/// 解放処理
	TextureManager::GetInstance()->Finalize();
	

	//vertexResource->Release();
	//materialResource->Release();

	sphere.Release();

	signatureBlob->Release();
	pixelShaderBlob->Release();
	vertexShaderBlob->Release();
	rootSignature->Release();
	graphicsPipelineState->Release();
	if (errorBlob) {
		errorBlob->Release();
	}

	// DirectXCommonの解放処理
	dxCommon->Finalize();

	windowsApp->BreakGameWindow();
	return 0;
}

