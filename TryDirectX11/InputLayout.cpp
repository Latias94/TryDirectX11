#include "InputLayout.h"
#include "GraphicsThrowMacros.h"

InputLayout::InputLayout( Graphics& gfx,
                          const std::vector<D3D11_INPUT_ELEMENT_DESC>& layout,
                          ID3DBlob* pVertexShaderBytecode )
{
    INFOMAN( gfx );

    // 创建并绑定顶点布局
    GFX_THROW_INFO( GetDevice( gfx )->CreateInputLayout(
            layout.data(),(UINT)layout.size(),
            pVertexShaderBytecode->GetBufferPointer(), // 这里 pBlob 存的是顶点着色器的二进制
            pVertexShaderBytecode->GetBufferSize(),
            &pInputLayout
    ) );
}

void InputLayout::Bind( Graphics& gfx ) noexcept
{
    GetContext( gfx )->IASetInputLayout( pInputLayout.Get() );
}