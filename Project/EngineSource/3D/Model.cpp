#include"Model.h"
#include<fstream>
#include<sstream>
#include<cassert>

#include"CreateBufferResource.h"
#include"MyMath.h"
#include"EasingManager.h"

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
AnimationPSO* Model::animationPSO_ = nullptr;

void Model::StaticInitialize(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, TextureManager* textureManager, TrianglePSO* trianglePSO, ParticlePSO* particlePSO, AnimationPSO* animationPSO, GridPSO* gridPSO, LogManager* logManager) {
	device_ = device;
	commandList_ = commandList;
	logManager_ = logManager;
	textureManager_ = textureManager;
	trianglePSO_ = trianglePSO;
	particlePSO_ = particlePSO;
	gridPSO_ = gridPSO;
	animationPSO_ = animationPSO;
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

void Model::PreDraw(BasePSO* pso) {
	commandList_->SetGraphicsRootSignature(pso->GetRootSignature());  // RootSignatureを設定。
	commandList_->SetPipelineState(pso->GetPipelineState()); // 指定したPSOを設定
}

void Model::PreDrawAnimation() {
	commandList_->SetGraphicsRootSignature(animationPSO_->GetRootSignature());  // RootSignatureを設定。
	commandList_->SetPipelineState(animationPSO_->GetPipelineState()); // 指定したPSOを設定
}

[[nodiscard]]
std::unique_ptr<Model> Model::CreateSphere(uint32_t subdivision) {

	// インスタンスを生成
	std::unique_ptr<Model> model = std::make_unique<Model>();

	// メッシュを作成
	std::unique_ptr<Mesh> tmpMesh = std::make_unique<Mesh>();
	tmpMesh->CreateSphereMesh(device_, subdivision);
	model->meshes_.push_back(std::move(tmpMesh));

	// マテリアルを作成
	std::unique_ptr<Material> tmpMaterial = std::make_unique<Material>();
	tmpMaterial->Initialize({ 1.0f,1.0f,1.0f,1.0f }, { 1.0f,1.0f,1.0f }, 500.0f, false);
	model->materials_[model->meshes_[0]->GetMaterialName()] = std::move(tmpMaterial);

	return model;
}

[[nodiscard]]
std::unique_ptr<Model> Model::CreateTrianglePlane() {

	// インスタンスを生成
	std::unique_ptr<Model> model = std::make_unique<Model>();

	// メッシュを作成
	std::unique_ptr<Mesh> tmpMesh = std::make_unique<Mesh>();
	tmpMesh->CreateTrianglePlaneMesh(device_);
	model->meshes_.push_back(std::move(tmpMesh));

	// マテリアルを作成
	std::unique_ptr<Material> tmpMaterial = std::make_unique<Material>();
	tmpMaterial->Initialize({ 1.0f,1.0f,1.0f,1.0f }, { 1.0f,1.0f,1.0f }, 500.0f, false);
	model->materials_[model->meshes_[0]->GetMaterialName()] = std::move(tmpMaterial);

	return model;
}

[[nodiscard]]
std::unique_ptr<Model> Model::CreatePlane(const Vector2& size) {

	// インスタンスを生成
	std::unique_ptr<Model> model = std::make_unique<Model>();

	// メッシュを作成
	std::unique_ptr<Mesh> tmpMesh = std::make_unique<Mesh>();
	tmpMesh->CreatePlaneMesh(device_,size);
	model->meshes_.push_back(std::move(tmpMesh));

	// マテリアルを作成
	std::unique_ptr<Material> tmpMaterial = std::make_unique<Material>();
	tmpMaterial->Initialize({ 1.0f,1.0f,1.0f,1.0f }, { 1.0f,1.0f,1.0f }, 500.0f, false);
	model->materials_[model->meshes_[0]->GetMaterialName()] = std::move(tmpMaterial);

	return model;
}

[[nodiscard]]
std::unique_ptr<Model> Model::CreateGridPlane(const Vector2& size) {

	// インスタンスを生成
	std::unique_ptr<Model> model = std::make_unique<Model>();

	// メッシュを作成
	std::unique_ptr<Mesh> tmpMesh = std::make_unique<Mesh>();
	tmpMesh->CreateGridPlaneMesh(device_, size);
	model->meshes_.push_back(std::move(tmpMesh));

	return model;
}

[[nodiscard]]
std::unique_ptr<Model> Model::CreateModel(const std::string& objFilename, const std::string& filename) {

	// インスタンスを生成
	std::unique_ptr<Model> model = std::make_unique<Model>();

	// Assimpを使ったモデルの生成するログを出す
	if (logManager_) {
		logManager_->Log("\nCreate From Assimp : Start loading Model file: " + filename + objFilename);
	}

	// データを読み込む処理
	if (logManager_) {
		logManager_->Log("Create From Assimp : Loading Model file data");
	}
	ModelData modelData = model->LoadModelFile(kDirectoryPath_, objFilename, filename);

	// モデルが無事に作成されたログを出す
	if (logManager_) {
		logManager_->Log("Create From Assimp : Success loaded Model file: " + filename + objFilename);
	}

	// メッシュを作成
	for (uint32_t index = 0; index < modelData.meshes.size(); ++index) {
		std::unique_ptr<Mesh> tmpMesh = std::make_unique<Mesh>();
		tmpMesh->CreateModelMesh(device_, modelData, index);

		model->meshes_.push_back(std::move(tmpMesh));
	}

	// マテリアルを作成
	for (uint32_t index = 0; index < modelData.materials.size(); ++index) {
		std::unique_ptr<Material> tmpMaterial = std::make_unique<Material>();
		tmpMaterial->Initialize(modelData.materials[index].color, modelData.materials[index].specularColor, modelData.materials[index].shininess, false);

		// テクスチャ情報があればを取得
		if (!modelData.materials[index].textureFilePath.empty()) {

			uint32_t textureHandle = textureManager_->Load(modelData.materials[index].textureFilePath);
			tmpMaterial->SetTextureHandle(textureHandle);
		}

		model->materials_[modelData.materials[index].name] = std::move(tmpMaterial);
	}

	// ローカル行列を取得
	model->localMatrix_ = modelData.rootNode.localMatrix;

	// モデルの名前を取得
	model->modelName_ = modelData.rootNode.name;

	// モデルのロード
	model->isLoad_ = true;

	return model;
}

// 描画
void Model::Draw(WorldTransform& worldTransform, const uint32_t& textureHandle, const Matrix4x4& VPMatrix, const Material* material) {

	// カメラ座標に変換
	if (isLoad_) {
		worldTransform.SetWVPMatrix(localMatrix_, VPMatrix);
	} else {
		worldTransform.SetWVPMatrix(VPMatrix);
	}

	for (uint32_t i = 0; i < meshes_.size(); ++i) {
		commandList_->IASetVertexBuffers(0, 1, &meshes_[i]->GetVertexBufferView());
		commandList_->IASetIndexBuffer(&meshes_[i]->GetIndexBufferView());
		commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// マテリアルを設定
		auto it = materials_.find(meshes_[i]->GetMaterialName());
		assert(it != materials_.end() && "Material not found");
		const Material* drawMaterial = it->second.get();

		// マテリアルが設定されていなければデフォルトのマテリアルを使う
		if (material == nullptr) {
			commandList_->SetGraphicsRootConstantBufferView(0, drawMaterial->GetMaterialResource()->GetGPUVirtualAddress());
		} else {
			commandList_->SetGraphicsRootConstantBufferView(0, material->GetMaterialResource()->GetGPUVirtualAddress());
		}
		commandList_->SetGraphicsRootConstantBufferView(1, worldTransform.GetTransformResource()->GetGPUVirtualAddress());
		commandList_->SetGraphicsRootDescriptorTable(2, textureManager_->GetTextureSrvHandlesGPU(textureHandle));

		if (meshes_[i]->GetTotalIndices() != 0) {
			commandList_->DrawIndexedInstanced(meshes_[i]->GetTotalIndices(), 1, 0, 0, 0);
		} else {
			commandList_->DrawInstanced(meshes_[i]->GetTotalVertices(), 1, 0, 0);
		}
	}
}

void Model::Draw(WorldTransform& worldTransform, const Matrix4x4& VPMatrix, const Material* material) {
	// カメラ座標に変換
	if (isLoad_) {
		worldTransform.SetWVPMatrix(localMatrix_, VPMatrix);
	} else {
		worldTransform.SetWVPMatrix(VPMatrix);
	}

	for (uint32_t i = 0; i < meshes_.size(); ++i) {
		commandList_->IASetVertexBuffers(0, 1, &meshes_[i]->GetVertexBufferView());
		commandList_->IASetIndexBuffer(&meshes_[i]->GetIndexBufferView());
		commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// マテリアルを設定
		auto it = materials_.find(meshes_[i]->GetMaterialName());
		assert(it != materials_.end() && "Material not found");
		const Material* drawMaterial = it->second.get();

		// マテリアルが設定されていなければデフォルトのマテリアルを使う
		if (material == nullptr) {
			commandList_->SetGraphicsRootConstantBufferView(0, drawMaterial->GetMaterialResource()->GetGPUVirtualAddress());
		} else {
			commandList_->SetGraphicsRootConstantBufferView(0, material->GetMaterialResource()->GetGPUVirtualAddress());
		}
		commandList_->SetGraphicsRootConstantBufferView(1, worldTransform.GetTransformResource()->GetGPUVirtualAddress());
		commandList_->SetGraphicsRootDescriptorTable(2, textureManager_->GetTextureSrvHandlesGPU(drawMaterial->GetTextureHandle()));

		if (meshes_[i]->GetTotalIndices() != 0) {
			commandList_->DrawIndexedInstanced(meshes_[i]->GetTotalIndices(), 1, 0, 0, 0);
		} else {
			commandList_->DrawInstanced(meshes_[i]->GetTotalVertices(), 1, 0, 0);
		}
	}
}

void Model::Draw(WorldTransform& worldTransform, const Matrix4x4& VPMatrix, ID3D12Resource* lightGroupResource, ID3D12Resource* cameraResource, const Material* material) {
	// カメラ座標に変換
	if (isLoad_) {
		worldTransform.SetWVPMatrix(localMatrix_, VPMatrix);
	} else {
		worldTransform.SetWVPMatrix(VPMatrix);
	}

	for (uint32_t i = 0; i < meshes_.size(); ++i) {
		commandList_->IASetVertexBuffers(0, 1, &meshes_[i]->GetVertexBufferView());
		commandList_->IASetIndexBuffer(&meshes_[i]->GetIndexBufferView());
		commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// マテリアルを設定
		auto it = materials_.find(meshes_[i]->GetMaterialName());
		assert(it != materials_.end() && "Material not found");
		const Material* drawMaterial = it->second.get();

		// マテリアルが設定されていなければデフォルトのマテリアルを使う
		if (material == nullptr) {
			commandList_->SetGraphicsRootConstantBufferView(0, drawMaterial->GetMaterialResource()->GetGPUVirtualAddress());
		} else {
			commandList_->SetGraphicsRootConstantBufferView(0, material->GetMaterialResource()->GetGPUVirtualAddress());
		}
		commandList_->SetGraphicsRootConstantBufferView(1, worldTransform.GetTransformResource()->GetGPUVirtualAddress());
		commandList_->SetGraphicsRootDescriptorTable(2, textureManager_->GetTextureSrvHandlesGPU(drawMaterial->GetTextureHandle()));
		commandList_->SetGraphicsRootConstantBufferView(3, lightGroupResource->GetGPUVirtualAddress());
		commandList_->SetGraphicsRootConstantBufferView(4, cameraResource->GetGPUVirtualAddress());
		if (meshes_[i]->GetTotalIndices() != 0) {
			commandList_->DrawIndexedInstanced(meshes_[i]->GetTotalIndices(), 1, 0, 0, 0);
		} else {
			commandList_->DrawInstanced(meshes_[i]->GetTotalVertices(), 1, 0, 0);
		}
	}
}

void Model::Draw(const uint32_t& numInstance,WorldTransforms& worldTransforms, const uint32_t& textureHandle, const Matrix4x4& VPMatrix, const Material* material) {
	// カメラ座標に変換
	if (isLoad_) {
		worldTransforms.SetWVPMatrix(numInstance, localMatrix_, VPMatrix);
	} else {
		worldTransforms.SetWVPMatrix(numInstance, VPMatrix);
	}

	for (uint32_t i = 0; i < meshes_.size(); ++i) {
		commandList_->IASetVertexBuffers(0, 1, &meshes_[i]->GetVertexBufferView());
		commandList_->IASetIndexBuffer(&meshes_[i]->GetIndexBufferView());
		commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// マテリアルを設定
		auto it = materials_.find(meshes_[i]->GetMaterialName());
		assert(it != materials_.end() && "Material not found");
		const Material* drawMaterial = it->second.get();

		// マテリアルが設定されていなければデフォルトのマテリアルを使う
		if (material == nullptr) {
			commandList_->SetGraphicsRootConstantBufferView(0, drawMaterial->GetMaterialResource()->GetGPUVirtualAddress());
		} else {
			commandList_->SetGraphicsRootConstantBufferView(0, material->GetMaterialResource()->GetGPUVirtualAddress());
		}
		commandList_->SetGraphicsRootDescriptorTable(1, *worldTransforms.GetInstancingSrvGPU());
		commandList_->SetGraphicsRootDescriptorTable(2, textureManager_->GetTextureSrvHandlesGPU(textureHandle));

		if (meshes_[i]->GetTotalIndices() != 0) {
			commandList_->DrawIndexedInstanced(meshes_[i]->GetTotalIndices(), 1, 0, 0, 0);
		} else {
			commandList_->DrawInstanced(meshes_[i]->GetTotalVertices(), 1, 0, 0);
		}
	}
}

void Model::Draw(const uint32_t& numInstance, WorldTransforms& worldTransforms, const Matrix4x4& VPMatrix, const Material* material) {
	// カメラ座標に変換
	if (isLoad_) {
		worldTransforms.SetWVPMatrix(numInstance, localMatrix_, VPMatrix);
	} else {
		worldTransforms.SetWVPMatrix(numInstance, VPMatrix);
	}

	for (uint32_t i = 0; i < meshes_.size(); ++i) {
		commandList_->IASetVertexBuffers(0, 1, &meshes_[i]->GetVertexBufferView());
		commandList_->IASetIndexBuffer(&meshes_[i]->GetIndexBufferView());
		commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// マテリアルを設定
		auto it = materials_.find(meshes_[i]->GetMaterialName());
		assert(it != materials_.end() && "Material not found");
		const Material* drawMaterial = it->second.get();

		// マテリアルが設定されていなければデフォルトのマテリアルを使う
		if (material == nullptr) {
			commandList_->SetGraphicsRootConstantBufferView(0, drawMaterial->GetMaterialResource()->GetGPUVirtualAddress());
		} else {
			commandList_->SetGraphicsRootConstantBufferView(0, material->GetMaterialResource()->GetGPUVirtualAddress());
		}
		commandList_->SetGraphicsRootDescriptorTable(1, *worldTransforms.GetInstancingSrvGPU());
		commandList_->SetGraphicsRootDescriptorTable(2, textureManager_->GetTextureSrvHandlesGPU(drawMaterial->GetTextureHandle()));

		if (meshes_[i]->GetTotalIndices() != 0) {
			commandList_->DrawIndexedInstanced(meshes_[i]->GetTotalIndices(), numInstance, 0, 0, 0);
		} else {
			commandList_->DrawInstanced(meshes_[i]->GetTotalVertices(), numInstance, 0, 0);
		}
	}
}

void Model::DrawLight(ID3D12Resource* lightGroupResource, ID3D12Resource* cameraResource) {
	commandList_->SetGraphicsRootConstantBufferView(3, lightGroupResource->GetGPUVirtualAddress());
	commandList_->SetGraphicsRootConstantBufferView(4, cameraResource->GetGPUVirtualAddress());
}

void Model::DrawAnimation(WorldTransform& worldTransform, const Matrix4x4& VPMatrix, const SkinCluster& skinCluster, const Material* material) {
	// カメラ座標に変換
	worldTransform.SetWVPMatrix(VPMatrix);

	for (uint32_t i = 0; i < meshes_.size(); ++i) {

		D3D12_VERTEX_BUFFER_VIEW vbvs[2] = {
			meshes_[i]->GetVertexBufferView(),
			skinCluster.influenceBufferView
		};

		commandList_->IASetVertexBuffers(0, 2, vbvs);
		commandList_->IASetIndexBuffer(&meshes_[i]->GetIndexBufferView());
		commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// マテリアルを設定
		auto it = materials_.find(meshes_[i]->GetMaterialName());
		assert(it != materials_.end() && "Material not found");
		const Material* drawMaterial = it->second.get();

		// マテリアルが設定されていなければデフォルトのマテリアルを使う
		if (material == nullptr) {
			commandList_->SetGraphicsRootConstantBufferView(0, drawMaterial->GetMaterialResource()->GetGPUVirtualAddress());
		} else {
			commandList_->SetGraphicsRootConstantBufferView(0, material->GetMaterialResource()->GetGPUVirtualAddress());
		}
		commandList_->SetGraphicsRootConstantBufferView(1, worldTransform.GetTransformResource()->GetGPUVirtualAddress());
		commandList_->SetGraphicsRootDescriptorTable(2, textureManager_->GetTextureSrvHandlesGPU(drawMaterial->GetTextureHandle()));

		commandList_->SetGraphicsRootDescriptorTable(3, skinCluster.paletteSrvHandle.second);

		if (meshes_[i]->GetTotalIndices() != 0) {
			commandList_->DrawIndexedInstanced(meshes_[i]->GetTotalIndices(), 1, 0, 0, 0);
		} else {
			commandList_->DrawInstanced(meshes_[i]->GetTotalVertices(), 1, 0, 0);
		}
	}
}

void Model::DrawGrid(WorldTransform& worldTransform, const Matrix4x4& VPMatrix, ID3D12Resource* cameraResource) {

	worldTransform.SetWVPMatrix(VPMatrix);

	commandList_->IASetVertexBuffers(0, 1, &meshes_[0]->GetVertexBufferView());
	commandList_->IASetIndexBuffer(&meshes_[0]->GetIndexBufferView());
	commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList_->SetGraphicsRootConstantBufferView(0, worldTransform.GetTransformResource()->GetGPUVirtualAddress());
	commandList_->SetGraphicsRootConstantBufferView(1, cameraResource->GetGPUVirtualAddress());
	commandList_->DrawIndexedInstanced(meshes_[0]->GetTotalIndices(), 1, 0, 0, 0);
}

[[nodiscard]]
ModelData Model::LoadModelFile(const std::string& directoryPath, const std::string& objFilename, const std::string& filename) {

	ModelData modelData;

	// ファイルを読み込み
	Assimp::Importer importer;
	std::string filePath = directoryPath + "/" + filename + "/" + objFilename;
	const aiScene* scene = importer.ReadFile(filePath.c_str(), aiProcess_Triangulate | aiProcess_FlipWindingOrder | aiProcess_FlipUVs);
	assert(scene && scene->HasMeshes()); // メッシュがないのは対応しない

	// Material解析
	for (uint32_t materialIndex = 0; materialIndex < scene->mNumMaterials; ++materialIndex) {
		aiMaterial* material = scene->mMaterials[materialIndex];
		LoadMaterialData materialData;

		// マテリアル名を取得
		aiString name;
		material->Get(AI_MATKEY_NAME, name);
		materialData.name = name.C_Str();

		if (name.length > 0) {
			materialData.name = name.C_Str();
		}

		// Assimpのデフォルトマテリアルまたは空のマテリアル名をスキップ
		if (materialData.name == "DefaultMaterial" || materialData.name.empty()) {
			continue;
		}

		// テクスチャを取得
		if (material->GetTextureCount(aiTextureType_DIFFUSE) != 0) {
			aiString textureFilePath;
			if (material->GetTexture(aiTextureType_DIFFUSE, 0, &textureFilePath) == AI_SUCCESS) {
				materialData.textureFilePath = directoryPath + "/" + filename + "/" + textureFilePath.C_Str();
			}
		}

		// 色を取得
		aiColor3D diffuseColor(1.0f, 1.0f, 1.0f);
		if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor)) {
			materialData.color = { diffuseColor.r, diffuseColor.g, diffuseColor.b, 1.0f };
		}

		// 鏡面反射の色を取得
		aiColor3D specularColor(1.0f, 1.0f, 1.0f);
		if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_SPECULAR, specularColor)) {
			materialData.specularColor = { specularColor.r, specularColor.g, specularColor.b };
		}

		// 輝度
		float shininess = 0.0f;
		if (AI_SUCCESS == material->Get(AI_MATKEY_SHININESS, shininess)) {
			materialData.shininess = shininess;
		}

		modelData.materials.push_back(std::move(materialData));
	}

	// Mesh解析
	for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
		aiMesh* mesh = scene->mMeshes[meshIndex];
		assert(mesh->HasNormals()); // 法線がないMeshは今回は非対応

		// 最初に頂点分メモリを確保する
		MeshData meshData;
		meshData.vertices.resize(mesh->mNumVertices);
		// メッシュに対応するマテリアル名を取得する
		aiMaterial* meshMaterial = scene->mMaterials[mesh->mMaterialIndex];
		aiString materialName;
		meshMaterial->Get(AI_MATKEY_NAME, materialName);

		if (materialName.length > 0) {
			meshData.materialName = materialName.C_Str();
		}

		// Assimpのデフォルトマテリアルまたは空のマテリアル名をスキップ
		if (meshData.materialName == "DefaultMaterial" || meshData.materialName.empty()) {
			continue;
		}

		// Vertex解析
		for (uint32_t vertexIndex = 0; vertexIndex < mesh->mNumVertices; ++vertexIndex) {
			aiVector3D& position = mesh->mVertices[vertexIndex];
			aiVector3D& normal = mesh->mNormals[vertexIndex];
			VertexData vertex;
			// 右手->左手に変換する
			vertex.position = { -position.x, position.y, position.z, 1.0f };
			vertex.normal = { -normal.x, normal.y, normal.z };

			// UVの適応
			if (mesh->HasTextureCoords(0)) {
				aiVector3D& texcoord = mesh->mTextureCoords[0][vertexIndex];
				vertex.texcoord = { texcoord.x, texcoord.y };
			} else {
				// UVがない場合、XZ平面に投影したUVを仮生成
				vertex.texcoord = { (position.x + 1.0f) * 0.5f, (position.z + 1.0f) * 0.5f };
			}

			meshData.vertices[vertexIndex] = vertex;
		}

		// Face解析
		for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex) {
			aiFace& face = mesh->mFaces[faceIndex];
			assert(face.mNumIndices >= 3);
			for (uint32_t i = 0; i < face.mNumIndices; ++i) {
				meshData.indices.push_back(face.mIndices[i]);
			}
		}

		for (uint32_t boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
			aiBone* bone = mesh->mBones[boneIndex];
			std::string jointName = bone->mName.C_Str();
			JointWeightData& jointWeightData = modelData.skinClusterData[jointName];

			aiMatrix4x4 bindPoseMatrixAssimp = bone->mOffsetMatrix.Inverse();
			aiVector3D scale, translate;
			aiQuaternion rotate;
			bindPoseMatrixAssimp.Decompose(scale, rotate, translate);
			Matrix4x4 bindPoseMatrix = MakeAffineMatrix({ scale.x,scale.y,scale.z }, { rotate.x,-rotate.y,-rotate.z,rotate.w }, { -translate.x,translate.y,translate.z });
			jointWeightData.inverseBindPoseMatrix = InverseMatrix(bindPoseMatrix);

			for (uint32_t weightIndex = 0; weightIndex < bone->mNumWeights; ++weightIndex) {
				jointWeightData.vertexWeights.push_back({ bone->mWeights[weightIndex].mWeight,bone->mWeights[weightIndex].mVertexId });
			}
		}

		modelData.meshes.push_back(std::move(meshData));
	}

	// シーン全体の階層構造を作る
	modelData.rootNode = ReadNode(scene->mRootNode);

	modelData_ = modelData;

	return modelData;
}

