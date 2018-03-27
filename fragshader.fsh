varying vec2 textureOut;
uniform sampler2D tex_y;
uniform sampler2D tex_u;
uniform lowp float opacity;
uniform int format;
void main(void) 
{
    vec3 yuv;
    vec3 rgb;
   // if(format==0)
   // {
        //case 0: //AV_PIX_FMT_YUV420P
        //case 12://AV_PIX_FMT_YUVJ420P
        //case 13://AV_PIX_FMT_YUVJ422P
        //case 25://AV_PIX_FMT_NV12
           // yuv.x = texture2D(tex_y, textureOut).r;
            //yuv.y = texture2D(tex_u, textureOut).r -0.5;
           // yuv.z = texture2D(tex_u, textureOut).g -0.5;
            //break;
   // }
   // else if(format==1)// case 1://AV_PIX_FMT_YUYV422
    {
        yuv.x = texture2D(tex_y, textureOut).r;
        yuv.y = texture2D(tex_u, textureOut).g-0.5;
        yuv.z = texture2D(tex_u, textureOut).a-0.5;

    }
    rgb = mat3( 1,       1,         1, 
                    0,       -0.3455,  1.779, 
                    1.4075, -0.7169,  0) * yuv;
    gl_FragColor = vec4(rgb, opacity);
}