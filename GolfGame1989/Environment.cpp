#include "pch.h"
#include "Environment.h"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>


Environment::Environment()
{
    bool result = InitializeTerrain();
    LoadEnvironmentData();
    CreateDataStrings();
    const int startEnviron = 0;  // ToDo: add error checking 
    m_currentEnviron = m_environs[startEnviron];
    BuildFlagVertex(m_environs[startEnviron].holePosition);
    BuildHoleVertex(m_environs[startEnviron].holePosition);
      
    LoadFixtureBucket12th();
}

void Environment::BuildFlagVertex(DirectX::SimpleMath::Vector3 aPos)
{
    const float poleHeight = .1;
    const float flagWidth = .02;
    const float flagHeight = .01;
    const DirectX::XMVECTORF32 flagColor = DirectX::Colors::Red;
    const DirectX::XMVECTORF32 poleColor = DirectX::Colors::White;

    DirectX::SimpleMath::Vector3 poleBase = DirectX::SimpleMath::Vector3::Zero;
    DirectX::SimpleMath::Vector3 poleTop = poleBase;
    poleTop.y += poleHeight;
    DirectX::SimpleMath::Vector3 flagTip = poleTop;
    flagTip.y -= flagHeight;
    flagTip.x -= flagWidth;
    flagTip.z -= flagWidth;
    double windDirection = GetWindDirection();
    flagTip = DirectX::SimpleMath::Vector3::Transform(flagTip, DirectX::SimpleMath::Matrix::CreateRotationY(static_cast<float>(windDirection)));

    DirectX::SimpleMath::Vector3 flagBottom = poleTop;
    flagBottom.y -= flagHeight + flagHeight;

    DirectX::VertexPositionColor poleBaseVertex(poleBase + aPos, poleColor);
    DirectX::VertexPositionColor poleTopVertex(poleTop + aPos, poleColor);
    DirectX::VertexPositionColor flagTopVertex(poleTop + aPos, flagColor);
    DirectX::VertexPositionColor flagTipVertex(flagTip + aPos, flagColor);
    DirectX::VertexPositionColor flagBottomVertex(flagBottom + aPos, flagColor);

    m_flagVertex.clear();
    m_flagVertex.push_back(poleBaseVertex);
    m_flagVertex.push_back(poleTopVertex);
    m_flagVertex.push_back(flagTopVertex);
    m_flagVertex.push_back(flagTipVertex);
    m_flagVertex.push_back(flagBottomVertex);
}

void Environment::BuildHoleVertex(DirectX::SimpleMath::Vector3 aPos)
{
    m_holeVertex.clear();
    const int vertexCount = m_holeResolution;
    const float holeRadius= static_cast<float>(m_holeRadius * m_currentEnviron.scale);
    for (int i = 0; i <= vertexCount; ++i)
    {
        double t = Utility::GetPi() * 2 * i / vertexCount;
        m_holeVertex.push_back(DirectX::VertexPositionColor(DirectX::SimpleMath::Vector3(static_cast<float>((holeRadius * cos(t))), static_cast<float>(m_landingHeight), static_cast<float>((holeRadius * -sin(t)))) + aPos, DirectX::Colors::Gray));
    }
}

bool Environment::BuildTerrainModel()
{
    m_terrainModel.clear();
    m_terrainModel.resize((m_terrainHeight - 1) * (m_terrainWidth - 1) * 6);
    if (m_terrainModel.size() < 1)
    {
        return false;
    }

    int index = 0;

    for (int j = 0; j < (m_terrainHeight - 1); ++j)
    {
        for (int i = 0; i < (m_terrainWidth - 1); ++i)
        {
            // Get the indexes to the four points of the quad
            int index1 = (m_terrainWidth * j) + i;          // Upper left.
            int index2 = (m_terrainWidth * j) + (i + 1);      // Upper right.
            int index3 = (m_terrainWidth * (j + 1)) + i;      // Bottom left.
            int index4 = (m_terrainWidth * (j + 1)) + (i + 1);  // Bottom right.

            // Now create two triangles for that quad
            // Triangle 1 - Upper left
            m_terrainModel[index].position = m_heightMap[index1].position;
            m_terrainModel[index].normal = m_heightMap[index1].normal;
            ++index;

            // Triangle 1 - Upper right
            m_terrainModel[index].position = m_heightMap[index2].position;
            m_terrainModel[index].normal = m_heightMap[index2].normal;
            ++index;

            // Triangle 1 - Bottom left
            m_terrainModel[index].position = m_heightMap[index3].position;
            m_terrainModel[index].normal = m_heightMap[index3].normal;
            ++index;

            // Triangle 2 - Bottom left
            m_terrainModel[index].position = m_heightMap[index3].position;
            m_terrainModel[index].normal = m_heightMap[index3].normal;
            ++index;

            // Triangle 2 - Upper right.
            m_terrainModel[index].position = m_heightMap[index2].position;
            m_terrainModel[index].normal = m_heightMap[index2].normal;
            ++index;

            // Triangle 2 - Bottom right.
            m_terrainModel[index].position = m_heightMap[index4].position;
            m_terrainModel[index].normal = m_heightMap[index4].normal;
            ++index;
        }
    }

    // to do: m_heightMap.clear(); once testing is complete
    return true;
}

bool Environment::CalculateTerrainNormals()
{
    std::vector<DirectX::SimpleMath::Vector3> normals;
    normals.clear();
    normals.resize((m_terrainHeight - 1) * (m_terrainWidth - 1));
    if (normals.size() < 1)
    {
        return false;
    }
    
    for (int j = 0; j < (m_terrainHeight - 1); ++j)
    {
        for (int i = 0; i < (m_terrainWidth - 1); ++i)
        {
            int index1 = ((j + 1) * m_terrainWidth) + i;      // Bottom left vertex.
            int index2 = ((j + 1) * m_terrainWidth) + (i + 1);  // Bottom right vertex.
            int index3 = (j * m_terrainWidth) + i;          // Upper left vertex.

            // Get three vertices from the face.
            DirectX::SimpleMath::Vector3 vertex1 = m_heightMap[index1].position;
            DirectX::SimpleMath::Vector3 vertex2 = m_heightMap[index2].position;
            DirectX::SimpleMath::Vector3 vertex3 = m_heightMap[index3].position;

            // Calculate the two vectors for this face.
            DirectX::SimpleMath::Vector3 vector1 = vertex1 - vertex3;
            DirectX::SimpleMath::Vector3 vector2 = vertex3 - vertex2;

            int index = (j * (m_terrainWidth - 1)) + i;

            normals[index] = DirectX::XMVector3Cross(vector1, vector2);
            normals[index].Normalize();
        }
    }

    // Now go through all the vertices and take a sum of the face normals that touch this vertex.
    for (int j = 0; j < m_terrainHeight; j++)
    {
        for (int i = 0; i < m_terrainWidth; i++)
        {
            DirectX::SimpleMath::Vector3 sum = DirectX::SimpleMath::Vector3::Zero;

            // Bottom left face.
            if (((i - 1) >= 0) && ((j - 1) >= 0))
            {
                int index = ((j - 1) * (m_terrainWidth - 1)) + (i - 1);
                sum += normals[index];
            }
            // Bottom right face.
            if ((i < (m_terrainWidth - 1)) && ((j - 1) >= 0))
            {
                int index = ((j - 1) * (m_terrainWidth - 1)) + i;
                sum += normals[index];
            }
            // Upper left face.
            if (((i - 1) >= 0) && (j < (m_terrainHeight - 1)))
            {
                int index = (j * (m_terrainWidth - 1)) + (i - 1);
                sum += normals[index];
            }
            // Upper right face.
            if ((i < (m_terrainWidth - 1)) && (j < (m_terrainHeight - 1)))
            {
                int index = (j * (m_terrainWidth - 1)) + i;
                sum += normals[index];
            }

            int index = (j * m_terrainWidth) + i;

            sum.Normalize();
            m_heightMap[index].normal = sum;
        }
    }

    return true;
}

void Environment::CreateDataStrings()
{
    std::stringstream inVal;
    inVal.precision(Utility::GetNumericalPrecisionForUI());

    for (int i = 0; i < m_environs.size(); ++i)
    {
        inVal.str(std::string());
        inVal << std::fixed << m_environs[i].airDensity;
        m_environs[i].airDensityStr = inVal.str();

        inVal.str(std::string());
        inVal << std::fixed << m_environs[i].gravity;
        m_environs[i].gravityStr = inVal.str();

        inVal.str(std::string());
        inVal << std::fixed << m_environs[i].wind.x;
        m_environs[i].windXStr = inVal.str();

        inVal.str(std::string());
        inVal << std::fixed << m_environs[i].wind.y;
        m_environs[i].windYStr = inVal.str();

        inVal.str(std::string());
        inVal << std::fixed << m_environs[i].wind.z;
        m_environs[i].windZStr = inVal.str();

        inVal.str(std::string());
        inVal << std::fixed << m_environs[i].landingFrictionScale;
        m_environs[i].landingFrictionScaleStr = inVal.str();

        inVal.str(std::string());
        inVal << std::fixed << m_environs[i].landingHardnessScale;
        m_environs[i].landingHardnessScaleStr = inVal.str();
    }
}

