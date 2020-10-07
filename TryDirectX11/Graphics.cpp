#include "Graphics.h"
#include "dxerr.h"
#include <sstream>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include "GraphicsThrowMacros.h"

namespace wrl = Microsoft::WRL;
namespace dx = DirectX;

// 自动设置连接器
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "D3DCompiler.lib")

Graphics::Graphics(HWND hWnd) {
    DXGI_SWAP_CHAIN_DESC sd = {};
    // 1. BufferDesc：这个结构体描述了待创建后台缓冲区的属性。在这里我们仅关注它的宽度、高
    // 度和像素格式属性。至于其他成员的细节可查看 SDK 文档。
    // 设置宽高为 0，就是让 D3D 自己看着办
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 0;
    sd.BufferDesc.RefreshRate.Denominator = 0;
    sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    // 2. SampleDesc：多重采样的质量级别以及对每个像素的采样次数，可参见 4.1.8 节。
    // 对于单次采样来说，我们要将采样数量指定为 1，质量级别指定为 0。
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    // 3. BufferUsage：由于我们要将数据渲染至后台缓冲区（即用它作为渲染目标），因此将此参数
    // 指定为 DXGI_USAGE_RENDER_TARGET_OUTPUT。
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    // 4. BufferCount：交换链中所用的缓冲区数量。如果将它指定为 1，即采用双缓冲。
    sd.BufferCount = 1;
    // 5. OutputWindow：渲染窗口的句柄。
    sd.OutputWindow = hWnd;
    // 6. Windowed：若指定为 true，程序将在窗口模式下运行；如果指定为 false，则采用全屏模式。
    sd.Windowed = TRUE;
    // 7. SwapEffect：如果使用双缓冲，指定为 DXGI_SWAP_EFFECT_FLIP_DISCARD。
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    // 8. Flags：可选标志。如果将其指定为 DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH，那
    // 么，当程序切换为全屏模式时，它将选择最适于当前应用程序窗口尺寸的显示模式。如果没有
    // 指定该标志，当程序切换为全屏模式时，将采用当前桌面的显示模式。
    sd.Flags = 0;

    UINT swapCreateFlags = 0u;
#ifndef NDEBUG
    swapCreateFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    // for checking results of d3d functions
    HRESULT hr;

    // create device and front/back buffers, and swap chain and rendering context
    GFX_THROW_INFO(D3D11CreateDeviceAndSwapChain(
            // 1.一个用于创建设备的视频适配器(译者注：显卡)的指针。如果传入 NULL，则 D3D 使用默认显卡。如果在
            // 机器上装有多个显卡，就可启用该参数。
            nullptr,
            // 2.我们希望创建的驱动设备类型(例如：硬件设备，WARP 设备，软件设备，或参考设备)
            D3D_DRIVER_TYPE_HARDWARE,
            // 3.实现软件渲染设备的动态库句柄。如果使用的驱动设备类型是软件设备，则该参数不能为 NULL。
            nullptr,
            // 4.创建标志。D3D 中的创建标志 0 用于我们的游戏发布，而标志 D3D11_CREATE_DEVICE_DEBUG 则允许我
            // 们创建可供调试的设备，在开发中这是比较有用的。
            swapCreateFlags,
            // 5.我们所希望创建的特征级别，以我们的希望值进行特征的安排。此书中我们只关心 D3D 11 或 Direct3D 10 的创建，
            // 当然我们也能够通过 D3D_FEATURE_LEVEL_9_3, D3D_FEATURE_LEVEL_9_2, D3D_FEATURE_LEVEL_9_1 来创建 Direct3D 9 的目标。
            nullptr,
            // 6.特征级别数组中的特征数量。
            0,
            // 7.SDK 版本号。因为我们只使用 DirectX 11 SDK，所以书中都是 D3D11_SDK_VERSION。
            D3D11_SDK_VERSION,
            // 8.交换链描述对象。
            &sd,
            // 9.设备对象地址(ID3D11-Device 类型)。
            &pSwap,
            &pDevice,
            // 10.所选择的特征级别地址，存储特征级别数组中，被创建成功的特征级别。
            nullptr,
            // 11.渲染环境地址(ID3D11Context 类型)。
            &pContext
    ));

    // gain access to texture subresource in swap chain (back buffer)
    wrl::ComPtr<ID3D11Resource> pBackBuffer;
    // 1. Buffer：希望获得的特定后台缓冲区的索引（有时后台缓冲区并不只一个，所以需要用索引来指明）。
    // 2. riid：希望获得的 ID3D11Resource 接口的 COM ID。
    // 3. ppSurface：返回一个指向 ID3D12Resource 接口的指针，这便是希望获得的后台缓冲区。
    // 调用 IDXGISwapChain::GetBuffer 方法会增加相关后台缓冲区的 COM 引用计数，所以在每次
    // 使用后一定要将其释放。通过 ComPtr 便可以自动做到这一点。
    GFX_THROW_INFO(pSwap->GetBuffer(0, __uuidof(ID3D11Resource), &pBackBuffer));
    // 接下来，使用 ID3D11Device::CreateRenderTargetView 方法来为获取的后台缓冲区创建渲染目标视图。
    // 1. pResource：指定用作渲染目标的资源。在上面的例子中是后台缓冲区（即为后台缓冲区创建了一个渲染目标视图）。
    // 2. pDesc：指向 D3D11_RENDER_TARGET_VIEW_DESC 数据结构实例的指针。该结构体描述了
    // 资源中元素的数据类型（格式）。如果该资源在创建时已指定了具体格式（即此资源不是无类型
    // 格式，not typeless），那么就可以把这个参数设为空指针，表示采用该资源创建时的格式，为它
    // 的第一个 mipmap 层级（后台缓冲区只有一种 mipmap 层级，有关 mipmap 的内容将在第 9 章展
    // 开讨论）创建一个视图。由于已经指定了后台缓冲区的格式，因此就将这个参数设置为空指针。
    // 3. DestDescriptor：引用所创建渲染目标视图的描述符句柄。
    GFX_THROW_INFO(pDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, &pTarget));

    D3D11_DEPTH_STENCIL_DESC dsDesc = {};
    // 1．DepthEnable：设置为 true，则开启深度缓冲；设置为 false，则禁用。当深度测试被禁止时，物体的绘制顺序就变得极为重要，
    // 否则位于遮挡物之后的像素片段也将被绘制出来（回顾 4.1.5 节）。如果深度缓冲被禁用，则深度缓冲区中的元素便不会被更新，
    // DepthWriteMask 项的设置也不会起作用。
    dsDesc.DepthEnable = TRUE;
    // 2．DepthWriteMask：可将此参数设置为 D3D11_DEPTH_WRITE_MASK_ZERO 或者 D3D11_DEPTH_WRITE_MASK_ALL，但两者不能共存。
    // 假设 DepthEnable 为 true，若把此参数设置为 D3D11_DEPTH_WRITE_MASK_ZERO 便会禁止对深度缓冲区的写操作，但仍可执行深度测试；
    // 若将该项设为 D3D11_DEPTH_WRITE_MASK_ALL，则通过深度测试与模板测试的深度数据将被写入深度缓冲区。这种控制深度数据读写的能力，为某些特效的实现提供了良好的契机。
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    // 3．DepthFunc：将该参数指定为枚举类型 D3D11_COMPARISON_FUNC 的成员之一，以此来定义深度测试所用的比较函数。此项一般被设为 D3D11_COMPARISON_FUNC_LESS，
    // 因而常常执行深度测试。即，若给定像素片段的深度值小于位于深度缓冲区中对应像素的深度值，则接受该像素片段（离摄像机近的物体遮挡距摄像机远的物体）。
    // 当然，也正如我们所看到的，Direct3D 也允许用户根据需求来自定义深度测试。
    dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
    wrl::ComPtr<ID3D11DepthStencilState> pDSState;
    GFX_THROW_INFO(pDevice->CreateDepthStencilState(&dsDesc, &pDSState));
    // 2．StencilRef：模板测试使用的 32 位模板参考值。 深度测试不用管
    pContext->OMSetDepthStencilState(pDSState.Get(), 1u);

    // 深度缓冲区只是一个存储深度信 息的 2D 纹理（如果使用模板，则模板信息也在该缓冲区中）。要创建纹理，我们必须填充
    // 一个 D3D11_TEXTURE2D_DESC 结构体来描述所要创建的纹理，
    wrl::ComPtr<ID3D11Texture2D> pDepthStencil;
    D3D11_TEXTURE2D_DESC descDepth = {};
    // 宽高和交换链一样
    descDepth.Width = 800u;
    descDepth.Height = 600u;
    // 3．MipLevels：多级渐近纹理层（mipmap level）的数量。多级渐近纹理将在后面的章节“纹理”中进行讲解。对于深度/模板缓冲区来说，我们的纹理只需要一个多级渐近纹理层。
    descDepth.MipLevels = 1u;
    // 4．ArraySize：在纹理数组中的纹理数量。对于深度/模板缓冲区来说，我们只需要一个纹理。
    descDepth.ArraySize = 1u;
    // 5．Format：一个 DXGI_FORMAT 枚举类型成员，它指定了纹理元素的格式。
    descDepth.Format = DXGI_FORMAT_D32_FLOAT;
    // 6. SampleDesc：多重采样数量和质量级别，不用就设为 1 和 0
    descDepth.SampleDesc.Count = 1u;
    descDepth.SampleDesc.Quality = 0u;
    // 7．Usage：表示纹理用途的 D3D11_USAGE 枚举类型成员。有 4 个可选值：
    // D3D11_USAGE_DEFAULT：表示 GPU（graphics processing unit，图形处理器）会对资源执行读写操作。CPU 不能读写这种资源。
    //    对于深度 / 模板缓冲区，我们使用 D3D11_USAGE_DEFAULT 标志值，因为 GPU 会执行所有读写深度 / 模板缓冲区的操作。
    // D3D10_USAGE_IMMUTABLE：表示在创建资源后，资源中的内容不会改变。这样可以获得一些内部优化，因为 GPU 会以只读方式访问这种资源。
    //    除了在创建资源时 CPU 会写入初始化数据外，其他任何时候 CPU 都不会对这种资源执行任何读写操作。
    // D3D10_USAGE_DYNAMIC：表示应用程序（CPU）会频繁更新资源中的数据内容（例 如，每帧更新一次）。GPU 可以从这种资源中读取数据，而 CPU 可以向这种资源中写 入数据。
    // D3D10_USAGE_STAGING：表示应用程序（CPU）会读取该资源的一个副本（即，该资源支持从显存到系统内存的数据复制操作）。
    descDepth.Usage = D3D11_USAGE_DEFAULT;
    // 8．BindFlags：指定该资源将会绑定到管线的哪个阶段。
    // 对于深度 / 模板缓冲区，该参数应设为 D3D11_BIND_DEPTH_STENCIL。其他可用于纹理的绑定标志值还有：
    // D3D11_BIND_RENDER_TARGET：将纹理作为一个渲染目标绑定到管线上。
    // D3D11_BIND_SHADER_RESOURCE：将纹理作为一个着色器资源绑定到管线上。
    descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    // CreateTexture2D 第二个参数是一个指向初始化数据的指针，这些初始化数据用来填充纹理。不过，由于个纹理被用作深度/模板缓冲区，
    // 所以我们不需要为它填充任何初始化数据。当执行深度缓存和模板操作时，Direct3D会自动向深度/模板缓冲区写入数据。所以，
    // 我们在这里将第二个参数指定为空值。
    GFX_THROW_INFO(pDevice->CreateTexture2D(&descDepth, nullptr, &pDepthStencil));

    // create view of depth stencil texture
    // 这个结构体描述了资源中这个元素数据类型（格式）。如果资源是一个有类型的格式（非 typeless），这个参数可以为空值，
    // 表示创建一个资源的第一个 mipmap 等级的视图（深度/模板缓冲也只能使用一个 mipmap 等级）。
    // 因为我们指定了深度/模板缓冲的格式，所以可以将这个参数设置为空值。
    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
    descDSV.Format = DXGI_FORMAT_D32_FLOAT;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0u;
    GFX_THROW_INFO(pDevice->CreateDepthStencilView(
            pDepthStencil.Get(), &descDSV, &pDSV
    ));

    // bind depth stencil view to OM
    pContext->OMSetRenderTargets(1u, pTarget.GetAddressOf(), pDSV.Get());

    // 我们通常会将 3D 场景绘制到与整个屏幕（在全屏模式下）或整个窗口工作区大小相当的后台缓冲区中。
    // 但是，有时只是希望把 3D 场景绘制到后台缓冲区的某个矩形子区域当中
    // 比如指定渲染目标的一个子区域，就可以创建多个 HUB
    D3D11_VIEWPORT vp;
    vp.Width = 800;
    vp.Height = 600;
    vp.MinDepth = 0;
    vp.MaxDepth = 1;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    pContext->RSSetViewports(1u, &vp);
}