void  Model::SetDefaultColor(const Vector4& color, const std::string& materialName) {

	auto it = materialName == "default" ? materials_.begin() : materials_.find(materialName);

	assert(it != materials_.end() && "Material not found");
	Material* material = it->second.get();
	material->SetColor(color);
}

void Model::SetDefaultSpecularColor(const Vector3& specularColor, const std::string& materialName) {

	auto it = materialName == "default" ? materials_.begin() : materials_.find(materialName);

	assert(it != materials_.end() && "Material not found");
	Material* material = it->second.get();
	material->SetSpecularColor(specularColor);
}

void Model::SetDefaultShiness(const float& shininess, const std::string& materialName) {

	auto it = materialName == "default" ? materials_.begin() : materials_.find(materialName);

	assert(it != materials_.end() && "Material not found");
	Material* material = it->second.get();
	material->SetShiness(shininess);
}

void  Model::SetDefaultIsEnableLight(const bool& isEnableLight, const std::string& materialName) {

	auto it = materialName == "default" ? materials_.begin() : materials_.find(materialName);

	assert(it != materials_.end() && "Material not found");
	Material* material = it->second.get();
	material->SetEnableLighting(isEnableLight);
}

void  Model::SetDefaultUVMatrix(const Matrix4x4& uvMatrix, const std::string& materialName) {
	
	auto it = materialName == "default" ? materials_.begin() : materials_.find(materialName);

	assert(it != materials_.end() && "Material not found");
	Material* material = it->second.get();
	material->SetUVMatrix(uvMatrix);
}