DirectX::SimpleMath::Vector3 Environment::GetGroundHeight(DirectX::SimpleMath::Vector3 aPos)
{
    DirectX::SimpleMath::Vector3 groundPos = aPos;
    groundPos.y = m_testHeight;
    return groundPos;
}

std::vector<DirectX::VertexPositionColor> Environment::GetTerrainColorVertex()
{
    std::vector<DirectX::VertexPositionColor> vertPosColor;
    vertPosColor.clear();
    vertPosColor.resize(m_terrainModel.size());
    DirectX::XMFLOAT4 terrainColor(1.0, 1.0, 1.0, 1.0); // ToDo: for testing, implement color control

    for (int i = 0; i < vertPosColor.size(); ++i)
    {
        vertPosColor[i].position = m_terrainModel[i].position;
        vertPosColor[i].color = terrainColor;
    }

    return vertPosColor;
}

float Environment::GetTerrainHeightAtPos(DirectX::XMFLOAT3 aPos) const
{
    DirectX::SimpleMath::Vector3 prePos = aPos;
    bool foundHeightBarry = false;
    bool foundHeight = false;
    int index = 0;
    
    for (int i = 0; i < m_terrainModel.size() / 3; ++i)
    //for (int i = 0; i < m_terrainModel.size(); ++i)
    {
        
        int index = i * 3;
        DirectX::XMFLOAT3 vertex1 = m_terrainModel[index].position;
        ++index;
        DirectX::XMFLOAT3 vertex2 = m_terrainModel[index].position;
        ++index;
        DirectX::XMFLOAT3 vertex3 = m_terrainModel[index].position;
        
        /*
        DirectX::XMFLOAT3 vertex1 = m_terrainModel[i].position;
        ++i;
        DirectX::XMFLOAT3 vertex2 = m_terrainModel[i].position;
        ++i;
        DirectX::XMFLOAT3 vertex3 = m_terrainModel[i].position;
        */
        foundHeight = CheckTerrainTriangleHeight(aPos, vertex1, vertex2, vertex3);

        float f = prePos.x;
        float g = prePos.z;
        DirectX::SimpleMath::Vector3 baryPos = DirectX::SimpleMath::Vector3::Barycentric(vertex1, vertex2, vertex3, f, g);

        //if (baryPos.x <= 1.0f && baryPos.y <= 1.0f && baryPos.z <= 1.0f)
        if (baryPos.x <= 1.0f && baryPos.x >= 0.0f && baryPos.y <= 1.0f && baryPos.y >= 0.0f && baryPos.z <= 1.0f && baryPos.z >= 0.0f)
        {
            foundHeightBarry = true;
        }
        else
        {
            foundHeightBarry = false;
        }


        if (foundHeight == true && foundHeightBarry == false)
        {
            int testBreak2 = 0;
            testBreak2++;
        }

        if (foundHeight == false && foundHeightBarry == true)
        {
            int testBreak2 = 0;
            testBreak2++;
        }

        if (foundHeight != foundHeightBarry)
        {
            int testBreak2 = 0;
            testBreak2++;
        }

        int testBreak = 0;

        if (foundHeight)
        {
            f = prePos.x;
            g = prePos.z;
            baryPos = DirectX::SimpleMath::Vector3::Barycentric(vertex1, vertex2, vertex3, f, g);

            testBreak = 0;
            return aPos.y;
        }
    }
    //float errorHeight = -2.0;
    float errorHeight = aPos.y;
    return errorHeight;
}

float Environment::GetTerrainHeightAtPos2(DirectX::XMFLOAT3 aPos) const
{
    bool foundHeight = false;
    int index = 0;

    for (int i = 0; i < m_terrainModel.size() / 3; ++i)
    {
        int index = i * 3;
        DirectX::XMFLOAT3 vertex1 = m_terrainModel[index].position;
        ++index;
        DirectX::XMFLOAT3 vertex2 = m_terrainModel[index].position;
        ++index;
        DirectX::XMFLOAT3 vertex3 = m_terrainModel[index].position;

        DirectX::SimpleMath::Vector3 testPos = aPos;

        foundHeight = CheckTerrainTriangleHeight(aPos, vertex1, vertex2, vertex3);

        if (foundHeight)
        {
            float f = testPos.x;
            float g = testPos.z;
            DirectX::SimpleMath::Vector3::Barycentric(vertex1, vertex2, vertex3, g, f, testPos);
            DirectX::SimpleMath::Vector3 testPos2 = aPos;
            //return testPos;
            int testBreak = 0;
            ++testBreak;
            return testPos.y;
            //return aPos.y;
        }
    }

    float errorHeight = -2.0;
    return errorHeight;
}

DirectX::SimpleMath::Vector3 Environment::GetTerrainNormal(DirectX::SimpleMath::Vector3 aPos) const
{
    bool foundHeight = false;

    for (int i = 0; i < m_terrainModel.size(); ++i)
    {
        DirectX::XMFLOAT3 vertex1 = m_terrainModel[i].position;
        ++i;
        DirectX::XMFLOAT3 vertex2 = m_terrainModel[i].position;
        ++i;
        DirectX::XMFLOAT3 vertex3 = m_terrainModel[i].position;
        
        DirectX::SimpleMath::Vector3 pos = aPos;
        foundHeight = CheckTerrainTriangleHeight(aPos, vertex1, vertex2, vertex3);

        if (foundHeight)
        {
            DirectX::SimpleMath::Vector3 norm1 = m_terrainModel[i-2].normal;
            DirectX::SimpleMath::Vector3 norm2 = m_terrainModel[i-1].normal;
            DirectX::SimpleMath::Vector3 norm3 = m_terrainModel[i].normal;

            float x = (norm1.x + norm2.x + norm3.x) / 3.0f;
            float y = (norm1.y + norm2.y + norm3.y) / 3.0f;
            float z = (norm1.z + norm2.z + norm3.z) / 3.0f;

            DirectX::SimpleMath::Vector3 norm(x, y, z);
 
            norm *= -1;
            norm.Normalize();
            return norm;
            //return aPos;
        }
    }
    
    
    return DirectX::SimpleMath::Vector3::UnitX;
}

DirectX::XMFLOAT3 Environment::GetTerrainPosition(DirectX::XMFLOAT3 aPos)
{
    bool foundHeight = false;
    int index = 0;

    for (int i = 0; i < m_terrainModel.size() / 3; ++i)
    {
        int index = i * 3;
        DirectX::XMFLOAT3 vertex1 = m_terrainModel[index].position;
        ++index;
        DirectX::XMFLOAT3 vertex2 = m_terrainModel[index].position;
        ++index;
        DirectX::XMFLOAT3 vertex3 = m_terrainModel[index].position;


        DirectX::SimpleMath::Vector3 testPos = aPos;
        foundHeight = CheckTerrainTriangleHeight(aPos, vertex1, vertex2, vertex3);

        if (foundHeight)
        {
            float f = testPos.x;
            float g = testPos.z;
            DirectX::SimpleMath::Vector3::Barycentric(vertex1, vertex2, vertex3, f, g, testPos);
            DirectX::SimpleMath::Vector3 testPos2 = aPos;
            //return testPos;
            int testBreak = 0;
            ++testBreak;
            return aPos;
        }
    }
    if (!foundHeight)
    {
        // height cannot be found
        throw std::exception();
    }
    
    return aPos;
}

