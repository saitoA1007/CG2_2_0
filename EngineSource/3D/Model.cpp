#include"Model.h"
#include<fstream>
#include<sstream>
#include<cassert>
#include"EngineSource/Core/TextureManager.h"
#include"EngineSource/Common/CreateBufferResource.h"
#include"EngineSource/Math/MyMath.h"
#include"WorldTransform.h"

#include<assimp/Importer.hpp>
#include<assimp/scene.h>
#include<assimp/postprocess.h>

using namespace GameEngine;

ID3D12Device* Model::device_ = nullptr;
ID3D12GraphicsCommandList* Model::commandList_ = nullptr;
TrianglePSO* Model::trianglePSO_ = nullptr;
ParticlePSO* Model::particlePSO_ = nullptr;
LogManager* Model::logManager_ = nullptr;
TextureManager* Model::textureManager_ = nullptr;

GridPSO* Model::gridPSO_ = nullptr;

void Model::StaticInitialize(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, TextureManager* textureManager, TrianglePSO* trianglePSO, ParticlePSO* particlePSO, GridPSO* gridPSO, LogManager* logManager) {
	device_ = device;
	commandList_ = commandList;
	logManager_ = logManager;
	textureManager_ = textureManager;
	trianglePSO_ = trianglePSO;
	particlePSO_ = particlePSO;
	gridPSO_ = gridPSO;
}

void Model::PreDraw(PSOMode psoMode, BlendMode blendMode) {
	
	switch (psoMode) {

		// 単体描画設定
	case PSOMode::Triangle:
		commandList_->SetGraphicsRootSignature(trianglePSO_->GetRootSignature());  // RootSignatureを設定。
		commandList_->SetPipelineState(trianglePSO_->GetPipelineState(blendMode)); // trianglePSOを設定
		break;

		// 複数描画設定
	case PSOMode::Partilce:
		commandList_->SetGraphicsRootSignature(particlePSO_->GetRootSignature());  // RootSignatureを設定。
		commandList_->SetPipelineState(particlePSO_->GetPipelineState(blendMode)); // particlePSOを設定
		break;

		// グリッド描画設定
	case PSOMode::Grid:
		commandList_->SetGraphicsRootSignature(gridPSO_->GetRootSignature());  // RootSignatureを設定。
		commandList_->SetPipelineState(gridPSO_->GetPipelineState()); // trianglePSOを設定
		break;
	}
}

void Model::PreDraw(DrawModel drowMode) {
	commandList_->SetGraphicsRootSignature(trianglePSO_->GetRootSignature());  // RootSignatureを設定。
	commandList_->SetPipelineState(trianglePSO_->GetDrawModePipelineState(drowMode)); // trianglePSOを設定
}

