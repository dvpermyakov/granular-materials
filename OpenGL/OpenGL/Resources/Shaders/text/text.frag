#version 330

uniform sampler2D gSampler;
uniform vec4 color;

in vec2 theTexCoord;

void main()
{
    vec4 texColor = texture2D(gSampler, theTexCoord);
    gl_FragColor = (texColor.r, texColor.r, texColor.r, texColor.r) * color;
} 