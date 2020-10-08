#pragma once
#include "Drawable.h"
#include "IndexBuffer.h"

// 这个类是为了重用某些 Bindable 而写的，例如要生成 80 个正方体，就不用再实例化 80 次 indexbuffer、shader 之类的
// Bindable 资源，它们完全可以只实例化一次。
template<class T>
class DrawableBase : public Drawable
{
public:
    bool IsStaticInitialized() const noexcept
    {
        return !staticBinds.empty();
    }
    void AddStaticBind( std::unique_ptr<Bindable> bind ) noexcept(!IS_DEBUG)
    {
        assert( "*Must* use AddIndexBuffer to bind index buffer" && typeid(*bind) != typeid(IndexBuffer) );
        staticBinds.push_back( std::move( bind ) );
    }
    void AddStaticIndexBuffer( std::unique_ptr<IndexBuffer> ibuf ) noexcept(!IS_DEBUG)
    {
        assert( "Attempting to add index buffer a second time" && pIndexBuffer == nullptr );
        pIndexBuffer = ibuf.get();
        staticBinds.push_back( std::move( ibuf ) );
    }

    // 因为 Drawable Draw 中对每个 Cube 都要调用 pIndexBuffer->GetCount()，而如果这个 Cube 没有初始化
    // （我们修改成某些 Bindable 只需初始化一次），pIndexBuffer 就是空的，因此这里是重新在 staticBinds 里面再找出来
    void SetIndexFromStatic() noexcept(!IS_DEBUG)
    {
        assert( "Attempting to add index buffer a second time" && pIndexBuffer == nullptr );
        for( const auto& b : staticBinds )
        {
            if( const auto p = dynamic_cast<IndexBuffer*>(b.get()) )
            {
                pIndexBuffer = p;
                return;
            }
        }
        assert( "Failed to find index buffer in static binds" && pIndexBuffer != nullptr );
    }
private:
    const std::vector<std::unique_ptr<Bindable>>& GetStaticBinds() const noexcept override
    {
        return staticBinds;
    }
private:
    static std::vector<std::unique_ptr<Bindable>> staticBinds;
};

template<class T>
std::vector<std::unique_ptr<Bindable>> DrawableBase<T>::staticBinds;