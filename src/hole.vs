varying float alpha;
varying vec4 diffuse,ambient;
varying vec3 normal,halfVector;
 
void main()
{
    /* first transform the normal into eye space and
    normalize the result */
    normal = normalize(gl_NormalMatrix * gl_Normal);
 
    /* pass the halfVector to the fragment shader */
    halfVector = gl_LightSource[0].halfVector.xyz;
 
    alpha = gl_FrontMaterial.diffuse.w;
    /* Compute the diffuse, ambient and globalAmbient terms */
    diffuse = gl_FrontMaterial.diffuse * gl_LightSource[0].diffuse;
    ambient = gl_FrontMaterial.ambient * gl_LightSource[0].ambient;
    ambient += gl_LightModel.ambient * gl_FrontMaterial.ambient;
    gl_Position = ftransform();
 
}

