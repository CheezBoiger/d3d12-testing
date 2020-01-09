
#include "imgui.h"
#include "DebugGUI.h"

namespace jcl {


struct FrameResource {
    gfx::Resource* vertexBuffer;
    gfx::Resource* indexBuffer;
    I32 vertexBufferSz;
    I32 indexBufferSz;
};


FrameResource* g_frameResources = nullptr;
gfx::GraphicsPipeline* g_graphicsPipeline = nullptr;
gfx::RootSignature* g_rootSignature = nullptr;
gfx::DescriptorTable* g_descriptorTable = nullptr;

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
}


void populateCommandListGUI(gfx::BackendRenderer* pRenderer, gfx::CommandList* pCmdList)
{
    //ImGui::NewFrame();
    
    //ImGui::EndFrame();
}
} // jcl