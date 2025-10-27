#pragma once
#include <d3d12.h>
#include<dxcapi.h>
#include<vector>
#include<string>

namespace GameEngine {

	class InputLayoutBuilder {
	public:

		/// <summary>
		/// 入力要素を登録する
		/// </summary>
		/// <param name="name"></param>
		/// <param name="index"></param>
		/// <param name="format"></param>
		void CreateInputElement(const std::string& name, uint32_t index, uint32_t slotIndex, DXGI_FORMAT format);

		/// <summary>
		/// 登録した入力要素を確定する
		/// </summary>
		void CreateInputLayoutDesc();

		/// <summary>
		/// リセットする
		/// </summary>
		void Reset();

		// 通常のオブジェクト用
		void CreateDefaultObjElement();
		// 通常の画像用
		void CreateDefaultSpriteElement();
		// 通常の線用
		void CreateDefaultLineElement();
		// 通常のアニメーション用
		void CreateDefaultAnimationElement();

	private:

		std::vector<D3D12_INPUT_ELEMENT_DESC> inputElementDescs_;
		D3D12_INPUT_LAYOUT_DESC inputLayoutDesc_{};
	};
}

