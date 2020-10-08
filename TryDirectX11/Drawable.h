#pragma once
#include "Graphics.h"
#include <DirectXMath.h>

class Bindable;

class Drawable
{
    template<class T>
    friend class DrawableBase; // 这样就能访问私有对象
public:
    Drawable() = default;
    Drawable(const Drawable&) = delete;
    virtual DirectX::XMMATRIX GetTransformXM() const noexcept = 0;
    void Draw(Graphics& gfx) const noexcept(!IS_DEBUG);
    virtual void Update(float dt) noexcept = 0;
    void AddBind(std::unique_ptr<Bindable> bind) noexcept(!IS_DEBUG);
    void AddIndexBuffer(std::unique_ptr<class IndexBuffer> ibuf) noexcept(!IS_DEBUG);
    virtual ~Drawable() = default;
private:
    // Drawable 也要访问 Static Bind
    virtual const std::vector<std::unique_ptr<Bindable>>& GetStaticBinds() const noexcept = 0;
private:
    const IndexBuffer* pIndexBuffer = nullptr;
    std::vector<std::unique_ptr<Bindable>> binds;
};