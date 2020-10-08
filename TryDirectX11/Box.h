#pragma once
#include "DrawbleBase.h"

class Box : public DrawableBase<Box>
{
public:
    // mt19937 Mersenne Twister算法译为马特赛特旋转演算法，是伪随机数发生器之一
    Box(Graphics& gfx, std::mt19937& rng,
        std::uniform_real_distribution<float>& adist,
        std::uniform_real_distribution<float>& ddist,
        std::uniform_real_distribution<float>& odist,
        std::uniform_real_distribution<float>& rdist);
    void Update(float dt) noexcept override;
    DirectX::XMMATRIX GetTransformXM() const noexcept override;
private:
    // positional
    float r;
    float roll = 0.0f;
    float pitch = 0.0f;
    float yaw = 0.0f;
    float theta;
    float phi;
    float chi;
    // speed (delta/s)
    // 角速度
    float droll;
    float dpitch;
    float dyaw;
    // 世界空间中旋转
    float dtheta;
    float dphi;
    float dchi;
};