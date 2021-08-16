#include <array>
#include <iomanip>
#include <sstream>

#define _USE_MATH_DEFINES
#include <cmath>

#define T_PGE_APPLICATION
#include "engine/tPixelGameEngine.h"

#include "linalg.h"

/*
  NOTES
  - in lianlg, every row is column
*/

using namespace linalg::aliases;
using namespace std;

namespace g
{
  constexpr uint32_t screenWidth = 600;
  constexpr uint32_t screenHeight = 380;
};

using floatT4x4 = std::array<std::array<float, 4>, 4>;
using floatT4 = std::array<float, 4>;

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
    m_cubeTranslationZ = std::min(m_cubeTranslationZ, 0.5f);

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
    tDX::vf2d leftUp = {m_originX - m_cellSize + (m_cubeTranslationX * m_cellSize * 2) , m_originY - m_cellSize + (m_cubeTranslationZ * m_cellSize * 2) };

    DrawRect(std::round(leftUp.x), std::round(leftUp.y), m_cellSize * 2, m_cellSize * 2, tDX::RED);

    // Camera
    DrawRect(m_originX - 4, m_originY - 5 + 10, 8, 10, tDX::BLUE);
    DrawRect(m_originX - 2, m_originY - 10 + 10, 4, 4, tDX::BLUE);

    // Draw frustum
    float fovx = 2 * std::atan(std::tan(ToRad(45.0f * 0.5)) * m_aspectRatio);
    float length = (std::tan(fovx/2.0f) * 120);

    DrawLine(m_originX, m_originY, m_originX - length, m_originY - 120, tDX::BLUE);
    DrawLine(m_originX, m_originY, m_originX + length, m_originY - 120, tDX::BLUE);

    // World matrix
    /*
    modelMatrixCube =
    {
      { 1, 0, 0, 0 },
      { 0, 1, 0, 0 },
      { 0, 0, 1, 0 },
      { cubeTranslationX, 0, cubeTranslationZ, 1 }
    };
    */

    m_modelMatrixCube =
    {
      { std::cos(ToRad(m_yaw)), 0, std::sin(ToRad(m_yaw)), 0 },
      { 0, 1, 0, 0 },
      { -std::sin(ToRad(m_yaw)), 0, std::cos(ToRad(m_yaw)), 0 },
      { m_cubeTranslationX, 0, m_cubeTranslationZ, 1 }
    };

    m_translationMatrix =
    {{
      {{ 1, 0, 0, m_cubeTranslationX }},
      {{ 0, 1, 0, 0                  }},
      {{ 0, 0, 1, m_cubeTranslationZ }},
      {{ 0, 0, 0, 1                  }},
    }};

    m_rotationMatrix =
    {{
      {{ cos(ToRad(m_yaw))     , 0, -std::sin(ToRad(m_yaw)), 0 }},
      {{ 0                     , 1, 0                      , 0 }},
      {{ std::sin(ToRad(m_yaw)), 0, std::cos(ToRad(m_yaw)) , 0 }},
      {{ 0                     , 0, 0                      , 1 }},
    }};

    floatT4x4 TUCNA = MatMatMul(m_translationMatrix, m_rotationMatrix);

    // View matrix, Projection , matrix
    float4x4 testLOOKAT = linalg::identity;
    float4x4 testPROJECTION = linalg::perspective_matrix(ToRad(45.0f), m_aspectRatio, 0.1f, 100.0f);

    m_mvpMatrixCube = linalg::mul(testPROJECTION, testLOOKAT, m_modelMatrixCube);
    m_projectionMatrixCube = testPROJECTION;
    m_viewMatrixCube = testLOOKAT;

    // 3D view
    const int32_t originX3D = m_windowWidth / 2;
    const int32_t originY3D = m_windowHeight + m_windowHeight / 2;

    DrawLine(0, originY3D, m_windowWidth - 1, originY3D, tDX::DARK_YELLOW);
    DrawLine(originX3D, m_windowHeight, originX3D, m_windowHeight + m_windowHeight - 1, tDX::DARK_YELLOW);

    // Cube
    std::array<linalg::aliases::float4, 8> transformedCube = m_cube;

    for (auto& vertex : transformedCube)
    {
      vertex = linalg::mul(m_mvpMatrixCube, vertex);
      vertex.xyz() = vertex.w > 0.0f ? vertex.xyz() / vertex.w : vertex.xyz();

      // Viewport
      vertex.x = (vertex.x + 1.0f) * (m_windowWidth - 1) * 0.5f + 0.0f; // plus X viewport origin
      vertex.y = (1.0f - vertex.y) * (m_windowHeight - 1) * 0.5f + m_windowHeight; // plus Y viewport origin
    }

    std::cout << transformedCube[0].x << ", " << transformedCube[0].y << ", " << transformedCube[0].z << ", " << transformedCube[0].w << std::endl;

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
    float4 worldVertex = linalg::mul(m_modelMatrixCube, m_cube[0]);
    float4 viewVertex = linalg::mul(m_viewMatrixCube, worldVertex);
    float4 projVertex = linalg::mul(m_projectionMatrixCube, viewVertex);

    std::stringstream modelMatrixToPrint;

    modelMatrixToPrint << std::fixed << std::setprecision(2) <<
      std::setw(6) << m_modelMatrixCube[0][0] << std::setw(6) << m_modelMatrixCube[1][0] << std::setw(6) << m_modelMatrixCube[2][0] << std::setw(6) << m_modelMatrixCube[3][0] << "    |" << std::setw(5) << worldVertex.x << "|" << '\n' <<
      std::setw(6) << m_modelMatrixCube[0][1] << std::setw(6) << m_modelMatrixCube[1][1] << std::setw(6) << m_modelMatrixCube[2][1] << std::setw(6) << m_modelMatrixCube[3][1] << "    |" << std::setw(5) << worldVertex.y << "|" << '\n' <<
      std::setw(6) << m_modelMatrixCube[0][2] << std::setw(6) << m_modelMatrixCube[1][2] << std::setw(6) << m_modelMatrixCube[2][2] << std::setw(6) << m_modelMatrixCube[3][2] << "    |" << std::setw(5) << worldVertex.z << "|" << '\n' <<
      std::setw(6) << m_modelMatrixCube[0][3] << std::setw(6) << m_modelMatrixCube[1][3] << std::setw(6) << m_modelMatrixCube[2][3] << std::setw(6) << m_modelMatrixCube[3][3] << "    |" << std::setw(5) << worldVertex.w << "|" << '\n';

    DrawString(310, 10, "Model to world");
    DrawString(300, 25, modelMatrixToPrint.str());

    std::stringstream lookAtToPrint;

    lookAtToPrint << std::fixed << std::setprecision(2) <<
      "  Eye    " << std::setw(6) << m_eye[0] << std::setw(6) << m_eye[1] << std::setw(6) << m_eye[2] << '\n' <<
      "  Target " << std::setw(6) << m_target[0] << std::setw(6) << m_target[1] << std::setw(6) << m_target[2] << '\n' <<
      "  Up     " << std::setw(6) << m_up[0] << std::setw(6) << m_up[1] << std::setw(6) << m_up[2] << '\n';

    DrawString(310, 70, "LookAt input data", tDX::GREY);
    DrawString(300, 85, lookAtToPrint.str(), tDX::GREY);

    std::stringstream viewMatrixToPrint;

    viewMatrixToPrint << std::fixed << std::setprecision(2) <<
      std::setw(6) << m_viewMatrixCube[0][0] << std::setw(6) << m_viewMatrixCube[1][0] << std::setw(6) << m_viewMatrixCube[2][0] << std::setw(6) << m_viewMatrixCube[3][0] << "    |" << std::setw(5) << viewVertex.x << "|" << '\n' <<
      std::setw(6) << m_viewMatrixCube[0][1] << std::setw(6) << m_viewMatrixCube[1][1] << std::setw(6) << m_viewMatrixCube[2][1] << std::setw(6) << m_viewMatrixCube[3][1] << "    |" << std::setw(5) << viewVertex.y << "|" << '\n' <<
      std::setw(6) << m_viewMatrixCube[0][2] << std::setw(6) << m_viewMatrixCube[1][2] << std::setw(6) << m_viewMatrixCube[2][2] << std::setw(6) << m_viewMatrixCube[3][2] << "    |" << std::setw(5) << viewVertex.z << "|" << '\n' <<
      std::setw(6) << m_viewMatrixCube[0][3] << std::setw(6) << m_viewMatrixCube[1][3] << std::setw(6) << m_viewMatrixCube[2][3] << std::setw(6) << m_viewMatrixCube[3][3] << "    |" << std::setw(5) << viewVertex.w << "|" << '\n';

    DrawString(310, 130, "world to View");
    DrawString(300, 145, viewMatrixToPrint.str());

    std::stringstream projectionMatrixToPrint;

    projectionMatrixToPrint << std::fixed << std::setprecision(2) <<
      std::setw(6) << m_projectionMatrixCube[0][0] << std::setw(6) << m_projectionMatrixCube[1][0] << std::setw(6) << m_projectionMatrixCube[2][0] << std::setw(6) << m_projectionMatrixCube[3][0] << "    |" << std::setw(5) << projVertex.x << "|" << '\n' <<
      std::setw(6) << m_projectionMatrixCube[0][1] << std::setw(6) << m_projectionMatrixCube[1][1] << std::setw(6) << m_projectionMatrixCube[2][1] << std::setw(6) << m_projectionMatrixCube[3][1] << "    |" << std::setw(5) << projVertex.y << "|" << '\n' <<
      std::setw(6) << m_projectionMatrixCube[0][2] << std::setw(6) << m_projectionMatrixCube[1][2] << std::setw(6) << m_projectionMatrixCube[2][2] << std::setw(6) << m_projectionMatrixCube[3][2] << "    |" << std::setw(5) << projVertex.z << "|" << '\n' <<
      std::setw(6) << m_projectionMatrixCube[0][3] << std::setw(6) << m_projectionMatrixCube[1][3] << std::setw(6) << m_projectionMatrixCube[2][3] << std::setw(6) << m_projectionMatrixCube[3][3] << "    |" << std::setw(5) << projVertex.w << "|" << '\n';

    DrawString(310, 190, "Projection");
    DrawString(300, 205, projectionMatrixToPrint.str());

    std::stringstream mvpMatrixToPrint;

    mvpMatrixToPrint << std::fixed << std::setprecision(2) <<
      std::setw(6) << m_mvpMatrixCube[0][0] << std::setw(6) << m_mvpMatrixCube[1][0] << std::setw(6) << m_mvpMatrixCube[2][0] << std::setw(6) << m_mvpMatrixCube[3][0] << '\n' <<
      std::setw(6) << m_mvpMatrixCube[0][1] << std::setw(6) << m_mvpMatrixCube[1][1] << std::setw(6) << m_mvpMatrixCube[2][1] << std::setw(6) << m_mvpMatrixCube[3][1] << '\n' <<
      std::setw(6) << m_mvpMatrixCube[0][2] << std::setw(6) << m_mvpMatrixCube[1][2] << std::setw(6) << m_mvpMatrixCube[2][2] << std::setw(6) << m_mvpMatrixCube[3][2] << '\n' <<
      std::setw(6) << m_mvpMatrixCube[0][3] << std::setw(6) << m_mvpMatrixCube[1][3] << std::setw(6) << m_mvpMatrixCube[2][3] << std::setw(6) << m_mvpMatrixCube[3][3] << '\n';

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
  // Utils methods
  float ToRad(float deg) { return deg * (float)M_PI / 180.0f; }
  float Dot(const floatT4& v1, const floatT4& v2) { return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2] + v1[3] * v2[3]; }

  floatT4x4 MatMatMul(const floatT4x4& m1, const floatT4x4& m2)
  {
    floatT4x4 mul;

    const floatT4& row_11 = m1[0];
    const floatT4& row_21 = m1[1];
    const floatT4& row_31 = m1[2];
    const floatT4& row_41 = m1[3];

    const floatT4 col_12 = { m2[0][0], m2[1][0], m2[2][0], m2[3][0] };
    const floatT4 col_22 = { m2[0][1], m2[1][1], m2[2][1], m2[3][1] };
    const floatT4 col_32 = { m2[0][2], m2[1][2], m2[2][2], m2[3][2] };
    const floatT4 col_42 = { m2[0][3], m2[1][3], m2[2][3], m2[3][3] };

    mul =
    {{
      {{ Dot(col_12, row_11), Dot(col_22, row_11), Dot(col_32, row_11), Dot(col_42, row_11) }},
      {{ Dot(col_12, row_21), Dot(col_22, row_21), Dot(col_32, row_21), Dot(col_42, row_21) }},
      {{ Dot(col_12, row_31), Dot(col_22, row_31), Dot(col_32, row_31), Dot(col_42, row_31) }},
      {{ Dot(col_12, row_41), Dot(col_22, row_41), Dot(col_32, row_41), Dot(col_42, row_41) }},
    }};

    return mul;
  }

  // Constants to specify UI
  constexpr static int32_t m_windowWidth = g::screenWidth / 2;
  constexpr static int32_t m_windowHeight = g::screenHeight / 2;
  constexpr static int32_t m_originX = 156;
  constexpr static int32_t m_originY = 156;
  constexpr static int32_t m_cellSize = m_windowHeight / 14;
  constexpr static uint8_t m_gridRows = m_windowHeight / m_cellSize + 1;
  constexpr static uint8_t m_gridCols = m_windowWidth / m_cellSize + 1;
  constexpr static float m_aspectRatio = (float)m_windowWidth / (float)m_windowHeight;

  // Cube transformations
  float m_cubeTranslationX = 0.0f;
  float m_cubeTranslationZ = -2.0f;
  float m_yaw = 0;

  // Matrices to describe a scene
  float4x4 m_modelMatrixCube;
  float4x4 m_viewMatrixCube;
  float4x4 m_projectionMatrixCube;
  float4x4 m_mvpMatrixCube;

  // New matrices
  floatT4x4 m_translationMatrix;
  floatT4x4 m_rotationMatrix;

  // Look at
  float3 m_eye = { 0, 0, 0 };
  float3 m_target = { 0, 0, -1 };
  float3 m_up = { 0, 1, 0 };

  // Cube
  std::array<linalg::aliases::float4, 8> m_cube =
  {{
    {-0.5, -0.5, -0.5, 1.0},
    { 0.5, -0.5, -0.5, 1.0},
    { 0.5,  0.5, -0.5, 1.0},
    {-0.5,  0.5, -0.5, 1.0},
    {-0.5, -0.5,  0.5, 1.0},
    { 0.5, -0.5,  0.5, 1.0},
    { 0.5,  0.5,  0.5, 1.0},
    {-0.5,  0.5,  0.5, 1.0}
  }};
};


int main()
{
  MatrixDemo demo;
  if (demo.Construct(g::screenWidth, g::screenHeight, 2, 2))
    demo.Start();

  return 0;
}