bool Environment::CheckTerrainTriangleHeight(DirectX::XMFLOAT3& aPos, DirectX::XMFLOAT3 v0, DirectX::XMFLOAT3 v1, DirectX::XMFLOAT3 v2) const
{
    /*
    bool foundHeightBarry = false;
    float f = aPos.x;
    float g = aPos.z;
    DirectX::SimpleMath::Vector3 baryPos = DirectX::SimpleMath::Vector3::Barycentric(v0, v1, v2, f, g);

    //if (baryPos.x <= 1.0f && baryPos.y <= 1.0f && baryPos.z <= 1.0f)
    if (baryPos.x <= 1.0f && baryPos.x >= 0.0f && baryPos.y <= 1.0f && baryPos.y >= 0.0f && baryPos.z <= 1.0f && baryPos.z >= 0.0f)
    {
        DirectX::SimpleMath::Vector3 vZero = v0;
        DirectX::SimpleMath::Vector3 vOne = v1;
        DirectX::SimpleMath::Vector3 vTwo = v2;

        baryPos.Normalize();

        DirectX::SimpleMath::Vector3 cartPos;
        cartPos = baryPos.x * vZero + baryPos.y * vOne + baryPos.z * vTwo;
        
        float testSum = baryPos.x + baryPos.y + baryPos.z;
        //float pY = (baryPos.x * v0.y) + (baryPos.y * v1.y) + (baryPos.z * v2.y);
        //float pY = ((1 - f - g) * v0.y) + (f * v1.y) + (g * v2.y);
        float pY = ((1 - baryPos.y - baryPos.z) * v0.y) + (baryPos.y * v1.y) + (baryPos.z * v2.y);

        //bool testCheck = CheckTerrainTriangleHeight2(aPos, v0, v1, v2);

        DirectX::SimpleMath::Vector3 cartTest = baryPos.x * vZero + baryPos.y * vOne + baryPos.z * vTwo;


        int testBreak = 0;
        //aPos = cartTest;

        //aPos.y += baryPos.y;
        //aPos.y = aPos.y * cartPos.y;
        //aPos.y = cartPos.y;
        //aPos.y = cartPos.y + v2.y;
        //bool testBool = CheckTerrainTriangleHeight2(aPos, v0, v1, v2);
        foundHeightBarry = true;
    }
    else
    {
        foundHeightBarry = false;
    }

    return foundHeightBarry;
    */

    auto area = [](DirectX::SimpleMath::Vector3 p1, DirectX::SimpleMath::Vector3 p2, DirectX::SimpleMath::Vector3 p3)
    {
        float a = abs(static_cast<float>(p1.x * (p2.z - p3.z) + p2.x * (p3.z - p1.z) + p3.x * (p1.z - p2.z)) * .5);
        return a;
    };

    float AA = area(v0, v1, v2);
    float A1 = area(aPos, v1, v2);
    float A2 = area(v0, aPos, v2);
    float A3 = area(v0, v1, aPos);
    float AASum = A1 + A2 + A3;
    bool isAreaRight = (AA == A1 + A2 + A3);
    float AADifference = AA - AASum;
    
    // Starting position of the ray that is being cast
    DirectX::XMFLOAT3 startVector(aPos.x, 0.0f, aPos.z);
    
    // The direction the ray is being cast
    DirectX::XMFLOAT3 directionVector(0.0f, -1.0f, 0.0f);

    // Calculate the two edges from the three points given
    DirectX::SimpleMath::Vector3 edge1(v1.x - v0.x, v1.y - v0.y, v1.z - v0.z);
    DirectX::SimpleMath::Vector3 edge2(v2.x - v0.x, v2.y - v0.y, v2.z - v0.z);

    // Calculate the normal of the triangle from the two edges // ToDo use cross prod funcs
    DirectX::SimpleMath::Vector3 normal;

    edge1.Cross(edge2, normal);
    normal.Normalize();

    // Find the distance from the origin to the plane.
    float distance = ((-normal.x * v0.x) + (-normal.y * v0.y) + (-normal.z * v0.z));

    // Get the denominator of the equation.
    float denominator = ((normal.x * directionVector.x) + (normal.y * directionVector.y) + (normal.z * directionVector.z));

    // Make sure the result doesn't get too close to zero to prevent divide by zero.
    if (fabs(denominator) < 0.0001f)
    {
        return false;
    }

    // Get the numerator of the equation.
    float numerator = -1.0f * (((normal.x * startVector.x) + (normal.y * startVector.y) + (normal.z * startVector.z)) + distance);

    // Calculate where we intersect the triangle.
    float t = numerator / denominator;

    // Find the intersection vector.
    DirectX::SimpleMath::Vector3 Q;
    Q.x = startVector.x + (directionVector.x * t);
    Q.y = startVector.y + (directionVector.y * t);
    Q.z = startVector.z + (directionVector.z * t);

    // Find the three edges of the triangle.
    DirectX::SimpleMath::Vector3 e1(v1.x - v0.x, v1.y - v0.y, v1.z - v0.z);
    DirectX::SimpleMath::Vector3 e2(v2.x - v1.x, v2.y - v1.y, v2.z - v1.z);
    DirectX::SimpleMath::Vector3 e3(v0.x - v2.x, v0.y - v2.y, v0.z - v2.z);

    // Calculate the normal for the first edge.
    DirectX::SimpleMath::Vector3 edgeNormal;
    e1.Cross(normal, edgeNormal);

    // Calculate the determinant to see if it is on the inside, outside, or directly on the edge.
    DirectX::SimpleMath::Vector3 temp(Q.x - v1.x, Q.y - v1.y, Q.z - v1.z);

    float determinant = ((edgeNormal.x * temp.x) + (edgeNormal.y * temp.y) + (edgeNormal.z * temp.z));

    // Check if it is outside.
    if (determinant > 0.001f)
    {
        return false;
    }

    // Calculate the normal for the second edge
    e2.Cross(normal, edgeNormal);

    // Calculate the determinant to see if it is on the inside, outside, or directly on the edge.
    temp.x = Q.x - v1.x;
    temp.y = Q.y - v1.y;
    temp.z = Q.z - v1.z;

    determinant = ((edgeNormal.x * temp.x) + (edgeNormal.y * temp.y) + (edgeNormal.z * temp.z));

    // Check if it is outside.
    if (determinant > 0.001f)
    {
        return false;
    }

    // Calculate the normal for the third edge.
    e3.Cross(normal, edgeNormal);

    // Calculate the determinant to see if it is on the inside, outside, or directly on the edge.
    temp.x = Q.x - v2.x;
    temp.y = Q.y - v2.y;
    temp.z = Q.z - v2.z;

    determinant = ((edgeNormal.x * temp.x) + (edgeNormal.y * temp.y) + (edgeNormal.z * temp.z));

    // Check if it is outside.
    if (determinant > 0.001f)
    {
        return false;
    }
    
    ////////////////////////////////////////////

    float f = aPos.x;
    float g = aPos.z;
    DirectX::SimpleMath::Vector3 vbaryPos = DirectX::SimpleMath::Vector3::Barycentric(v0, v1, v2, f, g);

    DirectX::SimpleMath::Vector3 vZero(0.0, 0.0, 0.0);
    DirectX::SimpleMath::Vector3 vOne(-1.0, 0.0, -1.0);
    DirectX::SimpleMath::Vector3 vTwo(0.0, 0.0, -1.0);
    f = -0.5f;
    g = 0.5f;
    DirectX::SimpleMath::Vector3 testBary = DirectX::SimpleMath::Vector3::Barycentric(vZero, vOne, vTwo, f, g);



    int testBreak = 0;

    ////////////////////////////////////////////

    // Now we have our height.
    aPos.y = vbaryPos.y;
    //aPos.y = Q.y;
    return true;
    
}

bool Environment::CheckTerrainTriangleHeight3(DirectX::XMFLOAT3& aPos, DirectX::XMFLOAT3 v0, DirectX::XMFLOAT3 v1, DirectX::XMFLOAT3 v2) const
{
    /*
    bool foundHeightBarry = false;
    float f = aPos.x;
    float g = aPos.z;
    DirectX::SimpleMath::Vector3 baryPos = DirectX::SimpleMath::Vector3::Barycentric(v0, v1, v2, f, g);

    //if (baryPos.x <= 1.0f && baryPos.y <= 1.0f && baryPos.z <= 1.0f)
    if (baryPos.x <= 1.0f && baryPos.x >= 0.0f && baryPos.y <= 1.0f && baryPos.y >= 0.0f && baryPos.z <= 1.0f && baryPos.z >= 0.0f)
    {
        DirectX::SimpleMath::Vector3 vZero = v0;
        DirectX::SimpleMath::Vector3 vOne = v1;
        DirectX::SimpleMath::Vector3 vTwo = v2;

        baryPos.Normalize();

        DirectX::SimpleMath::Vector3 cartPos;
        cartPos = baryPos.x * vZero + baryPos.y * vOne + baryPos.z * vTwo;

        float testSum = baryPos.x + baryPos.y + baryPos.z;
        //float pY = (baryPos.x * v0.y) + (baryPos.y * v1.y) + (baryPos.z * v2.y);
        //float pY = ((1 - f - g) * v0.y) + (f * v1.y) + (g * v2.y);
        float pY = ((1 - baryPos.y - baryPos.z) * v0.y) + (baryPos.y * v1.y) + (baryPos.z * v2.y);

        //bool testCheck = CheckTerrainTriangleHeight2(aPos, v0, v1, v2);

        DirectX::SimpleMath::Vector3 cartTest = baryPos.x * vZero + baryPos.y * vOne + baryPos.z * vTwo;


        int testBreak = 0;
        //aPos = cartTest;

        //aPos.y += baryPos.y;
        //aPos.y = aPos.y * cartPos.y;
        //aPos.y = cartPos.y;
        //aPos.y = cartPos.y + v2.y;
        //bool testBool = CheckTerrainTriangleHeight2(aPos, v0, v1, v2);
        foundHeightBarry = true;
    }
    else
    {
        foundHeightBarry = false;
    }

    return foundHeightBarry;
    */




    // Starting position of the ray that is being cast
    DirectX::XMFLOAT3 startVector(aPos.x, 0.0f, aPos.z);

    // The direction the ray is being cast
    DirectX::XMFLOAT3 directionVector(0.0f, -1.0f, 0.0f);

    // Calculate the two edges from the three points given
    DirectX::SimpleMath::Vector3 edge1(v1.x - v0.x, v1.y - v0.y, v1.z - v0.z);
    DirectX::SimpleMath::Vector3 edge2(v2.x - v0.x, v2.y - v0.y, v2.z - v0.z);

    // Calculate the normal of the triangle from the two edges // ToDo use cross prod funcs
    DirectX::SimpleMath::Vector3 normal;

    edge1.Cross(edge2, normal);
    normal.Normalize();

    // Find the distance from the origin to the plane.
    float distance = ((-normal.x * v0.x) + (-normal.y * v0.y) + (-normal.z * v0.z));

    // Get the denominator of the equation.
    float denominator = ((normal.x * directionVector.x) + (normal.y * directionVector.y) + (normal.z * directionVector.z));

    // Make sure the result doesn't get too close to zero to prevent divide by zero.
    if (fabs(denominator) < 0.0001f)
    {
        return false;
    }

    // Get the numerator of the equation.
    float numerator = -1.0f * (((normal.x * startVector.x) + (normal.y * startVector.y) + (normal.z * startVector.z)) + distance);

    // Calculate where we intersect the triangle.
    float t = numerator / denominator;

    // Find the intersection vector.
    DirectX::SimpleMath::Vector3 Q;
    Q.x = startVector.x + (directionVector.x * t);
    Q.y = startVector.y + (directionVector.y * t);
    Q.z = startVector.z + (directionVector.z * t);

    // Find the three edges of the triangle.
    DirectX::SimpleMath::Vector3 e1(v1.x - v0.x, v1.y - v0.y, v1.z - v0.z);
    DirectX::SimpleMath::Vector3 e2(v2.x - v1.x, v2.y - v1.y, v2.z - v1.z);
    DirectX::SimpleMath::Vector3 e3(v0.x - v2.x, v0.y - v2.y, v0.z - v2.z);

    // Calculate the normal for the first edge.
    DirectX::SimpleMath::Vector3 edgeNormal;
    e1.Cross(normal, edgeNormal);

    // Calculate the determinant to see if it is on the inside, outside, or directly on the edge.
    DirectX::SimpleMath::Vector3 temp(Q.x - v1.x, Q.y - v1.y, Q.z - v1.z);

    float determinant = ((edgeNormal.x * temp.x) + (edgeNormal.y * temp.y) + (edgeNormal.z * temp.z));

    // Check if it is outside.
    if (determinant > 0.001f)
    {
        return false;
    }

    // Calculate the normal for the second edge
    e2.Cross(normal, edgeNormal);

    // Calculate the determinant to see if it is on the inside, outside, or directly on the edge.
    temp.x = Q.x - v1.x;
    temp.y = Q.y - v1.y;
    temp.z = Q.z - v1.z;

    determinant = ((edgeNormal.x * temp.x) + (edgeNormal.y * temp.y) + (edgeNormal.z * temp.z));

    // Check if it is outside.
    if (determinant > 0.001f)
    {
        return false;
    }

    // Calculate the normal for the third edge.
    e3.Cross(normal, edgeNormal);

    // Calculate the determinant to see if it is on the inside, outside, or directly on the edge.
    temp.x = Q.x - v2.x;
    temp.y = Q.y - v2.y;
    temp.z = Q.z - v2.z;

    determinant = ((edgeNormal.x * temp.x) + (edgeNormal.y * temp.y) + (edgeNormal.z * temp.z));

    // Check if it is outside.
    if (determinant > 0.001f)
    {
        return false;
    }

    ////////////////////////////////////////////

    float f = aPos.x;
    float g = aPos.z;
    DirectX::SimpleMath::Vector3 vbaryPos = DirectX::SimpleMath::Vector3::Barycentric(v0, v1, v2, f, g);

    DirectX::SimpleMath::Vector3 vZero(0.0, 0.0, 0.0);
    DirectX::SimpleMath::Vector3 vOne(-1.0, 0.0, -1.0);
    DirectX::SimpleMath::Vector3 vTwo(0.0, 0.0, -1.0);
    f = -0.5f;
    g = 0.5f;
    DirectX::SimpleMath::Vector3 testBary = DirectX::SimpleMath::Vector3::Barycentric(vZero, vOne, vTwo, f, g);



    int testBreak = 0;

    ////////////////////////////////////////////

    // Now we have our height.
    aPos.y = Q.y;
    return true;

}

