#include <array>
#include <iomanip>
#include <sstream>

#define T_PGE_APPLICATION
#include "engine/tPixelGameEngine.h"

#define PI 3.14159265358979323846f

using namespace std;

namespace g
{
  constexpr uint32_t screenWidth = 600;
  constexpr uint32_t screenHeight = 380;
};

using float4x4 = std::array<std::array<float, 4>, 4>;

struct float4 { float x, y, z, w; };
struct float3 { float x, y, z; };
struct float2 { float x, y; };

// Utils methods
float toRad(float deg) { return deg * PI / 180.0f; }
float dot(const float4& v1, const float4& v2) { return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w; }
float dot(const float3& v1, const float3& v2) { return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z; }

float3 cross(const float3& v1, const float3& v2)
{
  return { v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.x };
}

float3 normalize(const float3& v1)
{
  float length = std::sqrt(v1.x * v1.x + v1.y * v1.y + v1.z * v1.z);

  float3 normalized =
  {
    v1.x / length,
    v1.y / length,
    v1.z / length,
  };

  return normalized;
}

// Operator overloading

float3 operator-(const float3 &v1) { return { -v1.x, -v1.y, -v1.z }; }
float3 operator-(const float3 &v1, const float3 &v2)
{
  float3 difference =
  {
    v1.x - v2.x,
    v1.y - v2.y,
    v1.z - v2.z,
  };

  return difference;
}

float4x4 operator*(const float4x4& m1, const float4x4& m2)
{
  const float4 row_11 = { m1[0][0], m1[0][1], m1[0][2], m1[0][3] };
  const float4 row_21 = { m1[1][0], m1[1][1], m1[1][2], m1[1][3] };
  const float4 row_31 = { m1[2][0], m1[2][1], m1[2][2], m1[2][3] };
  const float4 row_41 = { m1[3][0], m1[3][1], m1[3][2], m1[3][3] };

  const float4 col_12 = { m2[0][0], m2[1][0], m2[2][0], m2[3][0] };
  const float4 col_22 = { m2[0][1], m2[1][1], m2[2][1], m2[3][1] };
  const float4 col_32 = { m2[0][2], m2[1][2], m2[2][2], m2[3][2] };
  const float4 col_42 = { m2[0][3], m2[1][3], m2[2][3], m2[3][3] };

  float4x4 mul =
  {{
    {{ dot(row_11, col_12), dot(row_11, col_22), dot(row_11, col_32), dot(row_11, col_42) }},
    {{ dot(row_21, col_12), dot(row_21, col_22), dot(row_21, col_32), dot(row_21, col_42) }},
    {{ dot(row_31, col_12), dot(row_31, col_22), dot(row_31, col_32), dot(row_31, col_42) }},
    {{ dot(row_41, col_12), dot(row_41, col_22), dot(row_41, col_32), dot(row_41, col_42) }},
  }};

  return mul;
}

float4 operator*(const float4x4& m1, const float4& v1)
{
  const float4 row_11 = { m1[0][0], m1[0][1], m1[0][2], m1[0][3] };
  const float4 row_21 = { m1[1][0], m1[1][1], m1[1][2], m1[1][3] };
  const float4 row_31 = { m1[2][0], m1[2][1], m1[2][2], m1[2][3] };
  const float4 row_41 = { m1[3][0], m1[3][1], m1[3][2], m1[3][3] };

  float4 mul =
  {
    dot(row_11, v1),
    dot(row_21, v1),
    dot(row_31, v1),
    dot(row_41, v1),
  };

  return mul;
}

float2& operator-=(float2& v1, const float2& v2)
{
  v1.x = v1.x - v2.x;
  v1.y = v1.y - v2.y;

  return v1;
}

float2 operator+(const float2& v1, const float s1) { return {v1.x + s1, v1.y + s1}; }
float2 operator+(const float2& v1, const float2& v2) { return {v1.x + v2.x, v1.y + v2.y}; }

class MatrixDemo : public tDX::PixelGameEngine
{
public:
  MatrixDemo()
  {
    sAppName = "3D matrix demo";
  }

  bool OnUserCreate() override
  {
    return true;
  }