void Model::SetDefaultUVMatrix(const Transform& uvTransform, const std::string& materialName) {

	auto it = materialName == "default" ? materials_.begin() : materials_.find(materialName);

	assert(it != materials_.end() && "Material not found");
	Material* material = it->second.get();
	material->SetUVTransform(uvTransform);
}

[[nodiscard]]
Node Model::ReadNode(aiNode* node) {
	Node result;

	aiVector3D scale, translate;
	aiQuaternion rotate;
	node->mTransformation.Decompose(scale, rotate, translate);
	result.transform.scale = { scale.x,scale.y,scale.z };
	result.transform.rotate = { rotate.x,-rotate.y,-rotate.z,rotate.w }; // x軸を反転、さらに回転方向が逆なので軸を反転する
	result.transform.translate = { -translate.x,translate.y,translate.z }; // x軸を反転
	result.localMatrix = MakeAffineMatrix(result.transform.scale, result.transform.rotate, result.transform.translate);

	result.name = node->mName.C_Str(); // Node名を格納
	result.children.resize(node->mNumChildren); // 子供の数だけ確保
	for (uint32_t childIndex = 0; childIndex < node->mNumChildren; ++childIndex) {
		// 再帰的に読んで階層構造を作っていく
		result.children[childIndex] = ReadNode(node->mChildren[childIndex]);
	}
	return result;
}

