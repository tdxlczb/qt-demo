#ifndef GRAPHICS_VERTICES_H
#define GRAPHICS_VERTICES_H

#include <vector>
#include <glm/glm.hpp>

enum class CustomGraphics
{
    None = 0, //默认为正方体
    Cube,     //正方体
    Cylinder, //圆柱
    Sphere,   //球体  
};

struct VertexAL {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texcoord;
};

///
/// \brief 生成圆柱体顶点数据，上为顶部，下为底部
/// \param pierRadius  半径
/// \param sectorCount 扇区数
/// \param pierHeight  高度
/// \return 返回顶点数据
///
std::vector<VertexAL> CreateCylinderVertices(float pierRadius, int sectorCount, float pierHeight);

///
/// \brief 生成球体顶点数据，按照经纬线划分
/// \param radius  半径
/// \param xSegNum   x轴片段数量，对应经度longitude，范围为-180度~180度
/// \param ySegNum   y轴片段数量，对应纬度latitude，范围为-90度~90度
/// \param vertices  返回顶点数组
/// \param indices   返回索引数组
///
void CreateSphereVertices(float radius, int xSegNum, int ySegNum, std::vector<VertexAL>& vertices, std::vector<unsigned int>& indices);

#endif // GRAPHICS_VERTICES_H