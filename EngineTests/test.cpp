#include <execution>

#include "../Dependencies/assimp64/include/assimp/MathFunctions.h"
#include "Source/Core/PhysicsEngine/IntegrationMethods/euler_integrator.cpp"
#include "gtest/gtest.h"
#include <glm.hpp>
#include "Source/Core/Scene/Object3D.cpp"
#include "Source/Core/Scene/Scene.cpp"

#define EULER_CONSTANT 2.7182818284590452353602874713527

TEST(TestCaseName, TestName)
{
  EXPECT_EQ(1, 1);
}

TEST(Integration, TestEuler)
{
  euler_integrator *integrator = new euler_integrator();
  glm::vec3 current = glm::vec3(1, 0, 0);
  float step_size = 0.000005f;
  float x = 0;
  float result = 0;
  while (x <= 1)
  {
    current = integrator->integrate(current,current,step_size);
    x+=step_size;
  }
  EXPECT_NEAR(EULER_CONSTANT , current.x , 0.1);
  
}
