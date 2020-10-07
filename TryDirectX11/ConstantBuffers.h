#pragma once

#include "Bindable.h"
#include "GraphicsThrowMacros.h"

// 到目前为止，我们一直使用的是静态缓冲（static buffer），它的内容是在初始化时固定下来的。相比之下，动态缓冲（dynamic buffer）的内容可以在每一帧中进行修改。
// 当实现一些动画效果时，我们通常使用动态缓冲区。例如，我们要模拟一个水波效果，并通过函数 f(x ,z ,t) 来描述水波方程，计算当时间为 t 时，
// xz 平面上的每个点的高度。在这一情景中，我们必须使用 “山峰与河谷” 中的那种三角形网格，将每个网格点代入 f(x, z , t) 函数得到相应的水波高度。
// 由于该函数依赖于时间 t（即，水面会随着时间而变化），我们必须在很短的时间内（比如 1/30 秒）重新计算这些网格点，以得到较为平滑的动画。
// 所以，我们必须使用动态顶点缓冲区来实时更新三角形网格顶点的高度。
// 前面提到，为了获得一个动态缓冲区，我们必须在创建缓冲区时将 Usage 标志值指定为 D3D11_USAGE_DYNAMIC；
// 同时，由于我们要向缓冲区写入数据，所以必须将 CPU 访问标志值指定为 D3D11_CPU_ACCESS_WRITE。
template<typename C>
class ConstantBuffer : public Bindable {
public:
    void Update(Graphics &gfx, const C &consts) {
        INFOMAN(gfx);
        D3D11_MAPPED_SUBRESOURCE msr;
        // D3D11_USAGE_DYNAMIC：表示应用程序（CPU）会频繁更新资源中的数据内 容（例如，每帧更新一次）。GPU 可以从这种资源中读取数据，
        // 使用映射 API （ID3D11DeviceContext::Map）时，CPU可以向这种资源中写入数据。因为新的数据要从 CPU内存（即系统 RAM）传送到 GPU 内存（即显存），
        // 所以从 CPU 动态地更新GPU 资源会有性能损失；若非必须，请勿使用 D3D11_USAGE_DYNAMIC。

        // 1．pResource：指向要访问的用于读/写的资源的指针。缓冲是一种 Direct3D 11 资源，其他类型的资源，例如纹理资源，也可以使用这个方法进行访问。
        // 2．Subresource：包含在资源中的子资源的索引。后面我们会看到如何使用这个索引，而缓冲不包含子资源，所以设置为 0。
        // 3．MapType：常用的标志有以下几个：
        //   D3D11_MAP_WRITE_DISCARD：让硬件抛弃旧缓冲，返回一个指向新分配缓冲的指针，通过指定这个标志，可以让我们写入新分配的缓冲的同时，让硬件绘制已抛弃的缓冲中的内容，可以防止绘制停顿。
        //   D3D11_MAP_WRITE_NO_OVERWRITE：我们只会写入缓冲中未初始化的部分；通过指定这个标志，可以让我们写入未初始化的缓冲的同时，让硬件绘制前面已经写入的内容，可以防止绘制停顿。
        //   D3D11_MAP_READ：表示应用程序（CPU）会读取 GPU 缓冲的的一个副本到系统内存中。
        // 4．MapFlags：可选标志，这里不使用，所以设置为 0；具体细节可参见 SDK 文档。
        // 5．pMappedResource：返回一个指向 D3D11_MAPPED_SUBRESOURCE 的指针，这样我们就可以访问用于读/写的资源数据。
        GFX_THROW_INFO(GetContext(gfx)->Map(
                pConstantBuffer.Get(), 0u,
                D3D11_MAP_WRITE_DISCARD, 0u,
                &msr
        ));
        memcpy(msr.pData, &consts, sizeof(consts));
        GetContext(gfx)->Unmap(pConstantBuffer.Get(), 0u);
    }

    ConstantBuffer(Graphics &gfx, const C &consts) {
        INFOMAN(gfx);
        // D3D11_BUFFER_DESC 参数介绍看 VertexBuffer
        D3D11_BUFFER_DESC cbd = {};
        cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        cbd.Usage = D3D11_USAGE_DYNAMIC;
        cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        cbd.MiscFlags = 0u;
        cbd.ByteWidth = sizeof(consts);
        cbd.StructureByteStride = 0u;
        D3D11_SUBRESOURCE_DATA csd = {};
        csd.pSysMem = &consts;
        GFX_THROW_INFO(GetDevice(gfx)->CreateBuffer(&cbd, &csd, &pConstantBuffer));
    }

    ConstantBuffer(Graphics &gfx) {
        INFOMAN(gfx);
        // D3D11_BUFFER_DESC 参数介绍看 VertexBuffer
        D3D11_BUFFER_DESC cbd = {};
        cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        cbd.Usage = D3D11_USAGE_DYNAMIC;
        cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        cbd.MiscFlags = 0u;
        cbd.ByteWidth = sizeof(C);
        cbd.StructureByteStride = 0u;
        GFX_THROW_INFO(GetDevice(gfx)->CreateBuffer(&cbd, nullptr, &pConstantBuffer));
    }

protected:
    Microsoft::WRL::ComPtr<ID3D11Buffer> pConstantBuffer;
};

template<typename C>
class VertexConstantBuffer : public ConstantBuffer<C> {
    // 为了下面 GetContext 不用 ConstantBuffer:: 这样来引用
    using ConstantBuffer<C>::pConstantBuffer;
    using Bindable::GetContext;
public:
    using ConstantBuffer<C>::ConstantBuffer;
    void Bind(Graphics &gfx) noexcept override {
        // 设置顶点着色器的常量缓存
        // 和顶点缓冲相比，不用描述常数缓存的布局
        GetContext(gfx)->VSSetConstantBuffers(0u, 1u, pConstantBuffer.GetAddressOf());
    }

};

template<typename C>
class PixelConstantBuffer : public ConstantBuffer<C> {
    using ConstantBuffer<C>::pConstantBuffer;
    using Bindable::GetContext;
public:
    using ConstantBuffer<C>::ConstantBuffer;

    void Bind(Graphics &gfx) noexcept override {
        GetContext(gfx)->PSSetConstantBuffers(0u, 1u, pConstantBuffer.GetAddressOf());
    }
};