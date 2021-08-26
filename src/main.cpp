#include <array>
#include <iomanip>
#include <sstream>

#define _USE_MATH_DEFINES
#include <cmath>

#define T_PGE_APPLICATION
#include "engine/tPixelGameEngine.h"

using namespace std;

namespace g
{
  constexpr uint32_t screenWidth = 600;
  constexpr uint32_t screenHeight = 380;
};

using floatT4x4 = std::array<std::array<float, 4>, 4>;
using floatT4 = std::array<float, 4>;
using floatT3 = std::array<float, 3>;

// Utils methods
float toRad(float deg) { return deg * (float)M_PI / 180.0f; }
float dot(const floatT4& v1, const floatT4& v2) { return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2] + v1[3] * v2[3]; }
float dot(const floatT3& v1, const floatT3& v2) { return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2]; }

floatT3 cross(const floatT3& v1, const floatT3& v2)
{
  return { v1[1]*v2[2] - v1[2]*v2[1], v1[2]*v2[0] - v1[0]*v2[2], v1[0]*v2[1] - v1[1]*v2[0] };
}

floatT3 normalize(const floatT3& v1)
{
  float length = std::sqrt(v1[0]*v1[0] + v1[1] * v1[1] + v1[2] * v1[2]);

  floatT3 normalized =
  {
    v1[0] / length,
    v1[1] / length,
    v1[2] / length,
  };

  return normalized;
}

// Operator overloading
floatT3 operator-(const floatT3 &v1) { return { -v1[0], -v1[1], -v1[2] }; }
floatT3 operator-(const floatT3 &v1, const floatT3 &v2)
{
  floatT3 difference =
  {
    v1[0] - v2[0],
    v1[1] - v2[1],
    v1[2] - v2[2],
  };

  return difference;
}

floatT4x4 operator*(const floatT4x4& m1, const floatT4x4& m2)
{
  const floatT4& row_11 = m1[0];
  const floatT4& row_21 = m1[1];
  const floatT4& row_31 = m1[2];
  const floatT4& row_41 = m1[3];

  const floatT4 col_12 = { m2[0][0], m2[1][0], m2[2][0], m2[3][0] };
  const floatT4 col_22 = { m2[0][1], m2[1][1], m2[2][1], m2[3][1] };
  const floatT4 col_32 = { m2[0][2], m2[1][2], m2[2][2], m2[3][2] };
  const floatT4 col_42 = { m2[0][3], m2[1][3], m2[2][3], m2[3][3] };

  floatT4x4 mul =
  {{
    {{ dot(row_11, col_12), dot(row_11, col_22), dot(row_11, col_32), dot(row_11, col_42) }},
    {{ dot(row_21, col_12), dot(row_21, col_22), dot(row_21, col_32), dot(row_21, col_42) }},
    {{ dot(row_31, col_12), dot(row_31, col_22), dot(row_31, col_32), dot(row_31, col_42) }},
    {{ dot(row_41, col_12), dot(row_41, col_22), dot(row_41, col_32), dot(row_41, col_42) }},
  }};

  return mul;
}

