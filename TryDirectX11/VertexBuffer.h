#pragma once
#include "Bindable.h"
#include "GraphicsThrowMacros.h"

class VertexBuffer : public Bindable
{
public:
    template<class V>
    VertexBuffer(Graphics& gfx, const std::vector<V>& vertices)
        :
        stride(sizeof(V))
    {
        INFOMAN(gfx);
        // 要创建一个顶点缓冲，我们必须执行以下步骤：
        // 1．填写一个 D3D11_BUFFER_DESC 结构体，描述我们所要创建的缓冲区。
        // 2．填写一个 D3D11_SUBRESOURCE_DATA 结构体，为缓冲区指定初始化数据。
        // 3．调用 ID3D11Device::CreateBuffer 方法来创建缓冲区。

        // 1．填写一个 D3D11_BUFFER_DESC 结构体，描述我们所要创建的缓冲区。
        D3D11_BUFFER_DESC bd = {};
        // 对于顶点缓冲区，该参数应设为 D3D11_BIND_VERTEX_BUFFER。
        bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        // 一个用于指定缓冲区用途的 D3D11_USAGE 枚举类型成员。有 4 个可选值：
        //（a）D3D10_USAGE_DEFAULT：表示 GPU 会对资源执行读写操作。在使用映射 API
        //  （例如 ID3D11DeviceContext::Map）时，CPU 在使用映射 API 时不能读写这种资源，但它
        //   能使用 ID3D11DeviceContext::UpdateSubresource。ID3D11DeviceContext::Map 方法会在 6.14 节中介绍。
        //（b）D3D11_USAGE_IMMUTABLE：表示在创建资源后，资源中的内容不会改变。
        //   这样可以获得一些内部优化，因为 GPU 会以只读方式访问这种资源。除了在创建资源时 CPU
        //   会写入初始化数据外，其他任何时候 CPU 都不会对这种资源执行任何读写操作，我们也无法映射或更新一个 immutable 资源。
        //（c）D3D11_USAGE_DYNAMIC：表示应用程序（CPU）会频繁更新资源中的数据内容（例如 ，每帧更新一次）。
        //   GPU 可以从这种资源中读取数据 ，使用映射 API（ID3D11DeviceContext::Map）时，CPU 可以向这种资源中写入数据。
        //   因为新的数据要从 CPU 内存（即系统 RAM）传送到 GPU 内存（即显存），所以从 CPU 动态地更新 GPU 资源
        //   会有性能损失；若非必须，请勿使用 D3D11_USAGE_DYNAMIC。
        //（d）D3D11_USAGE_STAGING：表示应用程序（CPU）会读取该资源的一个副本（即，
        //   该资源支持从显存到系统内存的数据复制操作）。显存到系统内存的复制是一个缓慢的操作，
        //   应尽量避免 。使用 ID3D11DeviceContext::CopyResource 和
        //   ID3D11DeviceContext::CopySubresourceRegion 方法可以复制资源，在 12.3.5 节会介绍一个复制资源的例子。
        bd.Usage = D3D11_USAGE_DEFAULT;
        // 指定 CPU 对资源的访问权限。设置为 0 则表示 CPU 无需读写缓冲。
        // 如果 CPU 需要向资源写入数据，则应指定 D3D11_CPU_ACCESS_WRITE。具有写访问权限的资源的 Usage 参数应设为 D3D11_USAGE_DYNAMIC 或 D3D11_USAGE_STAGING。
        // 如果 CPU 需要从资源读取数据 ，则应指定 D3D11_CPU_ACCESS_READ 。具有读访问权限的资源的 Usage 参数应设为 D3D11_USAGE_STAGING。
        // 当指定这些标志值时，应按需而定。通常，CPU 从 Direct3D 资源读取数据的速度较慢。CPU 向资源写入数据的速度虽然较快，
        // 但是把内存副本传回显存的过程仍很耗时。所以，最好的做法是（如果可能的话）不指定任何标志值，让资源驻留在显存中，只用 GPU 来读写数据。
        bd.CPUAccessFlags = 0u;
        // 我们不需要为顶点缓冲区指定任何杂项（miscellaneous）标志值，所以该参数设为 0。
        // 有关 D3D11_RESOURCE_MISC_FLAG 枚举类型的详情请参阅 SDK 文档。
        bd.MiscFlags = 0u;
        // 我们将要创建的顶点缓冲区的大小，单位为字节。
        bd.ByteWidth = UINT(sizeof(V) * vertices.size());
        // 存储在结构化缓冲中的一个元素的大小，以字节为单位。这个属性只用于结构化缓冲，其他缓冲可以设置为 0。
        // 所谓结构化缓冲，是指存储其中的元素大小都相等的缓冲。
        bd.StructureByteStride = sizeof(V);
        // 2．填写一个 D3D11_SUBRESOURCE_DATA 结构体，为缓冲区指定初始化数据。
        D3D11_SUBRESOURCE_DATA sd = {};
        // 包含初始化数据的系统内存数组的指针。当缓冲区可以存储 n 个顶点时，对应的初始化数组也应至少包含 n 个顶点，
        // 从而使整个缓冲区得到初始化。SysMemPitch 和 SysMemSlicePitch 成员用于纹理图像，
        // SysMemPitch 用于决定纹理的每行的开始位置，而 SysMemSlicePitch 用于决定每行的深度，它用于 3D 贴图。
        sd.pSysMem = vertices.data();
        // 3．调用 ID3D11Device::CreateBuffer 方法来创建缓冲区。
        GFX_THROW_INFO(GetDevice(gfx)->CreateBuffer(&bd, &sd, &pVertexBuffer));
    }
    void Bind(Graphics& gfx) noexcept override;
protected:
    UINT stride;
    Microsoft::WRL::ComPtr<ID3D11Buffer> pVertexBuffer;
};