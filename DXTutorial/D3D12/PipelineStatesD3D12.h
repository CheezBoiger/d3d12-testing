//
#pragma once

#include "../Renderer.h"
#include "D3D12Backend.h"

namespace gfx {


class GraphicsPipelineStateD3D12 : public GraphicsPipeline
{
public:
  GraphicsPipelineStateD3D12() { }
  D3D_PRIMITIVE_TOPOLOGY _topology;
};


class ComputePipelineStateD3D12 : public ComputePipeline
{
public:
  ComputePipelineStateD3D12() { }
};
} //