bool Environment::CheckTerrainTriangleHeight2(DirectX::XMFLOAT3& aPos, DirectX::XMFLOAT3 v0, DirectX::XMFLOAT3 v1, DirectX::XMFLOAT3 v2) const
{
    // Starting position of the ray that is being cast
    DirectX::XMFLOAT3 startVector(aPos.x, 0.0f, aPos.z);

    // The direction the ray is being cast
    DirectX::XMFLOAT3 directionVector(0.0f, -1.0f, 0.0f);

    // Calculate the two edges from the three points given
    DirectX::SimpleMath::Vector3 edge1(v1.x - v0.x, v1.y - v0.y, v1.z - v0.z);
    DirectX::SimpleMath::Vector3 edge2(v2.x - v0.x, v2.y - v0.y, v2.z - v0.z);

    // Calculate the normal of the triangle from the two edges // ToDo use cross prod funcs
    DirectX::SimpleMath::Vector3 normal;

    edge1.Cross(edge2, normal);
    normal.Normalize();

    // Find the distance from the origin to the plane.
    float distance = ((-normal.x * v0.x) + (-normal.y * v0.y) + (-normal.z * v0.z));

    // Get the denominator of the equation.
    float denominator = ((normal.x * directionVector.x) + (normal.y * directionVector.y) + (normal.z * directionVector.z));

    // Make sure the result doesn't get too close to zero to prevent divide by zero.
    if (fabs(denominator) < 0.0001f)
    {
        return false;
    }

    // Get the numerator of the equation.
    float numerator = -1.0f * (((normal.x * startVector.x) + (normal.y * startVector.y) + (normal.z * startVector.z)) + distance);

    // Calculate where we intersect the triangle.
    float t = numerator / denominator;

    // Find the intersection vector.
    DirectX::SimpleMath::Vector3 Q;
    Q.x = startVector.x + (directionVector.x * t);
    Q.y = startVector.y + (directionVector.y * t);
    Q.z = startVector.z + (directionVector.z * t);

    // Find the three edges of the triangle.
    DirectX::SimpleMath::Vector3 e1(v1.x - v0.x, v1.y - v0.y, v1.z - v0.z);
    DirectX::SimpleMath::Vector3 e2(v2.x - v1.x, v2.y - v1.y, v2.z - v1.z);
    DirectX::SimpleMath::Vector3 e3(v0.x - v2.x, v0.y - v2.y, v0.z - v2.z);

    // Calculate the normal for the first edge.
    DirectX::SimpleMath::Vector3 edgeNormal;
    e1.Cross(normal, edgeNormal);

    // Calculate the determinant to see if it is on the inside, outside, or directly on the edge.
    DirectX::SimpleMath::Vector3 temp(Q.x - v1.x, Q.y - v1.y, Q.z - v1.z);

    float determinant = ((edgeNormal.x * temp.x) + (edgeNormal.y * temp.y) + (edgeNormal.z * temp.z));

    // Check if it is outside.
    if (determinant > 0.001f)
    {
        return false;
    }

    // Calculate the normal for the second edge
    e2.Cross(normal, edgeNormal);

    // Calculate the determinant to see if it is on the inside, outside, or directly on the edge.
    temp.x = Q.x - v1.x;
    temp.y = Q.y - v1.y;
    temp.z = Q.z - v1.z;

    determinant = ((edgeNormal.x * temp.x) + (edgeNormal.y * temp.y) + (edgeNormal.z * temp.z));

    // Check if it is outside.
    if (determinant > 0.001f)
    {
        return false;
    }

    // Calculate the normal for the third edge.
    e3.Cross(normal, edgeNormal);

    // Calculate the determinant to see if it is on the inside, outside, or directly on the edge.
    temp.x = Q.x - v2.x;
    temp.y = Q.y - v2.y;
    temp.z = Q.z - v2.z;

    determinant = ((edgeNormal.x * temp.x) + (edgeNormal.y * temp.y) + (edgeNormal.z * temp.z));

    // Check if it is outside.
    if (determinant > 0.001f)
    {
        return false;
    }

    // Now we have our height.
    aPos.y = Q.y;
    return true;
}

// While this could be done once per environment update, future updates could have moment to moment wind changes
double Environment::GetWindDirection() const
{ 
    DirectX::SimpleMath::Vector3 windVec = m_currentEnviron.wind;
    DirectX::SimpleMath::Vector3 zeroDirection(-1.0, 0.0, -1.0);
    double direction = DirectX::XMVectorGetX(DirectX::XMVector3AngleBetweenNormals(DirectX::XMVector3Normalize(windVec), DirectX::XMVector3Normalize(zeroDirection)));
    if (DirectX::XMVectorGetY(DirectX::XMVector3Cross(windVec, zeroDirection)) > 0.0f)
    {
        direction = -direction;
    }

    return direction;
}

bool Environment::InitializeTerrain()
{
    bool result = LoadHeightMap();
    if (!result)
    {
        return false;
    }

    result = CalculateTerrainNormals();
    if (!result)
    {
        return false;
    }

    result = BuildTerrainModel();
    if (!result)
    {
        return false;
    }

    ScaleTerrain();

    return true;
}

