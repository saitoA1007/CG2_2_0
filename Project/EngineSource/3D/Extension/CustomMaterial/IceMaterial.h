#pragma once
#include"ICustomMaterial.h"
#include <wrl.h>

#include"Vector4.h"
#include"Vector3.h"
#include"Matrix4x4.h"
#include<cstdint>

class IceMaterial : public ICustomMaterial {
public:

	struct alignas(16) MaterialData {
		Vector4 baseColor;
		Vector4 color;
		Matrix4x4 uvTransform;
		Vector3 specularColor;
		float shininess;
		uint32_t textureHandle;
		uint32_t normalTextureHandle;
		uint32_t baseTextureHandle;
		float time;
		Vector3 rimColor;
		float rimIntensity;
	};

public:
	IceMaterial() = default;
	~IceMaterial();

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