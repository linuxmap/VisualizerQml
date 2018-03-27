
attribute vec4 vertexIn;
attribute vec2 textureIn;
varying vec2 textureOut;
uniform highp mat4 matrix;

void main(void)           
{
    gl_Position = vertexIn;
    textureOut = textureIn;
}