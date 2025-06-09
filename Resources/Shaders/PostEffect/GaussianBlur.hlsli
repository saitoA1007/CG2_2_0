
float32_t4 Get5x5GaussianBlur(Texture2D<float32_t4> tex, SamplerState smp, float32_t2 uv, float32_t dx, float32_t dy, float32_t4 rect)
{
    float32_t4 ret = tex.Sample(smp, uv);
    float32_t4 blurColor = float32_t4(0, 0, 0, 0);

    float32_t weights[5][5] =
    {
        { 1 / 273.0f, 4 / 273.0f, 7 / 273.0f, 4 / 273.0f, 1 / 273.0f },
        { 4 / 273.0f, 16 / 273.0f, 26 / 273.0f, 16 / 273.0f, 4 / 273.0f },
        { 7 / 273.0f, 26 / 273.0f, 41 / 273.0f, 26 / 273.0f, 7 / 273.0f },
        { 4 / 273.0f, 16 / 273.0f, 26 / 273.0f, 16 / 273.0f, 4 / 273.0f },
        { 1 / 273.0f, 4 / 273.0f, 7 / 273.0f, 4 / 273.0f, 1 / 273.0f }
    };

    float32_t offsets[5] = { -2.0f, -1.0f, 0.0f, 1.0f, 2.0f };

    for (int i = 0; i < 5; ++i)
    {
        for (int j = 0; j < 5; ++j)
        {
            float32_t2 offset = float32_t2(offsets[i] * dx, offsets[j] * dy);
            float32_t2 sampleUV = uv + offset;

            sampleUV.x = clamp(sampleUV.x, rect.x + dx * 0.5, rect.z - dx * 0.5);
            sampleUV.y = clamp(sampleUV.y, rect.y + dy * 0.5, rect.w - dy * 0.5);

            blurColor += tex.Sample(smp, sampleUV) * weights[i][j];
        }
    }

    return float32_t4(blurColor.rgb, ret.a);
}