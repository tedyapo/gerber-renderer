#ifndef GLSLSHADER_INCLUDED_
#define GLSLSHADER_INCLUDED_

class GLSLShader {
public:
  GLSLShader(const char *vertex_filename, const char *fragment_filename){
    LoadShaders(vertex_filename, fragment_filename);
  }

  ~GLSLShader(){

    glDetachShader(program, vertex_shader); 
    glDetachShader(program, fragment_shader); 
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    glDeleteProgram(program);
    glUseProgram(0); 
  }

  void Enable(){
    glUseProgram(program); 
  }

  void Disable(){
    glUseProgram(0); 
  }

  void SetVariable(const char *variable, float f1){
    GLuint location = glGetUniformLocation(program, variable);
    glUniform1f(location, f1);
  }

  void SetVariable(const char *variable, GLfloat f1, GLfloat f2){
    GLuint location = glGetUniformLocation(program, variable);
    glUniform2f(location, f1, f2);
  }

  void SetVariable(const char *variable, GLfloat f1, GLfloat f2, GLfloat f3){
    GLuint location = glGetUniformLocation(program, variable);
    glUniform3f(location, f1, f2, f3);
  }

  void SetVariable(const char *variable, GLfloat f1, GLfloat f2,
		   GLfloat f3, GLfloat f4){
    GLuint location = glGetUniformLocation(program, variable);
    glUniform4f(location, f1, f2, f3, f4);
  }

  void SetVariable(const char *variable, int i1){
    GLuint location = glGetUniformLocation(program, variable);
    glUniform1i(location, i1);
  }

  void SetVariable(const char *variable, GLint i1, GLint i2){
    GLuint location = glGetUniformLocation(program, variable);
    glUniform2i(location, i1, i2);
  }

  void SetVariable(const char *variable, GLint i1, GLint i2, GLint i3){
    GLuint location = glGetUniformLocation(program, variable);
    glUniform3i(location, i1, i2, i3);
  }

  void SetVariable(const char *variable, GLint i1, GLint i2,
		   GLint i3, GLint i4){
    GLuint location = glGetUniformLocation(program, variable);
    glUniform4i(location, i1, i2, i3, i4);
  }

  //#warning this doesn't work; can't treat as all same function!!!!
  void SetVector(const char *variable, GLint n_vectors,
		 const GLfloat *var){
    GLuint location = glGetUniformLocation(program, variable);
    //    glUniform1fv(location, n_components*n_vectors, var);
    glUniform3fv(location, n_vectors, var);
  }

  void SetVector(const char *variable,
		 const GLint var[4]){
    GLuint location = glGetUniformLocation(program, variable);
    glUniform4iv(location, 1, var);
  }

  //#warning privatize this
  GLuint program;
private:

  GLuint vertex_shader;
  GLuint fragment_shader;

  GLSLShader (const GLSLShader &a) {}
  GLSLShader &operator= (const GLSLShader &a) {return *this;};

  GLchar *ReadFileToString(const char *filename){
    FILE *fp = fopen(filename, "rt");
    if (NULL == fp){
      fprintf(stderr, "unable to open %s\n", filename);
      assert(0);
    }
    fseek(fp, 0, SEEK_END);
    int length = ftell(fp);
    char *string = new GLchar[length+1];
    memset(string, 0, length+1);
    rewind(fp);
    size_t size = fread(string, length, 1, fp);
    if (size != (size_t)1){
      fprintf(stderr, "bad read from file %s\n", filename);
      assert(0);
    }
    fclose(fp);
    return string;
  }

  void CheckShader(GLuint shader, const char *name = ""){
    int length;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
    if (length > 1){
      char *log = new char[length];
      int actual_len;
      glGetShaderInfoLog(shader, length, &actual_len, log);
      fprintf(stderr, "GLSL Shader error (%s): %s\n", name, log);
      delete log;
    }
    
    int status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (GL_TRUE != status){
      fprintf(stderr, "Aborting\n");
      exit(-1);
    }
  }

  void CheckProgram(GLuint program){
    int length;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length); 
    if (length > 1){
      char *log = new char[length];
      int actual_len;
      glGetProgramInfoLog(program, length, &actual_len, log);
      fprintf(stderr, "GLSL Program error: %s\n", log);
      delete log;
    }
    
    int status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (GL_TRUE != status){
      fprintf(stderr, "Aborting\n");
      exit(-1);
    }
  }

  void LoadShaders(const char *vertex_filename,
		   const char *fragment_filename){
    // load vertex shader
    GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
    const GLchar *vertex_source = ReadFileToString(vertex_filename);
    int vertex_len = strlen(vertex_source);
    glShaderSource(vertex, 1, &vertex_source, &vertex_len); 
    glCompileShader(vertex);
    CheckShader(vertex, "VERTEX");
    delete vertex_source;

    // load fragment shader
    GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
    const GLchar *fragment_source = ReadFileToString(fragment_filename);
    int fragment_len = strlen(fragment_source);
    glShaderSource(fragment, 1, &fragment_source, &fragment_len); 
    glCompileShader(fragment);
    CheckShader(fragment, "FRAGMENT");

    program = glCreateProgram(); 
    glAttachShader(program, vertex); 
    glAttachShader(program, fragment);
    glLinkProgram(program); 
    CheckProgram(program);
  }
};


#endif //#ifndef GLSLSHADER_INCLUDED_