void Graphics::EndFrame() {
    HRESULT hr;
#ifndef NDEBUG
    infoManager.Set();
#endif
    // 该函数的参数是同步区间和显示标识。如果同步区
    // 间是 0 表示立即绘制，1,2,3,4 表示在第 n 个垂直消隐(vertical blanking)之后绘制。
    if (FAILED(hr = pSwap->Present(1u, 0u))) {
        if (hr == DXGI_ERROR_DEVICE_REMOVED) {
            // 通常由于驱动程序崩溃 超频GPU然后出事
            throw GFX_DEVICE_REMOVED_EXCEPT(pDevice->GetDeviceRemovedReason());
        } else {
            throw GFX_EXCEPT(hr);
        }
    }
}

void Graphics::ClearBuffer(float red, float green, float blue) noexcept {
    const float color[] = {red, green, blue, 1.0f};
    // 清屏
    pContext->ClearRenderTargetView(pTarget.Get(), color);
    pContext->ClearDepthStencilView(pDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0u);
}

void Graphics::DrawIndexed(UINT count) noexcept(!IS_DEBUG) {
    GFX_THROW_INFO_ONLY(pContext->DrawIndexed(count, 0u, 0u));

    namespace wrl = Microsoft::WRL;
}


void Graphics::SetProjection(DirectX::FXMMATRIX
proj ) noexcept{
projection = proj;
}

