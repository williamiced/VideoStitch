uniform mat4 u_MVPMatrix;
uniform mat4 u_MVMatrix;

attribute vec4 a_Position;
attribute vec4 a_Color;
attribute vec3 a_Normal;
attribute vec2 a_TexCoordinate;

varying vec3 v_Position;        // This will be passed into the fragment shader.
varying vec4 v_Color;           // This will be passed into the fragment shader.
varying vec3 v_Normal;          // This will be passed into the fragment shader.
varying vec2 v_TexCoordinate;

void main() {
   // Transform the vertex into eye space.
   v_Position = vec3(u_MVMatrix * a_Position);

   // Pass through the color.
   v_Color = a_Color;

   // Pass through the texture coordinate.
   v_TexCoordinate = a_TexCoordinate;

   // Transform the normal's orientation into eye space.
   v_Normal = vec3(u_MVMatrix * vec4(a_Normal, 0.0));

   // gl_Position is a special variable used to store the final position.
   // Multiply the vertex by the matrix to get the final point in normalized screen coordinates.
   gl_Position = u_MVPMatrix * a_Position;
}