  bool OnUserUpdate(float fElapsedTime) override
  {
    Clear(tDX::BLACK);

    // Keyboard control
    const float coeficient = 2.0f * fElapsedTime;

    if (GetKey(tDX::D).bHeld) { m_cubeTranslationX += coeficient; }
    if (GetKey(tDX::A).bHeld) { m_cubeTranslationX -= coeficient; }
    if (GetKey(tDX::W).bHeld) { m_cubeTranslationZ -= coeficient; }
    if (GetKey(tDX::S).bHeld) { m_cubeTranslationZ += coeficient; }
    if (GetKey(tDX::E).bHeld) { m_yaw += coeficient * 30; }
    if (GetKey(tDX::Q).bHeld) { m_yaw -= coeficient * 30; }

    m_cubeTranslationZ = std::max(m_cubeTranslationZ, -5.0f);
    m_cubeTranslationZ = std::min(m_cubeTranslationZ, -1.0f);

    m_cubeTranslationX = std::max(m_cubeTranslationX, -5.0f);
    m_cubeTranslationX = std::min(m_cubeTranslationX, 4.5f);

    m_yaw = fmod(m_yaw, 360.0f);

    // Grid
    for (uint8_t row = 0; row < m_gridRows; row++)
      DrawLine(0, row * m_cellSize, m_windowWidth - 1, row * m_cellSize, tDX::VERY_DARK_GREY);

    for (uint8_t col = 0; col < m_gridCols; col++)
      DrawLine(col * m_cellSize, 0, col * m_cellSize, m_windowHeight - 1, tDX::VERY_DARK_GREY);

    // Axes
    DrawLine(0, m_originY, m_windowWidth - 1, m_originY, tDX::DARK_YELLOW);
    DrawLine(m_originX, 0, m_originX, m_windowHeight - 1, tDX::DARK_YELLOW);

    // 2D square
    float2 leftUp = {m_originX - m_cellSize + (m_cubeTranslationX * m_cellSize * 2), m_originY - m_cellSize + (m_cubeTranslationZ * m_cellSize * 2) };
    float size = m_cellSize * 2.0f;

    std::array<float2, 4> m_rectangle =
    {{
      {leftUp.x       , leftUp.y       },
      {leftUp.x + size, leftUp.y       },
      {leftUp.x + size, leftUp.y + size},
      {leftUp.x       , leftUp.y + size},
    }};

    float2 centerVertex = leftUp + m_cellSize;

    for (auto& vertex : m_rectangle)
    {
      vertex -= centerVertex;

      float2 rotatedVertex;
      rotatedVertex.x = vertex.x * cos(toRad(m_yaw)) - vertex.y * sin(toRad(m_yaw));
      rotatedVertex.y = vertex.x * sin(toRad(m_yaw)) + vertex.y * cos(toRad(m_yaw));

      //vertex.x = vertex.x * cos(toRad(m_yaw)) + vertex.y * sin(toRad(m_yaw));
      //vertex.y = (-vertex.x * sin(toRad(m_yaw))) + vertex.y * cos(toRad(m_yaw));

      vertex = rotatedVertex + centerVertex;
    }

    DrawCircle(std::round(centerVertex.x), std::round(centerVertex.y), 2, tDX::YELLOW);


    //DrawRect(std::round(leftUp.x), std::round(leftUp.y), m_cellSize * 2, m_cellSize * 2, tDX::RED);
    DrawLine(std::round(m_rectangle[0].x), std::round(m_rectangle[0].y), std::round(m_rectangle[1].x), std::round(m_rectangle[1].y), tDX::RED);
    DrawLine(std::round(m_rectangle[1].x), std::round(m_rectangle[1].y), std::round(m_rectangle[2].x), std::round(m_rectangle[2].y), tDX::RED);
    DrawLine(std::round(m_rectangle[2].x), std::round(m_rectangle[2].y), std::round(m_rectangle[3].x), std::round(m_rectangle[3].y), tDX::RED);
    DrawLine(std::round(m_rectangle[3].x), std::round(m_rectangle[3].y), std::round(m_rectangle[0].x), std::round(m_rectangle[0].y), tDX::RED);

    // Camera
    DrawRect(m_originX - 4, m_originY - 5 + 10, 8, 10, tDX::BLUE);
    DrawRect(m_originX - 2, m_originY - 10 + 10, 4, 4, tDX::BLUE);

    // Draw frustum
    float fovx = 2 * std::atan(std::tan(toRad(45.0f * 0.5)) * m_aspectRatio);
    float length = (std::tan(fovx/2.0f) * m_windowHeight);

    DrawLineClipped(std::round(m_originX), std::round(m_originY), std::round(m_originX - length), std::round(m_originY - m_windowHeight), {0, 0}, { m_windowWidth - 1, m_windowHeight - 1 }, tDX::BLUE);
    DrawLineClipped(std::round(m_originX), std::round(m_originY), std::round(m_originX + length), std::round(m_originY - m_windowHeight), { 0, 0 }, { m_windowWidth - 1, m_windowHeight - 1 }, tDX::BLUE);

    // World matrix
    m_translationMatrix =
    {{
      {{ 1, 0, 0, m_cubeTranslationX }},
      {{ 0, 1, 0, 0                  }},
      {{ 0, 0, 1, m_cubeTranslationZ }},
      {{ 0, 0, 0, 1                  }},
    }};

    m_rotationMatrix =
    {{
      {{ cos(toRad(m_yaw))     , 0, -std::sin(toRad(m_yaw)), 0 }},
      {{ 0                     , 1, 0                      , 0 }},
      {{ std::sin(toRad(m_yaw)), 0, std::cos(toRad(m_yaw)) , 0 }},
      {{ 0                     , 0, 0                      , 1 }},
    }};

    m_modelMatrix = m_translationMatrix * m_rotationMatrix;

    // View matrix
    float3 zaxis = normalize(m_target - m_eye);
    float3 xaxis = normalize(cross(zaxis, m_up));
    float3 yaxis = cross(xaxis, zaxis);

    zaxis = -zaxis;

    m_viewMatrix =
    {{
      {{ xaxis.x, xaxis.y, xaxis.z, -dot(xaxis, m_eye) }},
      {{ yaxis.x, yaxis.y, yaxis.z, -dot(yaxis, m_eye) }},
      {{ zaxis.x, zaxis.y, zaxis.z, -dot(zaxis, m_eye) }},
      {{ 0, 0, 0, 1 }}
    }};

    // Projection
    float fovY = 45.0f;
    float n = 0.1f;
    float f = 100.0f;
    float tan_fovY = std::tan(toRad(fovY/2.0f));

    m_projectionMatrix =
    {{
      {{ 1.0f/(m_aspectRatio*tan_fovY), 0              , 0           , 0              }},
      {{ 0                            , 1.0f / tan_fovY, 0           , 0              }},
      {{ 0                            , 0              , -(f+n)/(f-n), -(2*f*n)/(f-n) }},
      {{ 0                            , 0              , -1          , 0              }}
    }};

    m_mvpMatrix = m_projectionMatrix * m_viewMatrix * m_modelMatrix;

    // 3D view
    const int32_t originX3D = m_windowWidth / 2;
    const int32_t originY3D = m_windowHeight + m_windowHeight / 2;

    DrawLine(0, originY3D, m_windowWidth - 1, originY3D, tDX::DARK_YELLOW);
    DrawLine(originX3D, m_windowHeight, originX3D, m_windowHeight + m_windowHeight - 1, tDX::DARK_YELLOW);

    // Cube
    std::array<float4, 8> transformedCube = m_cube;

    for (auto& vertex : transformedCube)
    {
      vertex = m_mvpMatrix * vertex;

      if (vertex.w > 0.0f)
      {
        vertex.x = vertex.x / vertex.w;
        vertex.y = vertex.y / vertex.w;
        vertex.z = vertex.z / vertex.w;
      }

      // Viewport
      vertex.x = (vertex.x + 1.0f) * (m_windowWidth - 1) * 0.5f + 0.0f; // plus X viewport origin
      vertex.y = (1.0f - vertex.y) * (m_windowHeight - 1) * 0.5f + m_windowHeight; // plus Y viewport origin
    }

    tDX::vi2d clipWinPos = { 0, m_windowHeight };
    tDX::vi2d clipWinSize = { m_windowWidth - 1, m_windowHeight - 1 };

    DrawLineClipped(std::round(transformedCube[0].x), std::round(transformedCube[0].y), std::round(transformedCube[1].x), std::round(transformedCube[1].y), clipWinPos, clipWinSize, tDX::WHITE);
    DrawLineClipped(std::round(transformedCube[1].x), std::round(transformedCube[1].y), std::round(transformedCube[2].x), std::round(transformedCube[2].y), clipWinPos, clipWinSize, tDX::WHITE);
    DrawLineClipped(std::round(transformedCube[2].x), std::round(transformedCube[2].y), std::round(transformedCube[3].x), std::round(transformedCube[3].y), clipWinPos, clipWinSize, tDX::WHITE);
    DrawLineClipped(std::round(transformedCube[3].x), std::round(transformedCube[3].y), std::round(transformedCube[0].x), std::round(transformedCube[0].y), clipWinPos, clipWinSize, tDX::WHITE);

    DrawLineClipped(std::round(transformedCube[4].x), std::round(transformedCube[4].y), std::round(transformedCube[5].x), std::round(transformedCube[5].y), clipWinPos, clipWinSize, tDX::WHITE);
    DrawLineClipped(std::round(transformedCube[5].x), std::round(transformedCube[5].y), std::round(transformedCube[6].x), std::round(transformedCube[6].y), clipWinPos, clipWinSize, tDX::WHITE);
    DrawLineClipped(std::round(transformedCube[6].x), std::round(transformedCube[6].y), std::round(transformedCube[7].x), std::round(transformedCube[7].y), clipWinPos, clipWinSize, tDX::WHITE);
    DrawLineClipped(std::round(transformedCube[7].x), std::round(transformedCube[7].y), std::round(transformedCube[4].x), std::round(transformedCube[4].y), clipWinPos, clipWinSize, tDX::WHITE);

    DrawLineClipped(std::round(transformedCube[0].x), std::round(transformedCube[0].y), std::round(transformedCube[4].x), std::round(transformedCube[4].y), clipWinPos, clipWinSize, tDX::WHITE);
    DrawLineClipped(std::round(transformedCube[1].x), std::round(transformedCube[1].y), std::round(transformedCube[5].x), std::round(transformedCube[5].y), clipWinPos, clipWinSize, tDX::WHITE);
    DrawLineClipped(std::round(transformedCube[2].x), std::round(transformedCube[2].y), std::round(transformedCube[6].x), std::round(transformedCube[6].y), clipWinPos, clipWinSize, tDX::WHITE);
    DrawLineClipped(std::round(transformedCube[3].x), std::round(transformedCube[3].y), std::round(transformedCube[7].x), std::round(transformedCube[7].y), clipWinPos, clipWinSize, tDX::WHITE);

    //DrawCircle(std::round(transformedCube[0].x), std::round(transformedCube[0].y), 2, tDX::YELLOW);

    // Windows borders
    DrawRect(0, 0, m_windowWidth - 1, m_windowHeight - 1, tDX::WHITE);
    DrawRect(0, m_windowHeight, m_windowWidth - 1, m_windowHeight - 1, tDX::WHITE);

    // Print matrices
    float4 worldVertex = m_modelMatrix * m_cube[0];
    float4 viewVertex = m_viewMatrix * worldVertex;
    float4 projVertex = m_projectionMatrix * viewVertex;

    std::stringstream modelMatrixToPrint;

    modelMatrixToPrint << std::fixed << std::setprecision(2) <<
      std::setw(6) << m_modelMatrix[0][0] << std::setw(6) << m_modelMatrix[0][1] << std::setw(6) << m_modelMatrix[0][2] << std::setw(6) << m_modelMatrix[0][3] << "    |" << std::setw(5) << worldVertex.x << "|" << '\n' <<
      std::setw(6) << m_modelMatrix[1][0] << std::setw(6) << m_modelMatrix[1][1] << std::setw(6) << m_modelMatrix[1][2] << std::setw(6) << m_modelMatrix[1][3] << "    |" << std::setw(5) << worldVertex.y << "|" << '\n' <<
      std::setw(6) << m_modelMatrix[2][0] << std::setw(6) << m_modelMatrix[2][1] << std::setw(6) << m_modelMatrix[2][2] << std::setw(6) << m_modelMatrix[2][3] << "    |" << std::setw(5) << worldVertex.z << "|" << '\n' <<
      std::setw(6) << m_modelMatrix[3][0] << std::setw(6) << m_modelMatrix[3][1] << std::setw(6) << m_modelMatrix[3][2] << std::setw(6) << m_modelMatrix[3][3] << "    |" << std::setw(5) << worldVertex.w << "|" << '\n';

    DrawString(310, 10, "Model to world");
    DrawString(300, 25, modelMatrixToPrint.str());

    std::stringstream lookAtToPrint;

    lookAtToPrint << std::fixed << std::setprecision(2) <<
      "  Eye    " << std::setw(6) << m_eye.x << std::setw(6) << m_eye.y << std::setw(6) << m_eye.z << '\n' <<
      "  Target " << std::setw(6) << m_target.x << std::setw(6) << m_target.y << std::setw(6) << m_target.z << '\n' <<
      "  Up     " << std::setw(6) << m_up.x << std::setw(6) << m_up.y << std::setw(6) << m_up.z << '\n';

    DrawString(310, 70, "LookAt input data", tDX::GREY);
    DrawString(300, 85, lookAtToPrint.str(), tDX::GREY);

    std::stringstream viewMatrixToPrint;

    viewMatrixToPrint << std::fixed << std::setprecision(2) <<
      std::setw(6) << m_viewMatrix[0][0] << std::setw(6) << m_viewMatrix[0][1] << std::setw(6) << m_viewMatrix[0][2] << std::setw(6) << m_viewMatrix[0][3] << "    |" << std::setw(5) << viewVertex.x << "|" << '\n' <<
      std::setw(6) << m_viewMatrix[1][0] << std::setw(6) << m_viewMatrix[1][1] << std::setw(6) << m_viewMatrix[1][2] << std::setw(6) << m_viewMatrix[1][3] << "    |" << std::setw(5) << viewVertex.y << "|" << '\n' <<
      std::setw(6) << m_viewMatrix[2][0] << std::setw(6) << m_viewMatrix[2][1] << std::setw(6) << m_viewMatrix[2][2] << std::setw(6) << m_viewMatrix[2][3] << "    |" << std::setw(5) << viewVertex.z << "|" << '\n' <<
      std::setw(6) << m_viewMatrix[3][0] << std::setw(6) << m_viewMatrix[3][1] << std::setw(6) << m_viewMatrix[3][2] << std::setw(6) << m_viewMatrix[3][3] << "    |" << std::setw(5) << viewVertex.w << "|" << '\n';

    DrawString(310, 130, "world to View");
    DrawString(300, 145, viewMatrixToPrint.str());

    std::stringstream projectionMatrixToPrint;

    projectionMatrixToPrint << std::fixed << std::setprecision(2) <<
      std::setw(6) << m_projectionMatrix[0][0] << std::setw(6) << m_projectionMatrix[0][1] << std::setw(6) << m_projectionMatrix[0][2] << std::setw(6) << m_projectionMatrix[0][3] << "    |" << std::setw(5) << projVertex.x << "|" << '\n' <<
      std::setw(6) << m_projectionMatrix[1][0] << std::setw(6) << m_projectionMatrix[1][1] << std::setw(6) << m_projectionMatrix[1][2] << std::setw(6) << m_projectionMatrix[1][3] << "    |" << std::setw(5) << projVertex.y << "|" << '\n' <<
      std::setw(6) << m_projectionMatrix[2][0] << std::setw(6) << m_projectionMatrix[2][1] << std::setw(6) << m_projectionMatrix[2][2] << std::setw(6) << m_projectionMatrix[2][3] << "    |" << std::setw(5) << projVertex.z << "|" << '\n' <<
      std::setw(6) << m_projectionMatrix[3][0] << std::setw(6) << m_projectionMatrix[3][1] << std::setw(6) << m_projectionMatrix[3][2] << std::setw(6) << m_projectionMatrix[3][3] << "    |" << std::setw(5) << projVertex.w << "|" << '\n';

    DrawString(310, 190, "Projection");
    DrawString(300, 205, projectionMatrixToPrint.str());

    std::stringstream mvpMatrixToPrint;

    mvpMatrixToPrint << std::fixed << std::setprecision(2) <<
      std::setw(6) << m_mvpMatrix[0][0] << std::setw(6) << m_mvpMatrix[0][1] << std::setw(6) << m_mvpMatrix[0][2] << std::setw(6) << m_mvpMatrix[0][3] << '\n' <<
      std::setw(6) << m_mvpMatrix[1][0] << std::setw(6) << m_mvpMatrix[1][1] << std::setw(6) << m_mvpMatrix[1][2] << std::setw(6) << m_mvpMatrix[1][3] << '\n' <<
      std::setw(6) << m_mvpMatrix[2][0] << std::setw(6) << m_mvpMatrix[2][1] << std::setw(6) << m_mvpMatrix[2][2] << std::setw(6) << m_mvpMatrix[2][3] << '\n' <<
      std::setw(6) << m_mvpMatrix[3][0] << std::setw(6) << m_mvpMatrix[3][1] << std::setw(6) << m_mvpMatrix[3][2] << std::setw(6) << m_mvpMatrix[3][3] << '\n';

    DrawString(310, 250, "MVP");
    DrawString(300, 265, mvpMatrixToPrint.str());

    std::stringstream cubePointsPrint;

    cubePointsPrint << std::fixed << std::setprecision(1) <<
      std::setw(7) << transformedCube[0].x << std::setw(7) << transformedCube[0].y << std::setw(5) << transformedCube[0].z << std::setw(5) << transformedCube[0].w << '\n' <<
      std::setw(7) << transformedCube[1].x << std::setw(7) << transformedCube[1].y << std::setw(5) << transformedCube[1].z << std::setw(5) << transformedCube[1].w << '\n' <<
      std::setw(7) << transformedCube[2].x << std::setw(7) << transformedCube[2].y << std::setw(5) << transformedCube[2].z << std::setw(5) << transformedCube[2].w << '\n' <<
      std::setw(7) << transformedCube[3].x << std::setw(7) << transformedCube[3].y << std::setw(5) << transformedCube[3].z << std::setw(5) << transformedCube[3].w << '\n';

    DrawString(310, 310, "Cube vertices");
    DrawString(300, 325, cubePointsPrint.str());

    return true;
  }

private:
  // Constants to specify UI
  constexpr static int32_t m_windowWidth = g::screenWidth / 2;
  constexpr static int32_t m_windowHeight = g::screenHeight / 2;
  constexpr static int32_t m_originX = 156;
  constexpr static int32_t m_originY = 156;
  constexpr static int32_t m_cellSize = m_windowHeight / 14;
  constexpr static uint8_t m_gridRows = m_windowHeight / m_cellSize + 1;
  constexpr static uint8_t m_gridCols = m_windowWidth / m_cellSize + 1;