[[nodiscard]]
Model* Model::CreateSphere(uint32_t subdivision) {

	Model* model = new Model();

	// 頂点数とインデックス数を計算
	model->totalVertices_ = (subdivision + 1) * (subdivision + 1);
	model->totalIndices_ = subdivision * subdivision * 6;

	// 頂点バッファを作成
	// vertexResourceを作成
	model->vertexResource_ = CreateBufferResource(device_, sizeof(VertexData) * model->totalVertices_);
	// リソースの先頭のアドレスから使う
	model->vertexBufferView_.BufferLocation = model->vertexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点3つ分のサイズ
	model->vertexBufferView_.SizeInBytes = sizeof(VertexData) * model->totalVertices_;
	// 1頂点あたりのサイズ
	model->vertexBufferView_.StrideInBytes = sizeof(VertexData);

	// 頂点データを生成
	// 頂点リソースにデータを書き込む
	VertexData* vertexData = nullptr;
	// 書き込むためのアドレスを取得
	model->vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	// 緯度分割1つ分の角度
	const float kLatEvery = static_cast<float>(M_PI) / static_cast<float>(subdivision);
	// 経度分割1つ分の角度
	const float kLonEvery = 2.0f * static_cast<float>(M_PI) / static_cast<float>(subdivision);
	for (uint32_t latIndex = 0; latIndex <= subdivision; ++latIndex) {
		float lat = -static_cast<float>(M_PI) / 2.0f + kLatEvery * latIndex;
		float v = 1.0f - static_cast<float>(latIndex) / static_cast<float>(subdivision);
		for (uint32_t lonIndex = 0; lonIndex <= subdivision; ++lonIndex) {
			float lon = lonIndex * kLonEvery;
			float u = static_cast<float>(lonIndex) / static_cast<float>(subdivision);
			uint32_t start = latIndex * (subdivision + 1) + lonIndex;

			vertexData[start].position.x = cos(lat) * cos(lon);
			vertexData[start].position.y = sin(lat);
			vertexData[start].position.z = cos(lat) * sin(lon);
			vertexData[start].position.w = 1.0f;
			vertexData[start].texcoord = { u, v };
			vertexData[start].normal = { vertexData[start].position.x, vertexData[start].position.y, vertexData[start].position.z };
		}
	}

	// インデックスバッファを作成
	// 球用の頂点インデックスのリソースを作る
	model->indexResource_ = CreateBufferResource(device_, sizeof(uint32_t) * model->totalIndices_);
	// リソースの先頭のアドレスから使う
	model->indexBufferView_.BufferLocation = model->indexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズはインデックス6つ分のサイズ
	model->indexBufferView_.SizeInBytes = sizeof(uint32_t) * model->totalIndices_;
	// インデックスはuint32_tとする
	model->indexBufferView_.Format = DXGI_FORMAT_R32_UINT;

	// インデックスデータを生成
	// インデックスリソースにデータを書き込む
	uint32_t* indexData = nullptr;
	model->indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
	uint32_t index = 0;
	for (uint32_t latIndex = 0; latIndex < subdivision; ++latIndex) {
		for (uint32_t lonIndex = 0; lonIndex < subdivision; ++lonIndex) {
			uint32_t a = (latIndex * (subdivision + 1)) + lonIndex;
			uint32_t b = a + subdivision + 1;
			uint32_t c = a + 1;
			uint32_t d = b + 1;

			// 三角形1
			indexData[index] = a;
			index++;
			indexData[index] = b;
			index++;
			indexData[index] = d;
			index++;

			// 三角形2
			indexData[index] = a;
			index++;
			indexData[index] = d;
			index++;
			indexData[index] = c;
			index++;
		}
	}

	// マテリアルを作成
	model->defaultMaterial_ = std::make_unique<Material>();
	model->defaultMaterial_->Initialize({1.0f,1.0f,1.0f,1.0f}, { 1.0f,1.0f,1.0f },0.0f,false);

	return model;
}

[[nodiscard]]
Model* Model::CreateTrianglePlane() {

	Model* model = new Model();

	// 頂点数とインデックス数を計算
	model->totalVertices_ = 3;
	model->totalIndices_ = 0;

	// 頂点バッファを作成
	// vertexResourceを作成
	model->vertexResource_ = CreateBufferResource(device_, sizeof(VertexData) * model->totalVertices_);
	// リソースの先頭のアドレスから使う
	model->vertexBufferView_.BufferLocation = model->vertexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点3つ分のサイズ
	model->vertexBufferView_.SizeInBytes = sizeof(VertexData) * model->totalVertices_;
	// 1頂点あたりのサイズ
	model->vertexBufferView_.StrideInBytes = sizeof(VertexData);

	// 頂点データを生成
	// 頂点リソースにデータを書き込む
	VertexData* vertexData = nullptr;
	// 書き込むためのアドレスを取得
	model->vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	// 左下
	vertexData[0].position = { -0.5f,-0.5f,0.0f,1.0f };
	vertexData[0].position.w = 1.0f;
	vertexData[0].texcoord = { 0.0f,1.0f };
	vertexData[0].normal = { vertexData[0].position.x, vertexData[0].position.y, vertexData[0].position.z };
	// 上
	vertexData[1].position = { 0.0f,0.5f,0.0f,1.0f };
	vertexData[1].position.w = 1.0f;
	vertexData[1].texcoord = { 0.5f,0.0f };
	vertexData[1].normal = { vertexData[1].position.x, vertexData[1].position.y, vertexData[1].position.z };
	// 右下
	vertexData[2].position = { 0.5f,-0.5f,0.0f,1.0f };
	vertexData[2].position.w = 1.0f;
	vertexData[2].texcoord = { 1.0f,1.0f };
	vertexData[2].normal = { vertexData[2].position.x, vertexData[2].position.y, vertexData[2].position.z };

	// マテリアルを作成
	model->defaultMaterial_ = std::make_unique<Material>();
	model->defaultMaterial_->Initialize({ 1.0f,1.0f,1.0f,1.0f }, { 1.0f,1.0f,1.0f }, 0.0f, false);

	return model;
}