void Environment::LoadEnvironmentData()
{
    m_environs.clear();
    m_environs.resize(m_environsAvailable);

    int i = 0;

    m_environs[i].name = "12th";   
    m_environs[i].airDensity = 1.225;
    m_environs[i].gravity = -9.8;
    m_environs[i].holePosition = DirectX::SimpleMath::Vector3(3.0f, 0.0f, -0.5f); // 12th positon
    SetPosToTerrain(m_environs[i].holePosition);
    m_environs[i].landingFrictionScale = 1.0;
    m_environs[i].landingHardnessScale = 1.0;
    m_environs[i].par = 3;
    m_environs[i].scale = 0.02;
    m_environs[i].teeDirection = 0.0f;
    //m_environs[i].teePosition = DirectX::SimpleMath::Vector3(-2.0f, 0.0f, -2.0f); 
    m_environs[i].teePosition = DirectX::SimpleMath::Vector3(0.1f, 0.0f, 0.0f);
    SetPosToTerrain(m_environs[i].teePosition);
    m_environs[i].terrainColor = DirectX::Colors::Green;
    //m_environs[i].wind = DirectX::SimpleMath::Vector3(-0.4f, 0.0f, -0.9f);
    m_environs[i].wind = DirectX::SimpleMath::Vector3(0.0f, 0.0f, 0.0f);

    ++i;
    m_environs[i].name = "Breezy";    
    m_environs[i].airDensity = 1.225;
    m_environs[i].gravity = -9.8;
    m_environs[i].holePosition = DirectX::SimpleMath::Vector3(2.1f, 0.0f, 1.0f);
    m_environs[i].landingFrictionScale = 1.0;
    m_environs[i].landingHardnessScale = 1.0;
    m_environs[i].par = 5;
    m_environs[i].scale = 0.02;
    m_environs[i].teeDirection = 0.0f;
    m_environs[i].teePosition = DirectX::SimpleMath::Vector3(-2.0f, 0.0f, 0.0f);
    m_environs[i].terrainColor = DirectX::Colors::Green;
    m_environs[i].wind = DirectX::SimpleMath::Vector3(-10.0f, 0.0f, 0.0f);

    m_environs[i].name = "Calm";
    m_environs[i].airDensity = 1.225;
    m_environs[i].gravity = -9.8;
    m_environs[i].holePosition = DirectX::SimpleMath::Vector3(1.0f, 0.0f, 1.5f);
    m_environs[i].landingFrictionScale = 1.0;
    m_environs[i].landingHardnessScale = 1.0;
    m_environs[i].par = 3;
    m_environs[i].scale = 0.02;
    m_environs[i].teeDirection = 0.0f;
    m_environs[i].teePosition = DirectX::SimpleMath::Vector3(-2.0f, 0.0f, 0.0f);
    m_environs[i].terrainColor = DirectX::Colors::Green;
    m_environs[i].wind = DirectX::SimpleMath::Vector3(0.0f, 0.0f, 0.0f);

    /*
    ++i;
    m_environs[i].name = "Non Terrestrial (Alien Golf!!)";    
    m_environs[i].airDensity = 11.2;
    m_environs[i].gravity = -5.8;
    m_environs[i].holePosition = DirectX::SimpleMath::Vector3(0.0f, 0.0f, -1.0f);
    m_environs[i].landingFrictionScale = 1.0;
    m_environs[i].landingHardnessScale = 1.0;
    m_environs[i].par = 5;
    m_environs[i].scale = 0.02;
    m_environs[i].teeDirection = 45.0f;
    m_environs[i].teePosition = DirectX::SimpleMath::Vector3(-1.0f, 0.0f, 0.0f);
    m_environs[i].terrainColor = DirectX::Colors::Blue;
    m_environs[i].wind = DirectX::SimpleMath::Vector3(3.0f, 0.0f, 0.69f);
    */
}

