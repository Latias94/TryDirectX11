#include "IndexBuffer.h"
#include "GraphicsThrowMacros.h"

// create index buffer 索引默认情况下为 16 位
IndexBuffer::IndexBuffer(Graphics &gfx, const std::vector<unsigned short> &indices)
        :
        count((UINT) indices.size()) {
    INFOMAN(gfx);
    // D3D11_BUFFER_DESC 参数介绍看上面
    D3D11_BUFFER_DESC ibd = {};
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0u;
    ibd.MiscFlags = 0u;
    ibd.ByteWidth = UINT(count * sizeof(unsigned short));
    ibd.Usage = D3D11_USAGE_DEFAULT;
    ibd.StructureByteStride = sizeof(unsigned short);
    D3D11_SUBRESOURCE_DATA isd = {};
    isd.pSysMem = indices.data();
    GFX_THROW_INFO(GetDevice(gfx)->CreateBuffer(&ibd, &isd, &pIndexBuffer));
}

void IndexBuffer::Bind(Graphics &gfx) noexcept {
    // 第 2 个参数表示索引格式。在本例中，我们使用的是 32 位无符号整数（DWORD）；所以，该参数设为 DXGI_FORMAT_R32_UINT。
    // 如果你希望节约一些内存，不需要这大的取值范围，那么可以改用 16 位无符号整数。还要注意的是，
    // 在 IASetIndexBuffer 方法中指定的格式必须与 D3D11_BUFFER_DESC::ByteWidth 数据成员指定的字节长度一致，
    // 否则会出现问题。索引缓冲区只支持 DXGI_FORMAT_R16_UINT 和 DXGI_FORMAT_R32_UINT 两种格式。
    // 第 3 个参数是一个偏移值，它表示从索引缓冲区的起始位置开始、到输入装配时实际读取数据的位置之间的字节长度。
    // 如果希望跳过索引缓冲区前面的一部分数据，那么可以使用该参数。
    GetContext(gfx)->IASetIndexBuffer(pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0u);
}

UINT IndexBuffer::GetCount() const noexcept {
    return count;
}