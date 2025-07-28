#pragma once
#include"EngineSource/Math/Vector4.h"
#include"EngineSource/Math/Vector3.h"
#include"EngineSource/Math/Vector2.h"
#include"EngineSource/Math/Matrix4x4.h"

#include<iostream>
#include<vector>

struct VertexData {
	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;
};

struct LoadMaterialData {
	std::string textureFilePath;
	Vector4 color = { 1.0f,1.0f,1.0f,1.0f };
	Vector3 specularColor = { 1.0f,1.0f,1.0f };
	float shininess = 0.0f;
};

struct Node {
	Matrix4x4 localMatrix;
	std::string name;
	std::vector<Node> children;
};

struct MeshData {
	std::vector<VertexData> vertices;
	std::vector<uint32_t> indices;
	uint32_t materialIndex; // 使用するマテリアル
};

struct ModelData {
	std::vector<MeshData> meshes;
	std::vector<LoadMaterialData> materials;
	Node rootNode;
};

struct GridVertexData {
	Vector4 position;
};