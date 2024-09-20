attribute vec4 verIn;
attribute vec2 texIn;
varying   vec2 texOut;
uniform   mat4 transform;
void main(){
    gl_Position = transform * verIn;
    texOut      = texIn;
}