  constexpr static float m_aspectRatio = (float)m_windowWidth / (float)m_windowHeight;

  // Model
  constexpr static std::array<float4, 8> m_cube =
  {{
    {-0.5, -0.5, -0.5, 1.0 },
    { 0.5, -0.5, -0.5, 1.0 },
    { 0.5,  0.5, -0.5, 1.0 },
    {-0.5,  0.5, -0.5, 1.0 },
    {-0.5, -0.5,  0.5, 1.0 },
    { 0.5, -0.5,  0.5, 1.0 },
    { 0.5,  0.5,  0.5, 1.0 },
    {-0.5,  0.5,  0.5, 1.0 }
  }};

  // Default matrix
  constexpr static float4x4 m_identityMatrix =
  {{
    {{ 1, 0, 0, 0 }},
    {{ 0, 1, 0, 0 }},
    {{ 0, 0, 1, 0 }},
    {{ 0, 0, 0, 1 }}
  }};

  // Cube transformations
  float m_cubeTranslationX = 0.0f;
  float m_cubeTranslationZ = -2.0f;
  float m_yaw = 0;

  // Matrices to describe a scene
  float4x4 m_translationMatrix;
  float4x4 m_rotationMatrix;
  float4x4 m_modelMatrix;

  float4x4 m_viewMatrix;
  float4x4 m_projectionMatrix;

  float4x4 m_mvpMatrix;

  // Look at
  float3 m_eye = { 0, 0, 0 };
  float3 m_target = { 0, 0, -1 };
  float3 m_up = { 0, 1, 0 };
};


int main()
{
  MatrixDemo demo;
  if (demo.Construct(g::screenWidth, g::screenHeight, 2, 2))
    demo.Start();

  return 0;
}