floatT4 operator*(const floatT4x4& m1, const floatT4& v1)
{
  const floatT4& row_11 = m1[0];
  const floatT4& row_21 = m1[1];
  const floatT4& row_31 = m1[2];
  const floatT4& row_41 = m1[3];

  floatT4 mul =
  {{
    { dot(row_11, v1) },
    { dot(row_21, v1) },
    { dot(row_31, v1) },
    { dot(row_41, v1) },
  }};

  return mul;
}

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
    if (GetKey(tDX::E).bHeld) { m_yaw -= coeficient * 30; }
    if (GetKey(tDX::Q).bHeld) { m_yaw += coeficient * 30; }

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
    floatT3 zaxis = normalize(m_target - m_eye);
    floatT3 xaxis = normalize(cross(zaxis, m_up));
    floatT3 yaxis = cross(xaxis, zaxis);

    zaxis = -zaxis;

    m_viewMatrix =
    {{
      {{ xaxis[0], xaxis[1], xaxis[2], -dot(xaxis, m_eye) }},
      {{ yaxis[0], yaxis[1], yaxis[2], -dot(yaxis, m_eye) }},
      {{ zaxis[0], zaxis[1], zaxis[2], -dot(zaxis, m_eye) }},
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
    std::array<floatT4, 8> transformedCube = m_cube;

    for (auto& vertex : transformedCube)
    {
      vertex = m_mvpMatrix * vertex;

      if (vertex[3] > 0.0f)
      {
        vertex[0] = vertex[0] / vertex[3];
        vertex[1] = vertex[1] / vertex[3];
        vertex[2] = vertex[2] / vertex[3];
      }
      else
      {
        vertex[0] = vertex[0];
        vertex[1] = vertex[1];
        vertex[2] = vertex[2];
      }

      // Viewport
      vertex[0] = (vertex[0] + 1.0f) * (m_windowWidth - 1) * 0.5f + 0.0f; // plus X viewport origin
      vertex[1] = (1.0f - vertex[1]) * (m_windowHeight - 1) * 0.5f + m_windowHeight; // plus Y viewport origin
    }

    tDX::vi2d clipWinPos = { 0, m_windowHeight };
    tDX::vi2d clipWinSize = { m_windowWidth - 1, m_windowHeight - 1 };

    DrawLineClipped(std::round(transformedCube[0][0]), std::round(transformedCube[0][1]), std::round(transformedCube[1][0]), std::round(transformedCube[1][1]), clipWinPos, clipWinSize, tDX::WHITE);
    DrawLineClipped(std::round(transformedCube[1][0]), std::round(transformedCube[1][1]), std::round(transformedCube[2][0]), std::round(transformedCube[2][1]), clipWinPos, clipWinSize, tDX::WHITE);
    DrawLineClipped(std::round(transformedCube[2][0]), std::round(transformedCube[2][1]), std::round(transformedCube[3][0]), std::round(transformedCube[3][1]), clipWinPos, clipWinSize, tDX::WHITE);
    DrawLineClipped(std::round(transformedCube[3][0]), std::round(transformedCube[3][1]), std::round(transformedCube[0][0]), std::round(transformedCube[0][1]), clipWinPos, clipWinSize, tDX::WHITE);

    DrawLineClipped(std::round(transformedCube[4][0]), std::round(transformedCube[4][1]), std::round(transformedCube[5][0]), std::round(transformedCube[5][1]), clipWinPos, clipWinSize, tDX::WHITE);
    DrawLineClipped(std::round(transformedCube[5][0]), std::round(transformedCube[5][1]), std::round(transformedCube[6][0]), std::round(transformedCube[6][1]), clipWinPos, clipWinSize, tDX::WHITE);
    DrawLineClipped(std::round(transformedCube[6][0]), std::round(transformedCube[6][1]), std::round(transformedCube[7][0]), std::round(transformedCube[7][1]), clipWinPos, clipWinSize, tDX::WHITE);
    DrawLineClipped(std::round(transformedCube[7][0]), std::round(transformedCube[7][1]), std::round(transformedCube[4][0]), std::round(transformedCube[4][1]), clipWinPos, clipWinSize, tDX::WHITE);

    DrawLineClipped(std::round(transformedCube[0][0]), std::round(transformedCube[0][1]), std::round(transformedCube[4][0]), std::round(transformedCube[4][1]), clipWinPos, clipWinSize, tDX::WHITE);
    DrawLineClipped(std::round(transformedCube[1][0]), std::round(transformedCube[1][1]), std::round(transformedCube[5][0]), std::round(transformedCube[5][1]), clipWinPos, clipWinSize, tDX::WHITE);
    DrawLineClipped(std::round(transformedCube[2][0]), std::round(transformedCube[2][1]), std::round(transformedCube[6][0]), std::round(transformedCube[6][1]), clipWinPos, clipWinSize, tDX::WHITE);
    DrawLineClipped(std::round(transformedCube[3][0]), std::round(transformedCube[3][1]), std::round(transformedCube[7][0]), std::round(transformedCube[7][1]), clipWinPos, clipWinSize, tDX::WHITE);

    //DrawCircle(std::round(transformedCube[0].x), std::round(transformedCube[0].y), 2, tDX::YELLOW);

    // Windows borders
    DrawRect(0, 0, m_windowWidth - 1, m_windowHeight - 1, tDX::WHITE);
    DrawRect(0, m_windowHeight, m_windowWidth - 1, m_windowHeight - 1, tDX::WHITE);

    // Print matrices
    floatT4 worldVertex = m_modelMatrix * m_cube[0];
    floatT4 viewVertex = m_viewMatrix * worldVertex;
    floatT4 projVertex = m_projectionMatrix * viewVertex;

    std::stringstream modelMatrixToPrint;

    modelMatrixToPrint << std::fixed << std::setprecision(2) <<
      std::setw(6) << m_modelMatrix[0][0] << std::setw(6) << m_modelMatrix[0][1] << std::setw(6) << m_modelMatrix[0][2] << std::setw(6) << m_modelMatrix[0][3] << "    |" << std::setw(5) << worldVertex[0] << "|" << '\n' <<
      std::setw(6) << m_modelMatrix[1][0] << std::setw(6) << m_modelMatrix[1][1] << std::setw(6) << m_modelMatrix[1][2] << std::setw(6) << m_modelMatrix[1][3] << "    |" << std::setw(5) << worldVertex[1] << "|" << '\n' <<
      std::setw(6) << m_modelMatrix[2][0] << std::setw(6) << m_modelMatrix[2][1] << std::setw(6) << m_modelMatrix[2][2] << std::setw(6) << m_modelMatrix[2][3] << "    |" << std::setw(5) << worldVertex[2] << "|" << '\n' <<
      std::setw(6) << m_modelMatrix[3][0] << std::setw(6) << m_modelMatrix[3][1] << std::setw(6) << m_modelMatrix[3][2] << std::setw(6) << m_modelMatrix[3][3] << "    |" << std::setw(5) << worldVertex[3] << "|" << '\n';

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
      std::setw(6) << m_viewMatrix[0][0] << std::setw(6) << m_viewMatrix[0][1] << std::setw(6) << m_viewMatrix[0][2] << std::setw(6) << m_viewMatrix[0][3] << "    |" << std::setw(5) << viewVertex[0] << "|" << '\n' <<
      std::setw(6) << m_viewMatrix[1][0] << std::setw(6) << m_viewMatrix[1][1] << std::setw(6) << m_viewMatrix[1][2] << std::setw(6) << m_viewMatrix[1][3] << "    |" << std::setw(5) << viewVertex[1] << "|" << '\n' <<
      std::setw(6) << m_viewMatrix[2][0] << std::setw(6) << m_viewMatrix[2][1] << std::setw(6) << m_viewMatrix[2][2] << std::setw(6) << m_viewMatrix[2][3] << "    |" << std::setw(5) << viewVertex[2] << "|" << '\n' <<
      std::setw(6) << m_viewMatrix[3][0] << std::setw(6) << m_viewMatrix[3][1] << std::setw(6) << m_viewMatrix[3][2] << std::setw(6) << m_viewMatrix[3][3] << "    |" << std::setw(5) << viewVertex[3] << "|" << '\n';

    DrawString(310, 130, "world to View");
    DrawString(300, 145, viewMatrixToPrint.str());

    std::stringstream projectionMatrixToPrint;

    projectionMatrixToPrint << std::fixed << std::setprecision(2) <<
      std::setw(6) << m_projectionMatrix[0][0] << std::setw(6) << m_projectionMatrix[0][1] << std::setw(6) << m_projectionMatrix[0][2] << std::setw(6) << m_projectionMatrix[0][3] << "    |" << std::setw(5) << projVertex[0] << "|" << '\n' <<
      std::setw(6) << m_projectionMatrix[1][0] << std::setw(6) << m_projectionMatrix[1][1] << std::setw(6) << m_projectionMatrix[1][2] << std::setw(6) << m_projectionMatrix[1][3] << "    |" << std::setw(5) << projVertex[1] << "|" << '\n' <<
      std::setw(6) << m_projectionMatrix[2][0] << std::setw(6) << m_projectionMatrix[2][1] << std::setw(6) << m_projectionMatrix[2][2] << std::setw(6) << m_projectionMatrix[2][3] << "    |" << std::setw(5) << projVertex[2] << "|" << '\n' <<
      std::setw(6) << m_projectionMatrix[3][0] << std::setw(6) << m_projectionMatrix[3][1] << std::setw(6) << m_projectionMatrix[3][2] << std::setw(6) << m_projectionMatrix[3][3] << "    |" << std::setw(5) << projVertex[3] << "|" << '\n';

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
      std::setw(7) << transformedCube[0][0] << std::setw(7) << transformedCube[0][1] << std::setw(5) << transformedCube[0][2] << std::setw(5) << transformedCube[0][3] << '\n' <<
      std::setw(7) << transformedCube[1][0] << std::setw(7) << transformedCube[1][1] << std::setw(5) << transformedCube[1][2] << std::setw(5) << transformedCube[1][3] << '\n' <<
      std::setw(7) << transformedCube[2][0] << std::setw(7) << transformedCube[2][1] << std::setw(5) << transformedCube[2][2] << std::setw(5) << transformedCube[2][3] << '\n' <<
      std::setw(7) << transformedCube[3][0] << std::setw(7) << transformedCube[3][1] << std::setw(5) << transformedCube[3][2] << std::setw(5) << transformedCube[3][3] << '\n';

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
  constexpr static std::array<floatT4, 8> m_cube =
  {{
    {{-0.5, -0.5, -0.5, 1.0 }},
    {{ 0.5, -0.5, -0.5, 1.0 }},
    {{ 0.5,  0.5, -0.5, 1.0 }},
    {{-0.5,  0.5, -0.5, 1.0 }},
    {{-0.5, -0.5,  0.5, 1.0 }},
    {{ 0.5, -0.5,  0.5, 1.0 }},
    {{ 0.5,  0.5,  0.5, 1.0 }},
    {{-0.5,  0.5,  0.5, 1.0 }}
  }};

  // Default matrix
  constexpr static floatT4x4 m_identityMatrix =
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
  floatT4x4 m_translationMatrix;
  floatT4x4 m_rotationMatrix;
  floatT4x4 m_modelMatrix;

  floatT4x4 m_viewMatrix;
  floatT4x4 m_projectionMatrix;

  floatT4x4 m_mvpMatrix;

  // Look at
  floatT3 m_eye = { 0, 0, 0 };
  floatT3 m_target = { 0, 0, -1 };
  floatT3 m_up = { 0, 1, 0 };
};


int main()
{
  MatrixDemo demo;
  if (demo.Construct(g::screenWidth, g::screenHeight, 2, 2))
    demo.Start();

  return 0;
}
