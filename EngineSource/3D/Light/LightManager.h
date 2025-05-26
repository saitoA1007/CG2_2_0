#pragma once
#include <d3d12.h>
#include <wrl.h>
#include<memory>
#include"DirectionalLight.h"
#include"PointLight.h"
#include"SpotLight.h"

namespace GameEngine {

    class LightManager {
    public:
        // 定数バッファ
        struct LightGroupData {
            DirectionalLight::DirectionalLightData directionalLightData_;
            PointLight::PointLightData pointLightData_;
            SpotLight::SpotLightData spotLightData_;
        };

    public:
        LightManager() = default;
        ~LightManager() = default;

        /// <summary>
        /// 初期化
        /// </summary>
        /// <param name="device"></param>
        void Initialize(ID3D12Device* device);

        /// <summary>
        /// 更新処理
        /// </summary>
        void Update();

    public:

        /// <summary>
        /// リソース
        /// </summary>
        /// <returns></returns>
        ID3D12Resource* GetResource()const {return lightGroupResource_.Get();}

        /// <summary>
        /// 平行光源のデータ設定
        /// </summary>
        /// <param name="directionalData"></param>
        void SetDirectionalData(const DirectionalLight::DirectionalLightData& directionalData);

        /// <summary>
        /// 平行光源の方向
        /// </summary>
        /// <param name="lightdir"></param>
        void SetDirectionalDirction(const Vector3& lightdir);

        /// <summary>
        /// 平行光源の有効化
        /// </summary>
        /// <param name="active"></param>
        void SetDirectionalLightActive(const bool& active);

        /// <summary>
        /// 点光源のデータ設定
        /// </summary>
        /// <param name="pointData"></param>
        void SetPointData(const PointLight::PointLightData& pointData);

        /// <summary>
        /// 点光源の位置
        /// </summary>
        /// <param name="position"></param>
        void SetPointLightPosition(const Vector3& position);

        /// <summary>
        /// 点光源の有効化
        /// </summary>
        /// <param name="active"></param>
        void SetPointLightActive(const bool& active);

        /// <summary>
        /// スポットライトのデータ設定
        /// </summary>
        /// <param name="spotData"></param>
        void SetSpotData(const SpotLight::SpotLightData& spotData);

        /// <summary>
        /// スポットライトの位置
        /// </summary>
        /// <param name="position"></param>
        void SetSpotLightPosition(const Vector3& position);

        /// <summary>
        /// スポットライトの有効化
        /// </summary>
        /// <param name="active"></param>
        void SetSpotLightActive(const bool& active);

        std::unique_ptr<DirectionalLight> directionalLight_;
        std::unique_ptr<PointLight> pointLight_;
        std::unique_ptr<SpotLight> spotLight_;

    private:
        // リソース
        Microsoft::WRL::ComPtr<ID3D12Resource> lightGroupResource_;

        // 平行光源のデータを作る
        LightGroupData* lightGroupData_ = nullptr;
    };

}
