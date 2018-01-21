attribute vec3 vTangent;

varying vec3 n;
varying vec3 t;
varying vec3 b;
varying vec3 pos; // eye space

void main() {
  n = normalize(gl_NormalMatrix * gl_Normal);
  t = normalize(gl_NormalMatrix * vTangent);
  b = cross(n, t);
  
  pos = vec3(gl_ModelViewMatrix * gl_Vertex);
  
  gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
  gl_Position = ftransform();
}
