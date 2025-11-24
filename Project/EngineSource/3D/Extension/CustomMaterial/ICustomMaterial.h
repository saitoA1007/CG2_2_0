#pragma once
#include <d3d12.h>

class ICustomMaterial {
public:
	virtual ~ICustomMaterial() = default;

	// リソースを取得
	virtual ID3D12Resource* GetResource() = 0;
};