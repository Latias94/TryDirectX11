#include "Topology.h"

Topology::Topology( Graphics& gfx,D3D11_PRIMITIVE_TOPOLOGY type )
        :
        type( type )
{}

void Topology::Bind( Graphics& gfx ) noexcept
{
    // 设置图元类型，设定输入布局
    GetContext( gfx )->IASetPrimitiveTopology( type );
}