[[nodiscard]]
Model* Model::CreateGridPlane(const Vector2& size) {

	Model* model = new Model();

	// 頂点数とインデックス数を計算
	model->totalVertices_ = 4;
	model->totalIndices_ = 6;

	// 頂点バッファを作成
	// vertexResourceを作成
	model->vertexResource_ = CreateBufferResource(device_, sizeof(GridVertexData) * model->totalVertices_);
	// リソースの先頭のアドレスから使う
	model->vertexBufferView_.BufferLocation = model->vertexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点3つ分のサイズ
	model->vertexBufferView_.SizeInBytes = sizeof(GridVertexData) * model->totalVertices_;
	// 1頂点あたりのサイズ
	model->vertexBufferView_.StrideInBytes = sizeof(GridVertexData);

	// 頂点データを生成
	// 頂点リソースにデータを書き込む
	GridVertexData* vertexData = nullptr;
	// 書き込むためのアドレスを取得
	model->vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

	float left = -size.x / 2.0f;
	float right = size.x / 2.0f;
	float top = size.y / 2.0f;
	float bottom = -size.y / 2.0f;

	// 左上
	vertexData[0].position = { left,0.0f,top,1.0f }; // 左下
	// 右上
	vertexData[1].position = { right,0.0f,top,1.0f }; // 左上
	// 左下
	vertexData[2].position = { left,0.0f,bottom,1.0f }; // 右下
	// 右下
	vertexData[3].position = { right,0.0f,bottom,1.0f }; // 左上

	// インデックスバッファを作成
	// 球用の頂点インデックスのリソースを作る
	model->indexResource_ = CreateBufferResource(device_, sizeof(uint32_t) * model->totalIndices_);
	// リソースの先頭のアドレスから使う
	model->indexBufferView_.BufferLocation = model->indexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズはインデックス6つ分のサイズ
	model->indexBufferView_.SizeInBytes = sizeof(uint32_t) * model->totalIndices_;
	// インデックスはuint32_tとする
	model->indexBufferView_.Format = DXGI_FORMAT_R32_UINT;

	// インデックスデータを生成
	// インデックスリソースにデータを書き込む
	uint32_t* indexData = nullptr;
	model->indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
	// 三角形
	indexData[0] = 0;  indexData[1] = 1;  indexData[2] = 2;
	// 三角形2
	indexData[3] = 1;  indexData[4] = 3;  indexData[5] = 2;

	return model;
}

[[nodiscard]]
Model* Model::CreateModel(const std::string& objFilename, const std::string& filename) {

	if (logManager_) {
		logManager_->Log("\nCreateFromOBJ : Start loading OBJ file: " + filename + objFilename);
	}

	Model* model = new Model();

	// データを読み込む処理
	if (logManager_) {
		logManager_->Log("CreateFromOBJ : Loading OBJ file data");
	}
	ModelData modelData = model->LoadModelFile("Resources", objFilename, filename);

	// 描画する時に利用する頂点数
	model->totalVertices_ = UINT(modelData.vertices.size());
	model->totalIndices_ = 0;

	// 頂点リソースを作る
	if (logManager_) {
		logManager_->Log("CreateFromOBJ : Creating vertexResource");
	}
	model->vertexResource_ = CreateBufferResource(device_, sizeof(VertexData) * modelData.vertices.size());
	// 頂点バッファビューを作成する
	model->vertexBufferView_.BufferLocation = model->vertexResource_->GetGPUVirtualAddress();// リソースの先頭のアドレスから使う
	model->vertexBufferView_.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size());// 使用するリソースのサイズは頂点サイズ
	model->vertexBufferView_.StrideInBytes = sizeof(VertexData);// 1頂点あたりのサイズ

	// 頂点リソースにデータを書き込む
	VertexData* vertexData = nullptr;
	model->vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));// 書き込むためのアドレスを取得
	std::memcpy(vertexData, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size());// 頂点データをリソースにコピー

	// マテリアルを作成
	model->defaultMaterial_ = std::make_unique<Material>();
	model->defaultMaterial_->Initialize(modelData.material.color, modelData.material.specularColor, modelData.material.shininess, false);

	// OBJが無事に作成されたログを出す
	if (logManager_) {
		logManager_->Log("CreateFromOBJ : Success loaded OBJ file: " + filename + objFilename);
	}

	model->localMatrix_ = modelData.rootNode.localMatrix;

	return model;
}

