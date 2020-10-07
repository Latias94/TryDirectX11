#include "TransformCbuf.h"

TransformCbuf::TransformCbuf(Graphics &gfx, const Drawable &parent)
        :
        pVcbuf(gfx),
        parent(parent) {
}

void TransformCbuf::Bind(Graphics &gfx) noexcept {
    // 由于 CPU 中矩阵通常是行主序的，但 HLSL 中默认是列主序的，如果不想在 shader 里面转置，就要在传数据前转置一下。
    pVcbuf.Update(gfx,
                   DirectX::XMMatrixTranspose(
                           parent.GetTransformXM() * gfx.GetProjection()
                   )
    );
    pVcbuf.Bind(gfx);
}