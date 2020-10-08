#include "Box.h"
#include "BindableBase.h"
#include "GraphicsThrowMacros.h"

Box::Box(Graphics &gfx,
         std::mt19937 &rng,
         std::uniform_real_distribution<float> &adist,
         std::uniform_real_distribution<float> &ddist,
         std::uniform_real_distribution<float> &odist,
         std::uniform_real_distribution<float> &rdist)
        :
        r(rdist(rng)),
        droll(ddist(rng)),
        dpitch(ddist(rng)),
        dyaw(ddist(rng)),
        dphi(odist(rng)),
        dtheta(odist(rng)),
        dchi(odist(rng)),
        chi(adist(rng)),
        theta(adist(rng)),
        phi(adist(rng)) {

    // 不重复添加重复的资源
    if (!IsStaticInitialized()) {
        struct Vertex {
            struct {
                float x;
                float y;
                float z;
            } pos;
        };
        // 顺时针
        const std::vector<Vertex> vertices =
                {
                        {-1.0f, -1.0f, -1.0f},
                        {1.0f,  -1.0f, -1.0f},
                        {-1.0f, 1.0f,  -1.0f},
                        {1.0f,  1.0f,  -1.0f},
                        {-1.0f, -1.0f, 1.0f},
                        {1.0f,  -1.0f, 1.0f},
                        {-1.0f, 1.0f,  1.0f},
                        {1.0f,  1.0f,  1.0f},
                };
        AddStaticBind(std::make_unique<VertexBuffer>(gfx, vertices));

        auto pvs = std::make_unique<VertexShader>(gfx, L"VertexShader.cso");
        auto pvsbc = pvs->GetBytecode();
        AddStaticBind(std::move(pvs));

        AddStaticBind(std::make_unique<PixelShader>(gfx, L"PixelShader.cso"));

        // create index buffer 索引默认情况下为 16 位
        const std::vector<unsigned short> indices =
                {
                        0, 2, 1, 2, 3, 1,
                        1, 3, 5, 3, 7, 5,
                        2, 6, 3, 3, 6, 7,
                        4, 5, 7, 4, 7, 6,
                        0, 4, 2, 2, 4, 6,
                        0, 1, 4, 1, 5, 4
                };
        AddStaticIndexBuffer(std::make_unique<IndexBuffer>(gfx, indices));

        // lookup table for cube face colors
        // 给正方形每一个面一个颜色，因为顶点是共用的，所以顶点颜色会插值，我们可以给每个面都配一个单独的顶点和颜色，
        // 但这样顶点数量就会增加到 24 个。因此这里的做法是给每一个三角形一个索引，然后在 lookup table 里面找。
        struct ConstantBuffer2 {
            struct {
                float r;
                float g;
                float b;
                float a;
            } face_colors[6];
        };
        const ConstantBuffer2 cb2 =
                {
                        {
                                {1.0f, 0.0f, 1.0f}, // 第一个面的两个三角形颜色
                                {1.0f, 0.0f, 0.0f}, // 第二个面的 ...
                                {0.0f, 1.0f, 0.0f},
                                {0.0f, 0.0f, 1.0f},
                                {1.0f, 1.0f, 0.0f},
                                {0.0f, 1.0f, 1.0f},
                        }
                };
        AddStaticBind(std::make_unique<PixelConstantBuffer<ConstantBuffer2>>(gfx, cb2));

        const std::vector<D3D11_INPUT_ELEMENT_DESC> ied =
                {
                        // SemanticName: 一个与元素相关联的特定字符串，我们称之为语义（semantic），它传达了元素的预期用途。该参数可以是任意合法的语义名。
                        // 通过语义即可将顶点结构体（图 6.1 中的 struct Vertex）中的元素与顶点着色器输入签名＂ （vertex shader input signature，
                        // 即图 6.1 中的 VertexOut vs 的众参数）中的元素 -- 映射起来，如图 6.1 所示。
                        // SemanticIndex: 附加到语义上的索引。此成员的设计动机可从图 6.1 中看出。例如，顶点结构体中的纹理坐标可能不止一组，
                        // 而仪在语义名尾部添加一个素引，即可在不引入新语义名的情况下区分出这两组不同的纹理坐标。
                        // 在着色器代码中，未标明素引的语义将默认其索引值为 0，也就是说，图 6.1 中的 POSITION 与 POSITIONO 等价。
                        // Format: 在 Direct3D 中，要通过枚举类型 DXG1 FORMAT 中的成员来指定顶点元素的格式（即数据类型）
                        // Inputslot: 指定传递元素所用的输入槽（input slot index）索引。 Direct3D 共支持 16 个输入槽（索引值为 0-15），
                        // 可以通过它们来向输入装配阶段传递顶点数据。目前我们只会用到输入槽 0（即所有的顶点元素都来自同一个输入槽），
                        // 但在木章的习题 2 中将会涉及多输入槽的编程实践。
                        // AlignedByteOffset: 在特定输入槽中，从 C++ 顶点结构体的首地址到其中某点元素起始地
                        // 址的偏移量（用字节表示）。例如，在下列顶点结构体中，元素 Pos 的偏移量为 0 字节，因为它
                        // 的起始地址与顶点结构体的首地址一致；元素 Normal 的偏移量为 12 字节，因为跳过 Pos 所
                        // 占用的字节数才能找到 Normal 的起始地址
                        // InputSlotClass；我们暂且把此参数指定为 D3D11_INPUT_PER_VERTEX_DATA，
                        // 而其他选项则用于实现实例化（instancing）这种高级技术。
                        // InstanceDataStepRate： 目前指定为 0；其他值只用于高级实例技术。
                        {"Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
                };
        // 在定义了顶点结构体之后，我们必须设法描述该顶点结构体的分量结构，使 Direct3D 知道该如何使用每个分量，如何读取顶点数据。
        // 这一描述信息是以输入布局（ID3D11InputLayout）的形式提供给 Direct3D 的 。
        AddStaticBind(std::make_unique<InputLayout>(gfx, ied, pvsbc));

        AddStaticBind(std::make_unique<Topology>(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
    } else {
        SetIndexFromStatic();
    }

    // 单独绑定是因为每个 Cube 的变换方式都不一样
    AddBind(std::make_unique<TransformCbuf>(gfx, *this));
}

void Box::Update(float dt) noexcept {
    roll += droll * dt;
    pitch += dpitch * dt;
    yaw += dyaw * dt;
    theta += dtheta * dt;
    phi += dphi * dt;
    chi += dchi * dt;
}

DirectX::XMMATRIX Box::GetTransformXM() const noexcept {
    return DirectX::XMMatrixRotationRollPitchYaw(pitch, yaw, roll) *
           DirectX::XMMatrixTranslation(r, 0.0f, 0.0f) *
           DirectX::XMMatrixRotationRollPitchYaw(theta, phi, chi) *
           // 离摄像机远一点
           DirectX::XMMatrixTranslation(0.0f, 0.0f, 20.0f);
}
