#version 330

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 velocity;
layout (location = 2) in vec3 color;
layout (location = 3) in float lifeTime;
layout (location = 4) in float size;
layout (location = 5) in int type;

out vec3 thePosition;
out vec3 theVelocity;
out vec3 theColor;
out float theLifeTime;
out float theSize;
out int theType;

void main()
{
  thePosition = position;
  theVelocity = velocity;
  theColor = color;
  theLifeTime = lifeTime;
  theSize = size;
  theType = type;
}