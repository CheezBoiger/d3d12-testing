// Definitions that are used by both shaders and c++ program
#ifndef _COMMON_SHADER_PARAMS_H_
#define _COMMON_SHADER_PARAMS_H_

#define MATERIAL_USE_ALBEDO_MAP (1 << 0)
#define MATERIAL_USE_NORMAL_MAP (1 << 1)
#define MATERIAL_USE_METALLIC (1 << 2)
#define MATERIAL_USE_ROUGHNESS (1 << 3)
#define MATERIAL_USE_SPECULAR_GLOSSINESS (1<<4)

#define DIELECTRIC_SPECULAR_VALUE 0.04

#endif // _COMMON_SHADER_PARAMS_H_