void Environment::LoadFixtureBucket()
{
    m_fixtureBucket.clear();
    

    // add FlagStick   
    Fixture flagStick;
    flagStick.idNumber = 0;
    flagStick.position = m_currentEnviron.holePosition;
    flagStick.fixtureType = FixtureType::FIXTURETYPE_FLAGSTICK;
    flagStick.animationVariation = 0.0;
    flagStick.distanceToCamera = DirectX::SimpleMath::Vector3::Distance(flagStick.position, m_currentEnviron.teePosition);
    m_fixtureBucket.push_back(flagStick);
    
    // add Tee Box;
    Fixture teeBox;
    teeBox.idNumber = 1;
    teeBox.position = m_currentEnviron.teePosition;
    teeBox.fixtureType = FixtureType::FIXTURETYPE_TEEBOX;
    teeBox.animationVariation = 0.0;
    teeBox.distanceToCamera = DirectX::SimpleMath::Vector3::Distance(teeBox.position, m_currentEnviron.teePosition);
    m_fixtureBucket.push_back(teeBox);

    // randomized fixtures
    const float varMax = 10.0;
    
    const float posMin = -2.0;
    const float posMax = 4.0;
    const float xPosMin = 0.0;
    const float xPosMax = 7.0;
    const float yPos = m_landingHeight;
    const float zPosMin = -2.0;
    const float zPosMax = -1.6;

    const int fixtureTypeNumMin = 1;
    const int fixtureTypeNumMax = 6;
    const int fixtureCount = 55;
    int leftOrRightFairwayPlacement = 1;
    for (int i = 2; i < fixtureCount; ++i)  // start at 2 due to 0 being flag/hole and 1 being the tee box
    {
        float x = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX / (xPosMax))) - 1.0;
        float y = yPos;
        float z = static_cast <float> (rand()) / static_cast <float> (RAND_MAX / (zPosMax - zPosMin)) - 2.0;
        z *= leftOrRightFairwayPlacement; // to alternate tree placement or right or left side of fairway
        leftOrRightFairwayPlacement *= -1;
        float aVar = static_cast <float> (rand()) / static_cast <float> (RAND_MAX/ varMax);

        Fixture fixt;

        fixt.idNumber = i;
        fixt.position = DirectX::SimpleMath::Vector3(x, y, z);

        int fixtureNum = fixtureTypeNumMin + rand() / (RAND_MAX / (fixtureTypeNumMax - fixtureTypeNumMin));
        if (fixtureNum == 1)
        {
            fixt.fixtureType = FixtureType::FIXTURETYPE_TREE03;
        }
        else if (fixtureNum == 2)
        {
            fixt.fixtureType = FixtureType::FIXTURETYPE_TREE04;
        }
        else if (fixtureNum == 3)
        {
            fixt.fixtureType = FixtureType::FIXTURETYPE_TREE06;
        }
        else if (fixtureNum == 4)
        {
            fixt.fixtureType = FixtureType::FIXTURETYPE_TREE07;
        }
        else if (fixtureNum == 5)
        {
            fixt.fixtureType = FixtureType::FIXTURETYPE_TREE09;
        }
        else
        {
            fixt.fixtureType = FixtureType::FIXTURETYPE_TREE09;
        }

        fixt.animationVariation = aVar;
        fixt.distanceToCamera = DirectX::SimpleMath::Vector3::Distance(fixt.position, m_currentEnviron.teePosition);
        
        m_fixtureBucket.push_back(fixt);
    }

    // FIXTURETYPE_TREE03  V yellow green
    // FIXTURETYPE_TREE04  Circle Orange
    // FIXTURETYPE_TREE06  Triangle dark sides
    // FIXTURETYPE_TREE07  A tree
    // FIXTURETYPE_TREE09  A outline

    int i = 0;
    i = fixtureCount;
    Fixture fixt;
    fixt.idNumber = i;
    fixt.animationVariation = static_cast <float> (rand()) / static_cast <float> (RAND_MAX / 10.0);
    fixt.fixtureType = FixtureType::FIXTURETYPE_TREE03;
    fixt.position = DirectX::SimpleMath::Vector3(5.9, 0.0, 0.1);
    fixt.distanceToCamera = DirectX::SimpleMath::Vector3::Distance(fixt.position, m_currentEnviron.teePosition);
    m_fixtureBucket.push_back(fixt);

    ++i;
    fixt.idNumber = i;
    fixt.animationVariation = static_cast <float> (rand()) / static_cast <float> (RAND_MAX / 10.0);
    fixt.fixtureType = FixtureType::FIXTURETYPE_TREE04;
    fixt.position = DirectX::SimpleMath::Vector3(5.5, 0.0, -1.4);
    fixt.distanceToCamera = DirectX::SimpleMath::Vector3::Distance(fixt.position, m_currentEnviron.teePosition);
    m_fixtureBucket.push_back(fixt);

    ++i;
    fixt.idNumber = i;
    fixt.animationVariation = static_cast <float> (rand()) / static_cast <float> (RAND_MAX / 10.0);
    fixt.fixtureType = FixtureType::FIXTURETYPE_TREE06;
    fixt.position = DirectX::SimpleMath::Vector3(5.65, 0.0, -1.14);
    fixt.distanceToCamera = DirectX::SimpleMath::Vector3::Distance(fixt.position, m_currentEnviron.teePosition);
    m_fixtureBucket.push_back(fixt);

    ++i;
    fixt.idNumber = i;
    fixt.animationVariation = static_cast <float> (rand()) / static_cast <float> (RAND_MAX / 10.0);
    fixt.fixtureType = FixtureType::FIXTURETYPE_TREE09;
    fixt.position = DirectX::SimpleMath::Vector3(5.85, 0.0, -0.99);
    fixt.distanceToCamera = DirectX::SimpleMath::Vector3::Distance(fixt.position, m_currentEnviron.teePosition);
    m_fixtureBucket.push_back(fixt);

    ++i;
    fixt.idNumber = i;
    fixt.animationVariation = static_cast <float> (rand()) / static_cast <float> (RAND_MAX / 10.0);
    fixt.fixtureType = FixtureType::FIXTURETYPE_TREE03;
    fixt.position = DirectX::SimpleMath::Vector3(5.89, 0.0, -0.81);
    fixt.distanceToCamera = DirectX::SimpleMath::Vector3::Distance(fixt.position, m_currentEnviron.teePosition);
    m_fixtureBucket.push_back(fixt);

    ++i;
    fixt.idNumber = i;
    fixt.animationVariation = static_cast <float> (rand()) / static_cast <float> (RAND_MAX / 10.0);
    fixt.fixtureType = FixtureType::FIXTURETYPE_TREE09;
    fixt.position = DirectX::SimpleMath::Vector3(5.91, 0.0, -0.72);
    fixt.distanceToCamera = DirectX::SimpleMath::Vector3::Distance(fixt.position, m_currentEnviron.teePosition);
    m_fixtureBucket.push_back(fixt);

    ++i;
    fixt.idNumber = i;
    fixt.animationVariation = static_cast <float> (rand()) / static_cast <float> (RAND_MAX / 10.0);
    fixt.fixtureType = FixtureType::FIXTURETYPE_TREE04;
    fixt.position = DirectX::SimpleMath::Vector3(5.85, 0.0, -0.39);
    fixt.distanceToCamera = DirectX::SimpleMath::Vector3::Distance(fixt.position, m_currentEnviron.teePosition);
    m_fixtureBucket.push_back(fixt);

    ++i;
    fixt.idNumber = i;
    fixt.animationVariation = static_cast <float> (rand()) / static_cast <float> (RAND_MAX / 10.0);
    fixt.fixtureType = FixtureType::FIXTURETYPE_TREE06;
    fixt.position = DirectX::SimpleMath::Vector3(5.76, 0.0, -0.17);
    fixt.distanceToCamera = DirectX::SimpleMath::Vector3::Distance(fixt.position, m_currentEnviron.teePosition);
    m_fixtureBucket.push_back(fixt);

    ///////////////////// start positive z locs

    ++i;
    fixt.idNumber = i;
    fixt.animationVariation = static_cast <float> (rand()) / static_cast <float> (RAND_MAX / 10.0);
    fixt.fixtureType = FixtureType::FIXTURETYPE_TREE09;
    fixt.position = DirectX::SimpleMath::Vector3(5.92, 0.0, 0.27);
    fixt.distanceToCamera = DirectX::SimpleMath::Vector3::Distance(fixt.position, m_currentEnviron.teePosition);
    m_fixtureBucket.push_back(fixt);

    ++i;
    fixt.idNumber = i;
    fixt.animationVariation = static_cast <float> (rand()) / static_cast <float> (RAND_MAX / 10.0);
    fixt.fixtureType = FixtureType::FIXTURETYPE_TREE04;
    fixt.position = DirectX::SimpleMath::Vector3(5.96, 0.0, 0.47);
    fixt.distanceToCamera = DirectX::SimpleMath::Vector3::Distance(fixt.position, m_currentEnviron.teePosition);
    m_fixtureBucket.push_back(fixt);

    ++i;
    fixt.idNumber = i;
    fixt.animationVariation = static_cast <float> (rand()) / static_cast <float> (RAND_MAX / 10.0);
    fixt.fixtureType = FixtureType::FIXTURETYPE_TREE06;
    fixt.position = DirectX::SimpleMath::Vector3(5.82, 0.0, 0.66);
    fixt.distanceToCamera = DirectX::SimpleMath::Vector3::Distance(fixt.position, m_currentEnviron.teePosition);
    m_fixtureBucket.push_back(fixt);

    ++i;
    fixt.idNumber = i;
    fixt.animationVariation = static_cast <float> (rand()) / static_cast <float> (RAND_MAX / 10.0);
    fixt.fixtureType = FixtureType::FIXTURETYPE_TREE04;
    fixt.position = DirectX::SimpleMath::Vector3(5.79, 0.0, 0.93);
    fixt.distanceToCamera = DirectX::SimpleMath::Vector3::Distance(fixt.position, m_currentEnviron.teePosition);
    m_fixtureBucket.push_back(fixt);

    ++i;
    fixt.idNumber = i;
    fixt.animationVariation = static_cast <float> (rand()) / static_cast <float> (RAND_MAX / 10.0);
    fixt.fixtureType = FixtureType::FIXTURETYPE_TREE03;
    fixt.position = DirectX::SimpleMath::Vector3(5.69, 0.0, 1.23);
    fixt.distanceToCamera = DirectX::SimpleMath::Vector3::Distance(fixt.position, m_currentEnviron.teePosition);
    m_fixtureBucket.push_back(fixt);

    ++i;
    fixt.idNumber = i;
    fixt.animationVariation = static_cast <float> (rand()) / static_cast <float> (RAND_MAX / 10.0);
    fixt.fixtureType = FixtureType::FIXTURETYPE_TREE07;
    fixt.position = DirectX::SimpleMath::Vector3(5.539, 0.0, 1.45);
    fixt.distanceToCamera = DirectX::SimpleMath::Vector3::Distance(fixt.position, m_currentEnviron.teePosition);
    m_fixtureBucket.push_back(fixt);

    // start trees to create dog leg
    ////////////////////////////////////////////

    ++i;
    fixt.idNumber = i;
    fixt.animationVariation = static_cast <float> (rand()) / static_cast <float> (RAND_MAX / 10.0);
    fixt.fixtureType = FixtureType::FIXTURETYPE_TREE07;
    fixt.position = DirectX::SimpleMath::Vector3(3.0, 0.0, .75);
    fixt.distanceToCamera = DirectX::SimpleMath::Vector3::Distance(fixt.position, m_currentEnviron.teePosition);
    m_fixtureBucket.push_back(fixt);

    ++i;
    fixt.idNumber = i;
    fixt.animationVariation = static_cast <float> (rand()) / static_cast <float> (RAND_MAX / 10.0);
    fixt.fixtureType = FixtureType::FIXTURETYPE_TREE03;
    fixt.position = DirectX::SimpleMath::Vector3(2.7, 0.0, .97);
    fixt.distanceToCamera = DirectX::SimpleMath::Vector3::Distance(fixt.position, m_currentEnviron.teePosition);
    m_fixtureBucket.push_back(fixt);

    ++i;
    fixt.idNumber = i;
    fixt.animationVariation = static_cast <float> (rand()) / static_cast <float> (RAND_MAX / 10.0);
    fixt.fixtureType = FixtureType::FIXTURETYPE_TREE09;
    fixt.position = DirectX::SimpleMath::Vector3(2.4, 0.0, 1.17);
    fixt.distanceToCamera = DirectX::SimpleMath::Vector3::Distance(fixt.position, m_currentEnviron.teePosition);
    m_fixtureBucket.push_back(fixt);
}

bool Environment::SetPosToTerrainWithCheck(DirectX::XMFLOAT3& aPos)
{
    bool foundHeight = false;
    int index = 0;

    for (int i = 0; i < m_terrainModel.size() / 3; ++i)
    {
        int index = i * 3;
        DirectX::XMFLOAT3 vertex1 = m_terrainModel[index].position;
        ++index;
        DirectX::XMFLOAT3 vertex2 = m_terrainModel[index].position;
        ++index;
        DirectX::XMFLOAT3 vertex3 = m_terrainModel[index].position;

        foundHeight = CheckTerrainTriangleHeight(aPos, vertex1, vertex2, vertex3);

        if (foundHeight)
        {

            return true;
            //return aPos.y;
        }
    }

    return false;
}

void Environment::SetPosToTerrain(DirectX::XMFLOAT3& aPos)
{
    bool foundHeight = false;
    int index = 0;

    for (int i = 0; i < m_terrainModel.size() / 3; ++i)
    {
        if (foundHeight == false)
        {
            int index = i * 3;
            DirectX::XMFLOAT3 vertex1 = m_terrainModel[index].position;
            ++index;
            DirectX::XMFLOAT3 vertex2 = m_terrainModel[index].position;
            ++index;
            DirectX::XMFLOAT3 vertex3 = m_terrainModel[index].position;

            foundHeight = CheckTerrainTriangleHeight(aPos, vertex1, vertex2, vertex3);
        }
    } 
}

