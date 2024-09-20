varying vec2        texOut;
uniform sampler2D   tex_y;
uniform sampler2D   tex_u;
uniform sampler2D   tex_v;

void main(){
    vec3 yuv;
    vec3 rgb;
    yuv.x = texture2D(tex_y, texOut).r      ;
    yuv.y = texture2D(tex_u, texOut).r - 0.5;
    yuv.z = texture2D(tex_v, texOut).r - 0.5;
    rgb = mat3( 1.0,        1.0   ,         1.0,
                  0,        -0.344,       1.772,
              1.402,        -0.714,         0.0) * yuv;
    gl_FragColor = vec4(rgb, 1.0);
}