DirectX::XMMATRIX Graphics::GetProjection() const noexcept {
    return projection;
}

// Graphics exception stuff
Graphics::HrException::HrException(int line, const char *file, HRESULT hr, std::vector<std::string> infoMsgs) noexcept
        :
        Exception(line, file),
        hr(hr) {
    // join all info messages with newlines into single string
    for (const auto &m : infoMsgs) {
        info += m;
        info.push_back('\n');
    }
    // remove final newline if exists
    if (!info.empty()) {
        info.pop_back();
    }
}

const char *Graphics::HrException::what() const noexcept {
    std::ostringstream oss;
    oss << GetType() << std::endl
        << "[Error Code] 0x" << std::hex << std::uppercase << GetErrorCode()
        << std::dec << " (" << (unsigned long) GetErrorCode() << ")" << std::endl
        << "[Error String] " << GetErrorString() << std::endl
        << "[Description] " << GetErrorDescription() << std::endl;
    if (!info.empty()) {
        oss << "\n[Error Info]\n" << GetErrorInfo() << std::endl << std::endl;
    }
    oss << GetOriginString();
    whatBuffer = oss.str();
    return whatBuffer.c_str();
}

const char *Graphics::HrException::GetType() const noexcept {
    return "Chili Graphics Exception";
}

