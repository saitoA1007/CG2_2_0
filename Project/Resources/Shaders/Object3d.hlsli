
struct VertexShaderOutput
{
    float32_t4 position : SV_POSITION;
    float32_t2 texcoord : TEXCOORD1;
    float32_t3 normal : NORMAL1;
    float32_t3 worldPosition : POSITION1;
};

struct DirectionalLight
{
    float32_t4 color; // ライトの色
    float32_t3 direction; // ライトの向き
    float32_t intensity; // 輝度
    int32_t active;
};

struct PointLight
{
    float32_t4 color; // ライトの色
    float32_t3 position; // ライトの位置
    float32_t intensity; // 輝度
    int32_t active; // 有効化
    float32_t radius; // ライトの届く最大距離
    float32_t decay; // 減衰率
};

// 点光源の数
static const int POINTLIGHT_NUM = 3;

struct SpotLight
{
    float32_t4 color; // ライトの色
    float32_t3 position; // ライトの位置
    float32_t intensity; // 輝度
    float32_t3 direction; // ライトの方向
    float32_t distance; // ライトの最大距離
    float32_t decay; // 減衰率
    float32_t cosAngle; // 減衰率
    float32_t cosFalloffStart; // 
    int32_t active; // 有効化
};

// スポットライトの数
static const int SPOTLIGHT_NUM = 3;

struct AreaLight
{
    float32_t4 color; // ライトの色
    float32_t3 position; // ライトの中心位置
    float32_t intensity; // 輝度
    float32_t3 right; // ライトの右方向ベクトル
    float32_t width; // ライトの幅
    float32_t3 up; // ライトの上方向ベクトル
    float32_t height; // ライトの高さ
    float32_t distance; // ライトの届く最大距離
    float32_t decay; // 減衰率
    int32_t active; // 有効化
};

// 面光源の数
static const int AREALIGHT_NUM = 3;

// 環境マップ
struct EnvironmentData
{
    uint32_t textureIndex;
    int32_t isActive;
};

struct Skinned
{
    float32_t4 position;
    float32_t3 normal;
};