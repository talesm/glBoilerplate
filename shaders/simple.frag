#version 330 core
in vec2 ourTexCoord;
in vec4 ourColor;

out vec4 fragColor;
uniform sampler2D inputTex;   

void main()
{
    vec4 mainColor = texture(inputTex, ourTexCoord);
    fragColor      = mainColor * ourColor;
}