// 描画
void Model::Draw(WorldTransform& worldTransform, const uint32_t& textureHandle, const Matrix4x4& VPMatrix,const Material* material) {

	// カメラ座標に変換
	worldTransform.SetWVPMatrix(localMatrix_,VPMatrix);

	commandList_->IASetVertexBuffers(0, 1, &vertexBufferView_);
	commandList_->IASetIndexBuffer(&indexBufferView_);
	commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// マテリアルが設定されていなければデフォルトのマテリアルを使う
	if (material == nullptr) {
		commandList_->SetGraphicsRootConstantBufferView(0, defaultMaterial_->GetMaterialResource()->GetGPUVirtualAddress());
	} else {
		commandList_->SetGraphicsRootConstantBufferView(0, material->GetMaterialResource()->GetGPUVirtualAddress());
	}
	commandList_->SetGraphicsRootConstantBufferView(1, worldTransform.GetTransformResource()->GetGPUVirtualAddress());
	commandList_->SetGraphicsRootDescriptorTable(2, textureManager_->GetTextureSrvHandlesGPU(textureHandle));
	if (totalIndices_ != 0) {
		commandList_->DrawIndexedInstanced(totalIndices_, 1, 0, 0, 0);
	} else {
		commandList_->DrawInstanced(totalVertices_, 1, 0, 0);
	}
}

void Model::Draw(const uint32_t& numInstance,WorldTransforms& worldTransforms, const uint32_t& textureHandle, const Matrix4x4& VPMatrix, const Material* material) {
	// カメラ座標に変換
	worldTransforms.SetWVPMatrix(numInstance,VPMatrix);

	commandList_->IASetVertexBuffers(0, 1, &vertexBufferView_);
	commandList_->IASetIndexBuffer(&indexBufferView_);
	commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// マテリアルが設定されていなければデフォルトのマテリアルを使う
	if (material == nullptr) {
		commandList_->SetGraphicsRootConstantBufferView(0, defaultMaterial_->GetMaterialResource()->GetGPUVirtualAddress());
	} else {
		commandList_->SetGraphicsRootConstantBufferView(0, material->GetMaterialResource()->GetGPUVirtualAddress());
	}
	commandList_->SetGraphicsRootDescriptorTable(1, *worldTransforms.GetInstancingSrvGPU());
	commandList_->SetGraphicsRootDescriptorTable(2, textureManager_->GetTextureSrvHandlesGPU(textureHandle));
	if (totalIndices_ != 0) {
		commandList_->DrawIndexedInstanced(totalIndices_, numInstance, 0, 0, 0);
	} else {
		commandList_->DrawInstanced(totalVertices_, numInstance, 0, 0);
	}
}

void Model::DrawLight(ID3D12Resource* lightGroupResource, ID3D12Resource* cameraResource) {
	commandList_->SetGraphicsRootConstantBufferView(3, lightGroupResource->GetGPUVirtualAddress());
	commandList_->SetGraphicsRootConstantBufferView(4, cameraResource->GetGPUVirtualAddress());
}

void Model::DrawGrid(WorldTransform& worldTransform, const Matrix4x4& VPMatrix, ID3D12Resource* cameraResource) {

	worldTransform.SetWVPMatrix(VPMatrix);

	commandList_->IASetVertexBuffers(0, 1, &vertexBufferView_);
	commandList_->IASetIndexBuffer(&indexBufferView_);
	commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList_->SetGraphicsRootConstantBufferView(0, worldTransform.GetTransformResource()->GetGPUVirtualAddress());
	commandList_->SetGraphicsRootConstantBufferView(1, cameraResource->GetGPUVirtualAddress());
	commandList_->DrawIndexedInstanced(totalIndices_, 1, 0, 0, 0);
}

