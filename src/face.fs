varying vec3 n;
varying vec3 t;
varying vec3 b;
varying vec3 pos; // eye space

uniform sampler2D normal_tex;
uniform sampler2D ambient_tex;
uniform sampler2D diffuse_tex;
uniform sampler2D specular_tex;
uniform vec3 eye_pos;
 
void main()
{
  vec4 normal_map = texture2D(normal_tex, gl_TexCoord[0].st);
  vec4 ambient_refl = texture2D(ambient_tex, gl_TexCoord[0].st);
  vec4 diffuse_refl = texture2D(diffuse_tex, gl_TexCoord[0].st);
  vec4 specular_refl = texture2D(specular_tex, gl_TexCoord[0].st);

  // extract alpha and shininess from textures
  float alpha = diffuse_refl.w;
  float shininess = 255. * specular_refl.w;
  
  
  // move normals from normal map into eye coordinates
  vec3 t1 = normalize(t);
  vec3 n1 = normalize(n);
  vec3 b1 = cross(n1, t1);
  vec3 normal = normalize(normal_map.xyz - 0.5);
  vec3 n_temp = normal;
  normal.x = dot(n_temp, t1);
  normal.y = dot(n_temp, b1);
  normal.z = dot(n_temp, n1);

  /*
    // 4-light solution
  vec4 color = vec4(0., 0., 0., 0.);
  float factor = 0.25;
  for (int i=0; i<4; i++)
  {
    // calculate ambient component
    color += factor * gl_LightSource[i].ambient * ambient_refl;
    
    // calculate diffuse component
    vec3 light_dir = normalize(gl_LightSource[i].position.xyz - pos);
    
    float diffuse_factor = max(0., dot(normal, light_dir));
    color += factor * gl_LightSource[i].diffuse * diffuse_factor * diffuse_refl;
    
    // calculate specular component
    vec3 halfvec = normalize(light_dir - pos);
    
    float specular_factor = pow(max(0., dot(normal, halfvec)), shininess);
    color += factor * 7.*gl_LightSource[i].specular * specular_factor * specular_refl;
  }
  */

  vec4 color = vec4(0., 0., 0., 0.);
  int N = 5;
  float factor = 1./float(N);
  //  float f = pow(10., 1./float(N));
  for (int i=0; i<N; i++)
  {
    //    float lambda = pow(f, float(i+1))/10.;
    float lambda = float(i) * factor;
    vec3 lpos = gl_LightSource[0].position.xyz + 
      lambda * (gl_LightSource[3].position.xyz - gl_LightSource[0].position.xyz);

    // calculate ambient component
    color += factor * gl_LightSource[0].ambient * ambient_refl;
    
    // calculate diffuse component
    vec3 light_dir = normalize(lpos - pos);
    
    float diffuse_factor = max(0., dot(normal, light_dir));
    color += factor * gl_LightSource[0].diffuse * diffuse_factor * diffuse_refl;
    
    // calculate specular component
    vec3 halfvec = normalize(light_dir - pos);
    
    float specular_factor = pow(max(0., dot(normal, halfvec)), shininess);
    color += factor * 4. * gl_LightSource[0].specular * specular_factor * specular_refl;
  }

  color.w = alpha;
  gl_FragColor = color;
}
