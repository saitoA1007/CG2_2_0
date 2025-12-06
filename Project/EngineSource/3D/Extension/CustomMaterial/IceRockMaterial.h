#pragma once
#include"ICustomMaterial.h"
#include <wrl.h>

#include"Vector4.h"
#include"Vector3.h"
#include"Matrix4x4.h"
#include<cstdint>

class IceRockMaterial : public ICustomMaterial {
public:

	struct alignas(16) MaterialData {
		Vector4 color;
		int32_t enableLighting;
		float padding[3];
		Matrix4x4 uvTransform;
		Vector3 specularColor;
		float shininess;
		uint32_t textureHandle;
		Vector3 rimColor;
		float rimPower;
		float rimIntensity;
		float padding2[2];
	};

public:
	IceRockMaterial() = default;
	~IceRockMaterial();

	static void StaticInitialize(ID3D12Device* device);

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// リソースを取得
	/// </summary>
	/// <returns></returns>
	ID3D12Resource* GetResource() override { return materialResource_.Get(); }

public:

	// マテリアルにデータを書き込む
	MaterialData* materialData_ = nullptr;

private:

	static ID3D12Device* device_;

	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
};
