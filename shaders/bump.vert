#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 mainDirection;

out vec2 ourTexCoord;
out vec3 ourMainDirection;
out vec3 normal;
out vec3 tangent;
out vec3 binormal;

void main()
{
    vec4 vertPos = vec4(aPos, 1.0);
    gl_Position = projection * view * model * vertPos;
    ourMainDirection = (view * vec4(mainDirection, 1.0)).rgb;
    ourTexCoord = aTexCoord;
    
    mat4 modelView  = view * model;
    mat4 modelViewIt = transpose(inverse(modelView));

    normal = normalize((modelViewIt * vec4(0, 0, 1, 1)).xyz);
    tangent = normalize((modelViewIt * vec4(0, -1, 0, 1)).xyz);
    binormal = normalize((modelViewIt * vec4(-1, 0, 0, 1)).xyz);
}