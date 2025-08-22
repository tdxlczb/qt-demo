#include "graphics_vertices.h"

std::vector<VertexAL> CreateCylinderVertices(int sectorCount, float pierRadius, float pierHeight)
{
    const float PI = 3.14159265359f;
    float sectorStep = 2 * PI / sectorCount;//每个扇区的弧度
    float texcoodStep = 1.0f / sectorCount;//每个扇区的纹理大小

    //圆柱模型显示为：侧面环绕y轴
    //生成圆柱的顶点
    std::vector<VertexAL> unitVertices;
    for (size_t i = 0; i < sectorCount; i++)
    {
        float sectorAngle = i * sectorStep;
        float x = cos(sectorAngle) * pierRadius;
        float z = sin(sectorAngle) * pierRadius;
        //顶部顶点(y = height/2)
        VertexAL topVertex;
        topVertex.position = glm::vec3(x, pierHeight / 2, z);
        topVertex.normal = glm::vec3(x, 0.0f, z);
        topVertex.normal = glm::normalize(topVertex.normal);
        topVertex.texcoord = glm::vec2(i * texcoodStep, 1.0f);
        unitVertices.push_back(topVertex);

        //底部顶点(y = -height/2)
        VertexAL bottomVertex;
        bottomVertex.position = glm::vec3(x, -pierHeight / 2, z);
        bottomVertex.normal = glm::vec3(x, 0.0f, z);
        bottomVertex.normal = glm::normalize(bottomVertex.normal);
        bottomVertex.texcoord = glm::vec2(i * texcoodStep, 0.0f);
        unitVertices.push_back(bottomVertex);
    }

    std::vector<VertexAL> drawVertices;
    //三角形顶点绘制顺序为：012，123，234，345...
    //生成三角形绘制顶点
    for (size_t i = 0; i < unitVertices.size(); i++)
    {
        VertexAL vertex1;
        VertexAL vertex2;
        VertexAL vertex3;
        if (i == unitVertices.size() - 2) {
            vertex1 = unitVertices[i];
            vertex2 = unitVertices[i + 1];
            vertex3 = unitVertices[0];
        }
        else if (i == unitVertices.size() - 1) {
            vertex1 = unitVertices[i];
            vertex2 = unitVertices[0];
            vertex3 = unitVertices[1];
        }
        else {
            vertex1 = unitVertices[i];
            vertex2 = unitVertices[i + 1];
            vertex3 = unitVertices[i + 2];
        }
        drawVertices.push_back(vertex1);
        drawVertices.push_back(vertex2);
        drawVertices.push_back(vertex3);
    }
    return drawVertices;
}
