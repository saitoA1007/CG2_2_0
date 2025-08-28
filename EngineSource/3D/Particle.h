#pragma once
#include"EngineSource/Math/Matrix4x4.h"
#include"EngineSource/Math/Transform.h"
#include"EngineSource/Math/TransformationMatrix.h"

#include"VertexData.h"
#include"Mesh.h"
#include"Material.h"
#include"WorldTransforms.h"

#include"EngineSource/Core/ComputePSO/ParticleCSPSO.h"

#include <d3d12.h>

namespace GameEngine {
	class Particles {
	public:

		struct ParticleCS {
			Vector3 translate;
			Vector3 scale;
			float lifeTime;
			Vector3 velocity;
			float currentTime;
			Vector4 color;
		};

		struct PerView {
			Matrix4x4 viewProjection;
			Matrix4x4 billboardMatrix;
		};

	public:

		void Initialize();

		void Update();

		void Draw();

	private:

		static ID3D12Device* device_;
		static ID3D12GraphicsCommandList* commandList_;

		// PSO設定
		static ParticleCSPSO* particleCSPSO_;

		// 複数メッシュに対応
		std::unique_ptr<Mesh> mesh_;

		// 複数マテリアルに対応
		std::unique_ptr<Material> material_;
	};
}