#include "PixelShader.h"
#include "GraphicsThrowMacros.h"

PixelShader::PixelShader( Graphics& gfx,const std::wstring& path )
{
    INFOMAN( gfx );

    Microsoft::WRL::ComPtr<ID3DBlob> pBlob;
    // 这里也用 & 符号，就是为了 release 前面顶点着色器所用的 Blob，
    // 如果不释放而改用 GetAddressOf 取地址来放资源，就会产生内存泄漏。
    GFX_THROW_INFO( D3DReadFileToBlob( path.c_str(),&pBlob ) );
    GFX_THROW_INFO( GetDevice( gfx )->CreatePixelShader( pBlob->GetBufferPointer(),pBlob->GetBufferSize(),nullptr,&pPixelShader ) );
}

void PixelShader::Bind( Graphics& gfx ) noexcept
{
    GetContext( gfx )->PSSetShader( pPixelShader.Get(),nullptr,0u );
}