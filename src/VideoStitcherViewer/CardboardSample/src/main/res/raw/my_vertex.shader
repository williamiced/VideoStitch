uniform mat4 uMVPMatrix;
uniform vec4 uOffset;
attribute vec4 vPosition;
attribute vec2 vTexCoord0;
varying vec2 vTexCoord;
void main()
{
    gl_Position = uMVPMatrix * (vPosition + uOffset);
    vTexCoord = vTexCoord0;
}