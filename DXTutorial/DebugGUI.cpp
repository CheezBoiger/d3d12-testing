
#include "imgui.h"
#include "DebugGUI.h"

#include <vector>

namespace jcl {


struct FrameResource {
    gfx::Resource* vertexBuffer;
    gfx::Resource* indexBuffer;
    I32 vertexBufferSz;
    I32 indexBufferSz;
};


std::vector<FrameResource> g_frameResources;
gfx::GraphicsPipeline* g_graphicsPipeline = nullptr;
gfx::RootSignature* g_rootSignature = nullptr;
gfx::DescriptorTable* g_descriptorTable = nullptr;


const char* c_vertexShader = R"(
    
    struct SysInfo {
        
    };

    float4 VSMain(float3 Position : POSITION, float2 TexCoord : TEXCOORD) {
        float4 val = float4(0, 0, 0, 0);
        
        return val;
    }
)";


const char* c_pixelShader = R"(
    Texture2D<float4> UITexture : register( t0 );
    sampler UISampler : register( s0 );
    float4 PSMain( float4 texcoord : TEXCOORD ) : SV_TARGET {
        float4 color = UITexture.Sample( UISampler, texcoord );
        return color;
    }
)";


void createResources(gfx::BackendRenderer* pRenderer)
{
    
}


void initDebugGUI(gfx::BackendRenderer* pRenderer, HWND handle)
{
    IMGUI_CHECKVERSION( );
    ImGui::CreateContext( );
    ImGuiIO& io = ImGui::GetIO( );
    io.BackendRendererName = "BackendRenderer(TM)";
    io.BackendPlatformName = "Win64";
    io.ImeWindowHandle = handle;
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset | ImGuiBackendFlags_HasMouseCursors;
    
    io.KeyMap[ImGuiKey_Space] = VK_SPACE;
    io.KeyMap[ImGuiKey_Enter] = VK_RETURN;
    io.KeyMap[ImGuiKey_Backspace] = VK_BACK;
    io.KeyMap[ImGuiKey_LeftArrow] = VK_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = VK_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = VK_UP;
    io.KeyMap[ImGuiKey_DownArrow] = VK_DOWN;
    io.KeyMap[ImGuiKey_Delete] = VK_DELETE;
    io.KeyMap[ImGuiKey_Home] = VK_HOME;
    io.KeyMap[ImGuiKey_Insert] = VK_INSERT;
    io.KeyMap[ImGuiKey_End] = VK_END;

    ImGui::StyleColorsDark( );
    
    createResources(pRenderer);
}


void populateCommandListGUI(gfx::BackendRenderer* pRenderer, gfx::CommandList* pCmdList)
{
    //ImGui::NewFrame();
    
    //ImGui::EndFrame();
}


void cleanUpDebugGUI( gfx::BackendRenderer* pRenderer )
{
    ImGui::DestroyContext( );
}


void updateMouseCursor( R32 x, R32 y )
{
    ImGuiIO& io = ImGui::GetIO();
    io.MousePos.x = x;
    io.MousePos.y = y;
}
} // jcl