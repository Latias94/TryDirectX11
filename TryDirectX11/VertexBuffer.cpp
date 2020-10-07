#include "VertexBuffer.h"

void VertexBuffer::Bind(Graphics &gfx) noexcept {
    const UINT offset = 0u;
    // 在创建顶点缓冲区后，我们必须把它绑定到设备的输入槽上，只有这样才能将顶点送入管线。
    // 1．StartSlot：顶点缓冲区所要绑定的起始输入槽。一共有 16 个输入槽，索引依次为 0 到 15。
    // 2．NumBuffers：顶点缓冲区所要绑定的输入槽的数量，如果起始输入槽为索引 k，我
    //  们绑定了 n 个缓冲，那么缓冲将绑定在索引为 Ik，Ik+1……Ik+n-1的输入槽上。
    // 3．ppVertexBuffers：指向顶点缓冲区数组的第一个元素的指针。
    // 4．pStrides：指向步长数组的第一个元素的指针（该数组的每个元素对应一个顶点缓冲区，
    // 也就是，第 i 个步长对应于第 i 个顶点缓冲区）。这个步长是指顶点缓冲区中的元素的字节长度。
    // 5．pOffsets：指向偏移数组的第一个元素的指针（该数组的每个元素对应一个顶点缓冲区，
    // 也就是，第 i 个偏移量对应于第 i 个顶点缓冲区）。这个偏移量是指从顶点缓冲区的起始位置开始，
    // 到输入装配阶段将要开始读取数据的位置之间的字节长度。当希望跳过顶点缓冲区前面的一部分数据时，可以使用该参数。
    GetContext(gfx)->IASetVertexBuffers(
            0u,
            1u,
            pVertexBuffer.GetAddressOf(),
            &stride,
            &offset);
}