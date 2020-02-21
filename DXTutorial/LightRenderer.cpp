//
#include "LightRenderer.h"
#include "BackendRenderer.h"

namespace jcl {
namespace Lights {


gfx::Resource* lightOutput = nullptr;
gfx::RenderTargetView* lightOutputRTV = nullptr;

gfx::GraphicsPipeline* lightDeferredPipeline = nullptr;


void generateDeferredLightsCommands
    (
        gfx::CommandList* pList,
        LightSystem* pLightSystem
    )
{
}
} // Lights
} // jcl