[[nodiscard]]
Skeleton Model::CreateSkeleton(const Node& rootNode) {

	Skeleton skeleton;
	skeleton.root = CreateJoint(rootNode, {}, skeleton.joints);

	// 名前とindexのマッピングを行いアクセスしやすくする
	for (const Joint& joint : skeleton.joints) {
		skeleton.jointMap.emplace(joint.name, joint.index);
	}
	return skeleton;
}

[[nodiscard]]
int32_t Model::CreateJoint(const Node& node, const std::optional<int32_t>& parent, std::vector<Joint>& joints) {

	Joint joint;
	joint.name = node.name;
	joint.localMatrix = node.localMatrix;
	joint.skeletonSpaceMatrix = MakeIdentity4x4();
	joint.transform = node.transform;
	joint.index = static_cast<int32_t>(joints.size()); // 現在登録されている数をIndexに
	joint.parent = parent;
	joints.push_back(joint); // SkeletonのJoint列に追加
	for (const Node& child : node.children) {
		// 子Jointを作成し、そのIndexを登録
		int32_t childIndex = CreateJoint(child, joint.index, joints);
		joints[joint.index].children.push_back(childIndex);
	}
	return joint.index;
}

[[nodiscard]]
AnimationData Model::LoadAnimationFile(const std::string& objFilename, const std::string& filename) {

	// Assimpを使ったモデルの生成するログを出す
	if (logManager_) {
		logManager_->Log("Load AnimationData From Assimp : Start loading Model file: " + filename + objFilename);
	}

	AnimationData animation;
	Assimp::Importer importer;
	std::string filePath = kDirectoryPath_ + "/" + filename + "/" + objFilename;
	const aiScene* scene = importer.ReadFile(filePath.c_str(), 0);
	assert(scene->mNumAnimations != 0); // アニメーションがない
	aiAnimation* animationAssimp = scene->mAnimations[0];  // 最初のアニメーションだけ採用。
	animation.duration = float(animationAssimp->mDuration / animationAssimp->mTicksPerSecond); // 時間の単位を秒に変換

	// assimpでは個々のNodeのAnimationをchannelと呼んでいるのでchannelを回してNodeAnimationの情報を取ってくる
	for (uint32_t channelIndex = 0; channelIndex < animationAssimp->mNumChannels; ++channelIndex) {
		aiNodeAnim* nodeAnimationAssimp = animationAssimp->mChannels[channelIndex];
		NodeAnimation& nodeAnimation = animation.nodeAnimations[nodeAnimationAssimp->mNodeName.C_Str()];

		// 位置
		for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumPositionKeys; ++keyIndex) {
			aiVectorKey& keyAssimp = nodeAnimationAssimp->mPositionKeys[keyIndex];
			KeyframeVector3 keyframe;
			keyframe.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond);  // ここも秒に変換
			keyframe.value = { -keyAssimp.mValue.x,keyAssimp.mValue.y,keyAssimp.mValue.z }; // 右手->左手
			nodeAnimation.translate.push_back(keyframe);
		}
		// 回転
		for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumRotationKeys; ++keyIndex) {
			aiQuatKey& keyAssimp = nodeAnimationAssimp->mRotationKeys[keyIndex];
			KeyframeQuaternion keyframe;
			keyframe.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond);
			keyframe.value = { keyAssimp.mValue.x,-keyAssimp.mValue.y, -keyAssimp.mValue.z, keyAssimp.mValue.w };
			nodeAnimation.rotate.push_back(keyframe);
		}
		// 拡縮
		for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumScalingKeys; ++keyIndex) {
			aiVectorKey& keyAssimp = nodeAnimationAssimp->mScalingKeys[keyIndex];
			KeyframeVector3 keyframe;
			keyframe.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond);
			keyframe.value = { keyAssimp.mValue.x,keyAssimp.mValue.y, keyAssimp.mValue.z };
			nodeAnimation.scale.push_back(keyframe);
		}
	}

	// モデルが無事に作成されたログを出す
	if (logManager_) {
		logManager_->Log("Load AnimationData From Assimp : Success loaded Model file: " + filename + objFilename);
	}

	// 解析結果を返す
	return animation;
}