HRESULT Graphics::HrException::GetErrorCode() const noexcept {
    return hr;
}

std::string Graphics::HrException::GetErrorString() const noexcept {
    return reinterpret_cast<const char *const>(DXGetErrorString(hr));
}

std::string Graphics::HrException::GetErrorDescription() const noexcept {
    char buf[512];
    DXGetErrorDescriptionA(hr, buf, sizeof(buf));
    return buf;
}

std::string Graphics::HrException::GetErrorInfo() const noexcept {
    return info;
}

const char *Graphics::DeviceRemovedException::GetType() const noexcept {
    return "Chili Graphics Exception [Device Removed] (DXGI_ERROR_DEVICE_REMOVED)";
}

Graphics::InfoException::InfoException(int line, const char *file, std::vector<std::string> infoMsgs) noexcept
        :
        Exception(line, file) {
    // join all info messages with newlines into single string
    for (const auto &m : infoMsgs) {
        info += m;
        info.push_back('\n');
    }
    // remove final newline if exists
    if (!info.empty()) {
        info.pop_back();
    }
}

const char *Graphics::InfoException::what() const noexcept {
    std::ostringstream oss;
    oss << GetType() << std::endl
        << "\n[Error Info]\n" << GetErrorInfo() << std::endl << std::endl;
    oss << GetOriginString();
    whatBuffer = oss.str();
    return whatBuffer.c_str();
}

const char *Graphics::InfoException::GetType() const noexcept {
    return "Chili Graphics Info Exception";
}

std::string Graphics::InfoException::GetErrorInfo() const noexcept {
    return info;
}