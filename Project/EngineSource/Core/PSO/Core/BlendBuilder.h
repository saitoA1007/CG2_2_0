#pragma once
#include<d3d12.h>
#include<dxcapi.h>
#include<stdint.h>

#include"BlendMode.h"

namespace GameEngine {

	class BlendBuilder {
	public:

		/// <summary>
		/// 各ブレンドモードを作成
		/// </summary>
		void Initialize();

		/// <summary>
		/// 指定したブレンドモードを返す
		/// </summary>
		/// <param name="blendMode"></param>
		/// <returns></returns>
		D3D12_BLEND_DESC GetBlendDesc(BlendMode blendMode);

	private:

		D3D12_BLEND_DESC blendDesc_[BlendMode::kCountOfBlendMode];
	};
}