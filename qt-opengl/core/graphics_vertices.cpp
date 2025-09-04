#include "graphics_vertices.h"

namespace {
    const float PI = 3.14159265359f;
}
//float vertices[] = {
//     0.5f,  0.5f,  0.0f,  0.0f, 1.0f,  //0
//     0.5f, -0.5f,  0.0f,  0.0f, 0.0f,  //1
//     0.0f,  0.5f, -0.5f,  0.25f, 1.0f, //2
//     0.0f, -0.5f, -0.5f,  0.25f, 0.0f, //3
//    -0.5f,  0.5f,  0.0f,  0.50f, 1.0f, //4
//    -0.5f, -0.5f,  0.0f,  0.50f, 0.0f, //5
//     0.0f,  0.5f,  0.5f,  0.75f, 1.0f, //6
//     0.0f, -0.5f,  0.5f,  0.75f, 0.0f, //7
//};

std::vector<VertexAL> CreateCylinderVertices(float pierRadius, int sectorCount, float pierHeight)
{
    float sectorStep = 2 * PI / sectorCount;//每个扇区的弧度
    float texcoodStep = 1.0f / sectorCount;//每个扇区的纹理大小

    //圆柱模型显示为：侧面环绕y轴
    //生成圆柱的顶点：纹理从左到右，顶点就得按照逆时针顺序生成顶点
    //这里可以调整为i <= sectorCount，让最后一个点和第一个点重合，这样点虽然多了一个，但是能够让最后一个点的纹理和1.0f对应上，而不需要特殊处理
    std::vector<VertexAL> unitVertices;
    for (size_t i = 0; i < sectorCount; i++)
    {
        //从顶部向下看，x轴向右，z轴向下，所以逆时针顺序的角度是负值
        float sectorAngle = -(i * sectorStep);
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
    //生成三角形绘制顶点，注意最后两个三角形的纹理绕了一圈，要重新设置为x=1.0f;
    for (size_t i = 0; i < unitVertices.size(); i++)
    {
        VertexAL vertex1;
        VertexAL vertex2;
        VertexAL vertex3;
        if (i == unitVertices.size() - 2) {
            vertex1 = unitVertices[i];
            vertex2 = unitVertices[i + 1];
            vertex3 = unitVertices[0];
            vertex3.texcoord.x = 1.0f;//这里绕了一圈，纹理要重新设置为x=1.0f;
        }
        else if (i == unitVertices.size() - 1) {
            vertex1 = unitVertices[i];
            vertex2 = unitVertices[0];
            vertex3 = unitVertices[1];
            vertex2.texcoord.x = 1.0f;//这里绕了一圈，纹理要重新设置为x=1.0f;
            vertex3.texcoord.x = 1.0f;//这里绕了一圈，纹理要重新设置为x=1.0f;
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

void CreateSphereVertices(float radius, int xSegNum, int ySegNum, std::vector<VertexAL>& vertices, std::vector<unsigned int>& indices)
{
    // 假设一个顶点的纬度为α，范围-90~90度，经度为β，范围-180~180度，坐标(0,0,r)是正对我们的经纬度为0的顶点
    // xSegNum和ySegNum分别表示将经度范围(360度)和纬度范围(180度)分割了多少片段，xIndex和yIndex分别表示是第几个片段
    float xStep = 2 * PI / xSegNum;//每个x片段的弧度
    float yStep = PI / ySegNum;//每个y片段的弧度

    //生成顶点坐标，x轴方向以0~360进行遍历，y轴方向以-90~90进行遍历，方便生成纹理的uv坐标(水平坐标u，垂直坐标v)
    //这里遍历时xIndex <= xSegNum，而不是xIndex < xSegNum，这是为了让最后一个点和第一个点重合，这样点虽然多了一个，但是能够让最后一个点的纹理和1.0f对应上，而不需要特殊处理
    for (int xIndex = 0; xIndex <= xSegNum; xIndex++) {
        float yaw = xIndex * xStep;
        float u = (float)xIndex / (float)xSegNum;
        for (int yIndex = 0; yIndex <= ySegNum; yIndex++) {
            float pitch = -PI / 2.0f + yIndex * yStep;//这里从-90遍历到90
            float v = (float)yIndex / (float)ySegNum;
            float x = radius * cos(pitch) * sin(yaw);
            float y = radius * sin(pitch);
            float z = radius * cos(pitch) * cos(yaw);

            VertexAL vertex;
            vertex.position = glm::vec3(x, y, z);
            vertex.normal = glm::normalize(vertex.position);
            vertex.texcoord = glm::vec2(u, v);
            vertices.push_back(vertex);
        }
    }

    for (int i = 0; i < xSegNum; ++i) {
        for (int j = 0; j < ySegNum; ++j) {
            // 经纬网格面片的左下三角
            indices.push_back(i * (ySegNum + 1) + j);
            indices.push_back((i + 1) * (ySegNum + 1) + j);
            indices.push_back(i * (ySegNum + 1) + j + 1);
            // 经纬网格面片的右上三角
            indices.push_back(i * (ySegNum + 1) + j + 1);
            indices.push_back((i + 1) * (ySegNum + 1) + j);
            indices.push_back((i + 1) * (ySegNum + 1) + j + 1);
        }
    }
}
