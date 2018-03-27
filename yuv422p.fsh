varying vec2 textureOut;
uniform sampler2D tex_y;
uniform sampler2D tex_u;
uniform sampler2D tex_v;
//uniform lowp float opacity;
void main(void) 
{
    vec3 yuv;
    vec3 rgb;
    yuv.x = texture2D(tex_y, textureOut).r;
    yuv.y = texture2D(tex_u, textureOut).r-0.5;
    yuv.z = texture2D(tex_v, textureOut).r-0.5;
    rgb = mat3( 1,       1,         1, 
                    0,       -0.3455,  1.779, 
                    1.4075, -0.7169,  0) * yuv;
    gl_FragColor = vec4(rgb, 1);
}