#include "VertexShader.h"
#include "GraphicsThrowMacros.h"


VertexShader::VertexShader(Graphics &gfx, const std::wstring &path) {
    INFOMAN(gfx);
    // 这里也用 & 符号，就是为了 release 前面顶点着色器所用的 Blob，
    // 如果不释放而改用 GetAddressOf 取地址来放资源，就会产生内存泄漏。
    GFX_THROW_INFO(D3DReadFileToBlob(path.c_str(), &pBytecodeBlob));
    GFX_THROW_INFO(GetDevice(gfx)->CreateVertexShader(
            pBytecodeBlob->GetBufferPointer(),
            pBytecodeBlob->GetBufferSize(),
            nullptr,
            &pVertexShader
    ));
}

void VertexShader::Bind(Graphics &gfx) noexcept {
    GetContext(gfx)->VSSetShader(pVertexShader.Get(), nullptr, 0u);
}

ID3DBlob *VertexShader::GetBytecode() const noexcept {
    return pBytecodeBlob.Get();
}