#ifndef GRAPHICS_VERTICES_H
#define GRAPHICS_VERTICES_H

#include <vector>
#include <glm/glm.hpp>

enum class CustomGraphics
{
    None = 0,
    Cylinder
};

struct VertexAL {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texcoord;
};

///
/// \brief 生成圆柱体顶点数组
/// \param sectorCount 扇区数
/// \param pierRadius  半径
/// \param pierHeight  高度
/// \return 是否装载成功
///
std::vector<VertexAL> CreateCylinderVertices(int sectorCount, float pierRadius, float pierHeight);


#endif // GRAPHICS_VERTICES_H