void Environment::LoadFixtureBucket12th()
{
    bool result;
    DirectX::XMFLOAT3 fixtPos;

    m_fixtureBucket.clear();

    // add FlagStick   
    Fixture flagStick;
    flagStick.idNumber = 0;
    // set position to allign with terrain height
    fixtPos = m_currentEnviron.holePosition;
    result = SetPosToTerrainWithCheck(fixtPos);
    flagStick.position = fixtPos;
    flagStick.fixtureType = FixtureType::FIXTURETYPE_FLAGSTICK;
    flagStick.animationVariation = 0.0;
    flagStick.distanceToCamera = DirectX::SimpleMath::Vector3::Distance(flagStick.position, m_currentEnviron.teePosition);
    m_fixtureBucket.push_back(flagStick);

    // add Tee Box;
    Fixture teeBox;
    teeBox.idNumber = 1;
    // set position to allign with terrain height
    fixtPos = m_currentEnviron.teePosition;
    result = SetPosToTerrainWithCheck(fixtPos);
    teeBox.position = fixtPos;
    teeBox.fixtureType = FixtureType::FIXTURETYPE_TEEBOX;
    teeBox.animationVariation = 0.0;
    teeBox.distanceToCamera = DirectX::SimpleMath::Vector3::Distance(teeBox.position, m_currentEnviron.teePosition);
    m_fixtureBucket.push_back(teeBox);

    // randomized fixtures
    const float varMax = 10.0;

    const float posMin = -2.0;
    const float posMax = 4.0;
    const float xPosMin = 0.0;
    const float xPosMax = 6.0;
    const float yPos = m_landingHeight;
    const float zPosMin = -2.0;
    const float zPosMax = -1.6;

    const int fixtureTypeNumMin = 1;
    const int fixtureTypeNumMax = 6;
    const int fixtureCount = 25;
    int leftOrRightFairwayPlacement = 1;
    for (int i = 2; i < fixtureCount; ++i)  // start at 2 due to 0 being flag/hole and 1 being the tee box
    {
        float x = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX / (xPosMax))) - 1.0;
        float y = yPos;
        float z = static_cast <float> (rand()) / static_cast <float> (RAND_MAX / (zPosMax - zPosMin)) - 2.0;
        z *= leftOrRightFairwayPlacement; // to alternate tree placement or right or left side of fairway
        leftOrRightFairwayPlacement *= -1;
        float aVar = static_cast <float> (rand()) / static_cast <float> (RAND_MAX / varMax);

        Fixture fixt;

        fixt.idNumber = i;

        // set position to allign with terrain height
        fixtPos = DirectX::XMFLOAT3(x, y, z);       
        result = SetPosToTerrainWithCheck(fixtPos);
        fixt.position = fixtPos;

        int fixtureNum = fixtureTypeNumMin + rand() / (RAND_MAX / (fixtureTypeNumMax - fixtureTypeNumMin));
        if (fixtureNum == 1)
        {
            fixt.fixtureType = FixtureType::FIXTURETYPE_TREE03;
        }
        else if (fixtureNum == 2)
        {
            fixt.fixtureType = FixtureType::FIXTURETYPE_TREE07;
        }
        else if (fixtureNum == 3)
        {
            fixt.fixtureType = FixtureType::FIXTURETYPE_TREE06;
        }
        else if (fixtureNum == 4)
        {
            fixt.fixtureType = FixtureType::FIXTURETYPE_TREE07;
        }
        else if (fixtureNum == 5)
        {
            fixt.fixtureType = FixtureType::FIXTURETYPE_TREE09;
        }
        else
        {
            fixt.fixtureType = FixtureType::FIXTURETYPE_TREE09;
        }

        fixt.animationVariation = aVar;
        fixt.distanceToCamera = DirectX::SimpleMath::Vector3::Distance(fixt.position, m_currentEnviron.teePosition);

        m_fixtureBucket.push_back(fixt);
    }

    int i = 0;
    i = fixtureCount;
    Fixture fixt;
    fixt.idNumber = i;
    fixt.animationVariation = static_cast <float> (rand()) / static_cast <float> (RAND_MAX / 10.0);
    fixt.fixtureType = FixtureType::FIXTURETYPE_TREE03;
    // set position to allign with terrain height
    fixtPos = DirectX::XMFLOAT3(4.9, 0.0, 0.1);
    result = SetPosToTerrainWithCheck(fixtPos);
    fixt.position = fixtPos;
    fixt.distanceToCamera = DirectX::SimpleMath::Vector3::Distance(fixt.position, m_currentEnviron.teePosition);
    m_fixtureBucket.push_back(fixt);

    ++i;
    fixt.idNumber = i;
    fixt.animationVariation = static_cast <float> (rand()) / static_cast <float> (RAND_MAX / 10.0);
    fixt.fixtureType = FixtureType::FIXTURETYPE_TREE09;
    // set position to allign with terrain height
    fixtPos = DirectX::XMFLOAT3(4.5, 0.0, -1.4);
    result = SetPosToTerrainWithCheck(fixtPos);
    fixt.position = fixtPos;
    fixt.distanceToCamera = DirectX::SimpleMath::Vector3::Distance(fixt.position, m_currentEnviron.teePosition);
    m_fixtureBucket.push_back(fixt);

    ++i;
    fixt.idNumber = i;
    fixt.animationVariation = static_cast <float> (rand()) / static_cast <float> (RAND_MAX / 10.0);
    fixt.fixtureType = FixtureType::FIXTURETYPE_TREE06;
    // set position to allign with terrain height
    fixtPos = DirectX::XMFLOAT3(4.65, 0.0, -1.14);
    result = SetPosToTerrainWithCheck(fixtPos);
    fixt.position = fixtPos;
    fixt.distanceToCamera = DirectX::SimpleMath::Vector3::Distance(fixt.position, m_currentEnviron.teePosition);
    m_fixtureBucket.push_back(fixt);

    ++i;
    fixt.idNumber = i;
    fixt.animationVariation = static_cast <float> (rand()) / static_cast <float> (RAND_MAX / 10.0);
    fixt.fixtureType = FixtureType::FIXTURETYPE_TREE09;
    // set position to allign with terrain height
    fixtPos = DirectX::XMFLOAT3(4.85, 0.0, -0.99);
    result = SetPosToTerrainWithCheck(fixtPos);
    fixt.position = fixtPos;
    fixt.distanceToCamera = DirectX::SimpleMath::Vector3::Distance(fixt.position, m_currentEnviron.teePosition);
    m_fixtureBucket.push_back(fixt);

    ++i;
    fixt.idNumber = i;
    fixt.animationVariation = static_cast <float> (rand()) / static_cast <float> (RAND_MAX / 10.0);
    fixt.fixtureType = FixtureType::FIXTURETYPE_TREE03;
    // set position to allign with terrain height
    fixtPos = DirectX::XMFLOAT3(4.89, 0.0, -0.81);
    result = SetPosToTerrainWithCheck(fixtPos);
    fixt.position = fixtPos;
    fixt.distanceToCamera = DirectX::SimpleMath::Vector3::Distance(fixt.position, m_currentEnviron.teePosition);
    m_fixtureBucket.push_back(fixt);

    ++i;
    fixt.idNumber = i;
    fixt.animationVariation = static_cast <float> (rand()) / static_cast <float> (RAND_MAX / 10.0);
    fixt.fixtureType = FixtureType::FIXTURETYPE_TREE04;
    // set position to allign with terrain height
    fixtPos = DirectX::XMFLOAT3(3.91, 0.0, -0.72);
    result = SetPosToTerrainWithCheck(fixtPos);
    fixt.position = fixtPos;
    fixt.distanceToCamera = DirectX::SimpleMath::Vector3::Distance(fixt.position, m_currentEnviron.teePosition);
    m_fixtureBucket.push_back(fixt);

    ++i;
    fixt.idNumber = i;
    fixt.animationVariation = static_cast <float> (rand()) / static_cast <float> (RAND_MAX / 10.0);
    fixt.fixtureType = FixtureType::FIXTURETYPE_TREE04;
    // set position to allign with terrain height
    fixtPos = DirectX::XMFLOAT3(3.85, 0.0, -0.39);
    result = SetPosToTerrainWithCheck(fixtPos);
    fixt.position = fixtPos;
    fixt.distanceToCamera = DirectX::SimpleMath::Vector3::Distance(fixt.position, m_currentEnviron.teePosition);
    m_fixtureBucket.push_back(fixt);

    ++i;
    fixt.idNumber = i;
    fixt.animationVariation = static_cast <float> (rand()) / static_cast <float> (RAND_MAX / 10.0);
    fixt.fixtureType = FixtureType::FIXTURETYPE_TREE04;
    // set position to allign with terrain height
    fixtPos = DirectX::XMFLOAT3(3.76, 0.0, -0.17);
    result = SetPosToTerrainWithCheck(fixtPos);
    fixt.position = fixtPos;
    fixt.distanceToCamera = DirectX::SimpleMath::Vector3::Distance(fixt.position, m_currentEnviron.teePosition);
    m_fixtureBucket.push_back(fixt);

    ///////////////////// start positive z locs

    ++i;
    fixt.idNumber = i;
    fixt.animationVariation = static_cast <float> (rand()) / static_cast <float> (RAND_MAX / 10.0);
    fixt.fixtureType = FixtureType::FIXTURETYPE_TREE04;
    // set position to allign with terrain height
    fixtPos = DirectX::XMFLOAT3(3.92, 0.0, -0.17);
    result = SetPosToTerrainWithCheck(fixtPos);
    fixt.position = fixtPos;
    fixt.distanceToCamera = DirectX::SimpleMath::Vector3::Distance(fixt.position, m_currentEnviron.teePosition);
    m_fixtureBucket.push_back(fixt);

    ++i;
    fixt.idNumber = i;
    fixt.animationVariation = static_cast <float> (rand()) / static_cast <float> (RAND_MAX / 10.0);
    fixt.fixtureType = FixtureType::FIXTURETYPE_TREE06;
    // set position to allign with terrain height
    fixtPos = DirectX::XMFLOAT3(4.96, 0.0, 0.47);
    result = SetPosToTerrainWithCheck(fixtPos);
    fixt.position = fixtPos;
    fixt.distanceToCamera = DirectX::SimpleMath::Vector3::Distance(fixt.position, m_currentEnviron.teePosition);
    m_fixtureBucket.push_back(fixt);

    ++i;
    fixt.idNumber = i;
    fixt.animationVariation = static_cast <float> (rand()) / static_cast <float> (RAND_MAX / 10.0);
    fixt.fixtureType = FixtureType::FIXTURETYPE_TREE07;
    // set position to allign with terrain height
    fixtPos = DirectX::XMFLOAT3(4.82, 0.0, 0.66);
    result = SetPosToTerrainWithCheck(fixtPos);
    fixt.position = fixtPos;
    fixt.distanceToCamera = DirectX::SimpleMath::Vector3::Distance(fixt.position, m_currentEnviron.teePosition);
    m_fixtureBucket.push_back(fixt);

    ++i;
    fixt.idNumber = i;
    fixt.animationVariation = static_cast <float> (rand()) / static_cast <float> (RAND_MAX / 10.0);
    fixt.fixtureType = FixtureType::FIXTURETYPE_TREE09;
    // set position to allign with terrain height
    fixtPos = DirectX::XMFLOAT3(4.79, 0.0, 0.93);
    result = SetPosToTerrainWithCheck(fixtPos);
    fixt.position = fixtPos;
    fixt.distanceToCamera = DirectX::SimpleMath::Vector3::Distance(fixt.position, m_currentEnviron.teePosition);
    m_fixtureBucket.push_back(fixt);

    ++i;
    fixt.idNumber = i;
    fixt.animationVariation = static_cast <float> (rand()) / static_cast <float> (RAND_MAX / 10.0);
    fixt.fixtureType = FixtureType::FIXTURETYPE_TREE03;
    // set position to allign with terrain height
    fixtPos = DirectX::XMFLOAT3(4.69, 0.0, 1.23);
    result = SetPosToTerrainWithCheck(fixtPos);
    fixt.position = fixtPos;
    fixt.distanceToCamera = DirectX::SimpleMath::Vector3::Distance(fixt.position, m_currentEnviron.teePosition);
    m_fixtureBucket.push_back(fixt);

    ++i;
    fixt.idNumber = i;
    fixt.animationVariation = static_cast <float> (rand()) / static_cast <float> (RAND_MAX / 10.0);
    fixt.fixtureType = FixtureType::FIXTURETYPE_TREE07;
    // set position to allign with terrain height
    fixtPos = DirectX::XMFLOAT3(4.539, 0.0, 1.45);
    result = SetPosToTerrainWithCheck(fixtPos);
    fixt.position = fixtPos;
    fixt.distanceToCamera = DirectX::SimpleMath::Vector3::Distance(fixt.position, m_currentEnviron.teePosition);
    m_fixtureBucket.push_back(fixt);
    
    ++i;
    fixt.idNumber = i;
    fixt.animationVariation = Utility::ToRadians(45.0);
    fixt.fixtureType = FixtureType::FIXTURETYPE_BRIDGE;
    // set position to allign with terrain height
    DirectX::XMFLOAT3 bridge1 = DirectX::XMFLOAT3(1.225, 0.0, -.9);
    result = SetPosToTerrainWithCheck(bridge1);
    fixt.position = bridge1;
    fixt.distanceToCamera = DirectX::SimpleMath::Vector3::Distance(fixt.position, m_currentEnviron.teePosition);
    m_fixtureBucket.push_back(fixt);

    ++i;
    fixt.idNumber = i;
    fixt.animationVariation = Utility::ToRadians(45.0);
    fixt.fixtureType = FixtureType::FIXTURETYPE_BRIDGE;
    // set position to allign with terrain height
    DirectX::XMFLOAT3 bridge2 = DirectX::XMFLOAT3(3.6, 0.0, 1.5);
    result = SetPosToTerrainWithCheck(bridge2);
    fixt.position = bridge2;
    fixt.distanceToCamera = DirectX::SimpleMath::Vector3::Distance(fixt.position, m_currentEnviron.teePosition);
    m_fixtureBucket.push_back(fixt);
}