[[nodiscard]]
Model::ModelData Model::LoadModelFile(const std::string& directoryPath, const std::string& objFilename, const std::string& filename) {

	ModelData modelData; // 構築するModelData
	
	// objファイルを読み込み
	Assimp::Importer importer;
	std::string filePath = directoryPath + "/" + filename + "/" + objFilename;
	const aiScene* scene = importer.ReadFile(filePath.c_str(), aiProcess_Triangulate | aiProcess_FlipWindingOrder | aiProcess_FlipUVs);
	assert(scene->HasMeshes()); // メッシュがないのは対応しない
		
	// Mesh解析
	for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
		aiMesh* mesh = scene->mMeshes[meshIndex];
		assert(mesh->HasNormals()); // 法線がないMeshは今回は非対応
		assert(mesh->HasTextureCoords(0)); // TexcoordがないMeshは今回は非対応

		// Face解析
		for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex) {
			aiFace& face = mesh->mFaces[faceIndex];
			assert(face.mNumIndices >= 3); // 三角形より大きければ通す

			// Vertex解析
			for (uint32_t element = 0; element < face.mNumIndices; ++element) {
				uint32_t vertexIndex = face.mIndices[element];
				aiVector3D& position = mesh->mVertices[vertexIndex];
				aiVector3D& normal = mesh->mNormals[vertexIndex];
				aiVector3D& texcoord = mesh->mTextureCoords[0][vertexIndex];
				VertexData vertex;
				vertex.position = { position.x,position.y,position.z,1.0f };
				vertex.normal = { normal.x,normal.y,normal.z };
				vertex.texcoord = { texcoord.x,texcoord.y};
				// 右手->左手に変換する
				vertex.position.x *= -1.0f;
				vertex.normal.x *= -1.0f;
				modelData.vertices.push_back(vertex);
			}
		}
	}

	// Material解析
	for (uint32_t materialIndex = 0; materialIndex < scene->mNumMaterials; ++materialIndex) {
		aiMaterial* material = scene->mMaterials[materialIndex];
		// テクスチャを取得
		if (material->GetTextureCount(aiTextureType_DIFFUSE) != 0) {
			aiString textureFilePath;
			material->GetTexture(aiTextureType_DIFFUSE, 0, &textureFilePath);
			modelData.material.textureFilePath = directoryPath + "/" + filename + "/" + textureFilePath.C_Str();
		}

		// 色を取得
		aiColor3D diffuseColor(1.0f, 1.0f, 1.0f);   // デフォルト値（白）
		if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor)) {
			modelData.material.color = { diffuseColor.r, diffuseColor.g, diffuseColor.b,1.0f };
		}

		// 鏡面反射の色を取得
		aiColor3D specularColor(1.0f, 1.0f, 1.0f);   // デフォルト値（白）
		if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_SPECULAR, specularColor)) {
			modelData.material.specularColor = { specularColor.r, specularColor.g, specularColor.b};
		}

		// 輝度
		float shininess = 0.0f; // デフォルト値
		if (AI_SUCCESS == material->Get(AI_MATKEY_SHININESS, shininess)) {
			modelData.material.shininess = shininess;
		}
	}

	// シーン全体の階層構造を作る
	modelData.rootNode = ReadNode(scene->mRootNode);

	return modelData;
}

void  Model::SetDefaultColor(const Vector4& color) {
	defaultMaterial_->SetColor(color);
}

void Model::SetDefaultSpecularColor(const Vector3& specularColor) {
	defaultMaterial_->SetSpecularColor(specularColor);
}

void Model::SetDefaultShiness(const float& shininess) {
	defaultMaterial_->SetShiness(shininess);
}

void  Model::SetDefaultIsEnableLight(const bool& isEnableLight) {
	defaultMaterial_->SetEnableLighting(isEnableLight);
}

void  Model::SetDefaultUVMatrix(const Matrix4x4& uvMatrix) {
	defaultMaterial_->SetUVMatrix(uvMatrix);
}

[[nodiscard]]
Model::Node Model::ReadNode(aiNode* node) {
	Node result;

	aiMatrix4x4 aiLocalMatrix = node->mTransformation; // nodeのlocalMatrixを取得
	aiLocalMatrix.Transpose(); // 列ベクトル形式を行ベクトル形式に転置
	for (uint32_t y = 0; y < 4; ++y) {
		for (uint32_t x = 0; x < 4; ++x) {
			result.localMatrix.m[y][x] = aiLocalMatrix[y][x];
		}
	}

	result.name = node->mName.C_Str(); // Node名を格納
	result.children.resize(node->mNumChildren); // 子供の数だけ確保
	for (uint32_t childIndex = 0; childIndex < node->mNumChildren; ++childIndex) {
		// 再帰的に読んで階層構造を作っていく
		result.children[childIndex] = ReadNode(node->mChildren[childIndex]);
	}
	return result;
}