bool Environment::LoadHeightMap()
{
    FILE* filePtr;    
    char* filename = "heightmap14.bmp";

    // Open the height map file 
    int error = fopen_s(&filePtr, filename, "rb");
    if (error != 0)
    {
        return false;
    }

    // Read in the file header
    BITMAPFILEHEADER bitmapFileHeader;
    size_t count = fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr);
    if (count != 1)
    {
        return false;
    }

    // Read in the bitmap info header.
    BITMAPINFOHEADER bitmapInfoHeader;
    count = fread(&bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr);
    if (count != 1)
    {
        return false;
    }

    // Save the dimensions of the terrain.
    m_terrainWidth = bitmapInfoHeader.biWidth;
    m_terrainHeight = bitmapInfoHeader.biHeight;

    // Calculate the size of the bitmap image data.
    //int imageSize = m_terrainWidth * m_terrainHeight * 3;
    int imageSize = m_terrainWidth * m_terrainHeight * 3 + m_terrainWidth;

    // Allocate memory for the bitmap image data.
    unsigned char* bitmapImage = new unsigned char[imageSize];
    if (!bitmapImage)
    {
        return false;
    }

    // Move to the beginning of the bitmap data.
    fseek(filePtr, bitmapFileHeader.bfOffBits, SEEK_SET);
    
    UINT pitch = m_terrainWidth * 3;
    UINT excessPitch = 0;
    while (double(pitch / 4) != double(pitch) / 4.0)
    {
        pitch++;
        excessPitch++;
    }

    // Read in the bitmap image data.
    //count = fread(bitmapImage, 1, imageSize, filePtr);
    count = fread(bitmapImage, 1, pitch * m_terrainHeight, filePtr);
    if (count != imageSize)
    {
        return false;
    }

    std::vector<char> testMap;
    testMap.clear();
    for (int i = 0; i < imageSize; ++i)
    {
        testMap.push_back(bitmapImage[i]);
    }

    // Close the file.
    error = fclose(filePtr);
    if (error != 0)
    {
        return false;
    }

    // Create the structure to hold the height map data.
    m_heightMap.clear();
    m_heightMap.resize(m_terrainWidth * m_terrainHeight);

    // Initialize the position in the image data buffer.
    int k = 0;
    int index;
    unsigned char height;
    
    // Read the image data into the height map.
    //for (int i = 0; i < m_terrainHeight; i++)
    for (int j = 0; j < m_terrainWidth; j++)
    {
        //for (int j = 0; j < m_terrainWidth; j++)
        for (int i = 0; i < m_terrainHeight; i++)
        {
            height = bitmapImage[k];
            
            // To read values in backwards since bitmap read in is fliped
            index = (m_terrainWidth * (m_terrainHeight - 1 - j)) + i;
            
            m_heightMap[index].position.x = (float)j;
            m_heightMap[index].position.y = (float)height * m_heightScale; // scale height during input
            m_heightMap[index].position.z = (float)i;
            
            k += 3;
        }
        k += excessPitch;
        //k += 1;
    }

    // Release the bitmap image data.
    delete[] bitmapImage;
    bitmapImage = 0;

    return true;
}

void Environment::ScaleTerrain()
{
    const float scale = .2;
    //const float scale = 10.0;
    const float xTransform = -1.4f;
    //const float yTransform = -.08f;
    const float yTransform = 5.0f;
    const float zTransform = -3.2f;

    for (int i = 0; i < m_heightMap.size(); ++i)
    {
        m_heightMap[i].position.x *= scale;
        m_heightMap[i].position.y *= scale;
        m_heightMap[i].position.z *= scale;

        m_heightMap[i].position.x += xTransform;
        m_heightMap[i].position.y += yTransform;
        m_heightMap[i].position.z += zTransform;
    }
    
    for (int i = 0; i < m_terrainModel.size(); ++i)
    {
        m_terrainModel[i].position.x *= scale;
        m_terrainModel[i].position.y *= scale;
        m_terrainModel[i].position.z *= scale;

        m_terrainModel[i].position.x += xTransform;
        m_terrainModel[i].position.y += yTransform;
        m_terrainModel[i].position.z += zTransform;
    }
}

void Environment::SetLandingHeight(double aLandingHeight)
{
    m_landingHeight = aLandingHeight;
}

void Environment::SetLauchHeight(double aLaunchHeight)
{
    m_launchHeight = aLaunchHeight;
}

void Environment::SortFixtureBucketByDistance()
{
    std::sort(m_fixtureBucket.begin(), m_fixtureBucket.end(),
        [](const auto& i, const auto& j) {return i.distanceToCamera > j.distanceToCamera; });
}

void Environment::UpdateEnvironment(const int aIndex)
{
    m_currentEnviron = m_environs[aIndex];
    m_landingFriction = m_environs[aIndex].landingFrictionScale;
    m_landingHardness = m_environs[aIndex].landingHardnessScale;
    BuildFlagVertex(m_environs[aIndex].holePosition);
    BuildHoleVertex(m_environs[aIndex].holePosition);
}

void Environment::UpdateFixtures(const DirectX::SimpleMath::Vector3 &aPos)
{
    UpdateFixtureDistanceToCamera(aPos);
    SortFixtureBucketByDistance();
}

void Environment::UpdateFixtureDistanceToCamera(const DirectX::SimpleMath::Vector3 &aCameraPos)
{
    for (int i = 0; i < m_fixtureBucket.size(); ++i)
    {
        m_fixtureBucket[i].distanceToCamera = DirectX::SimpleMath::Vector3::Distance(m_fixtureBucket[i].position, aCameraPos);
    }
}
