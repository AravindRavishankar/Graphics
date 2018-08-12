#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#define MIN(x,y) (x < y ? x : y)
#define MAX(x,y) (x > y ? x : y)
#include <glad/glad.h>
#include <GLFW/glfw3.h>
int HEIGHT_OF_WINDOW;
int WIDTH_OF_WINDOW;
#define INF 1000
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
 
#include <FTGL/ftgl.h>
#include <GLFW/glfw3.h>
#include <SOIL/SOIL.h>

using namespace std;
struct VAO {
	GLuint VertexArrayID;
	GLuint VertexBuffer;
	GLuint ColorBuffer;
	GLuint TextureBuffer;
	GLuint TextureID;

	GLenum PrimitiveMode; // GL_POINTS, GL_LINE_STRIP, GL_LINE_LOOP, GL_LINES, GL_LINE_STRIP_ADJACENCY, GL_LINES_ADJACENCY, GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN, GL_TRIANGLES, GL_TRIANGLE_STRIP_ADJACENCY and GL_TRIANGLES_ADJACENCY
	GLenum FillMode; // GL_FILL, GL_LINE
	int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID; // For use with normal shader
	GLuint TexMatrixID; // For use with texture shader
} Matrices;


struct FTGLFont {
	FTFont* font;
	GLuint fontMatrixID;
	GLuint fontColorID;
} GL3Font;
GLuint programID,fontProgramID, textureProgramID;;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	// Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

glm::vec3 getRGBfromHue (int hue)
{
	float intp;
	float fracp = modff(hue/60.0, &intp);
	float x = 1.0 - abs((float)((int)intp%2)+fracp-1.0);

	if (hue < 60)
		return glm::vec3(1,x,0);
	else if (hue < 120)
		return glm::vec3(x,1,0);
	else if (hue < 180)
		return glm::vec3(0,1,x);
	else if (hue < 240)
		return glm::vec3(0,x,1);
	else if (hue < 300)
		return glm::vec3(x,0,1);
	else
		return glm::vec3(1,0,x);
}

void debug(vector <float>a)
{
	for(int i=0;i<a.size();i++)
	{
		cout<<a[i]<<" ";
	}
	cout<<endl;
}
static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
struct VAO* create3DTexturedObject (GLenum primitive_mode, int numVertices,  GLfloat* vertex_buffer_data,  GLfloat* texture_buffer_data, GLuint textureID, GLenum fill_mode=GL_FILL)
{
	struct VAO* vao = new struct VAO;
	vao->PrimitiveMode = primitive_mode;
	vao->NumVertices = numVertices;
	vao->FillMode = fill_mode;
	vao->TextureID = textureID;

	// Create Vertex Array Object
	// Should be done after CreateWindow and before any other GL calls
	glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
	glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
	glGenBuffers (1, &(vao->TextureBuffer));  // VBO - textures

	glBindVertexArray (vao->VertexArrayID); // Bind the VAO
	glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices
	glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
	glVertexAttribPointer(
						  0,                  // attribute 0. Vertices
						  3,                  // size (x,y,z)
						  GL_FLOAT,           // type
						  GL_FALSE,           // normalized?
						  0,                  // stride
						  (void*)0            // array buffer offset
						  );

	glBindBuffer (GL_ARRAY_BUFFER, vao->TextureBuffer); // Bind the VBO textures
	glBufferData (GL_ARRAY_BUFFER, 2*numVertices*sizeof(GLfloat), texture_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
	glVertexAttribPointer(
						  2,                  // attribute 2. Textures
						  2,                  // size (s,t)
						  GL_FLOAT,           // type
						  GL_FALSE,           // normalized?
						  0,                  // stride
						  (void*)0            // array buffer offset
						  );

	return vao;
}



	
/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices,  GLfloat* vertex_buffer_data,  GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
    struct VAO* vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->NumVertices = numVertices;
    vao->FillMode = fill_mode;

    // Create Vertex Array Object
    // Should be done after CreateWindow and before any other GL calls
    glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
    glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
    glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

    glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
    glVertexAttribPointer(
                          0,                  // attribute 0. Vertices
                          3,                  // size (x,y,z)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
    glVertexAttribPointer(
                          1,                  // attribute 1. Color
                          3,                  // size (r,g,b)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices,  GLfloat* vertex_buffer_data,  GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
    GLfloat* color_buffer_data = new GLfloat [3*numVertices];
    for (int i=0; i<numVertices; i++) {
        color_buffer_data [3*i] = red;
        color_buffer_data [3*i + 1] = green;
        color_buffer_data [3*i + 2] = blue;
    }

    return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
    // Change the Fill Mode for this object
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

    // Bind the VAO to use
    glBindVertexArray (vao->VertexArrayID);

    // Enable Vertex Attribute 0 - 3d Vertices
    glEnableVertexAttribArray(0);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

    // Enable Vertex Attribute 1 - Color
    glEnableVertexAttribArray(1);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

    // Draw the geometry !
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

void draw3DTexturedObject (struct VAO* vao)
{
	// Change the Fill Mode for this object
	glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

	// Bind the VAO to use
	glBindVertexArray (vao->VertexArrayID);

	// Enable Vertex Attribute 0 - 3d Vertices
	glEnableVertexAttribArray(0);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

	// Bind Textures using texture units
	glBindTexture(GL_TEXTURE_2D, vao->TextureID);

	// Enable Vertex Attribute 2 - Texture
	glEnableVertexAttribArray(2);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->TextureBuffer);

	// Draw the geometry !
	glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle

	// Unbind Textures to be safe
	glBindTexture(GL_TEXTURE_2D, 0);
}
GLuint createTexture (const char* filename)
{
	GLuint TextureID;
	// Generate Texture Buffer
	glGenTextures(1, &TextureID);
	// All upcoming GL_TEXTURE_2D operations now have effect on our texture buffer
	glBindTexture(GL_TEXTURE_2D, TextureID);
	// Set our texture parameters
	// Set texture wrapping to GL_REPEAT
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// Set texture filtering (interpolation)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Load image and create OpenGL texture
	int twidth, theight;
	unsigned char* image = SOIL_load_image(filename, &twidth, &theight, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, twidth, theight, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D); // Generate MipMaps to use
	SOIL_free_image_data(image); // Free the data read from file after creating opengl texture
	glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done, so we won't accidentily mess it up

	return TextureID;
}


void print(string a)
{
	cout<<a<<endl;
}
glm::mat4 MVP;
glm::mat4 VP; //MVP = Projection * View * Model
class Point
{
	public:
	float x,y,z;
	Point(float x1=0,float y1=0,float z1=0)
	{
		x=x1;
		y=y1;
		z=z1;
	}
	void update(Point a)
	{
		x+=a.x;
		y+=a.y;
		z+=a.z;
	}
	void set(Point p)
	{
		x=p.x;
		y=p.y;
		z=p.z;
	}
	void print()
	{
		cout<<x<<" "<<y<<" "<<z<<endl;
	}
};
class vect
{
	public:
		Point initial;
		float x,y,z;
		float angle;
		//angle made by the vector with origin in radians
		vect(Point p=Point(0,0,0),float x1=0,float y1=0,float z1=0)
		{
			x=x1;
			y=y1;
			z=z1;
			initial=p;
			angle=computeAngle(y,x);

			
		}
		float computeAngle(float x,float y ) //returns angle in radians between 0 to 2*pi
		{
			float a=atan2(x,y);
			if(a<0)
				return a+2*M_PI;
			return a;
		}
		void form(Point q,Point p)
		{
			x=p.x-q.x;
			y=p.y-q.y;
			z=p.z-q.z;
			initial=q;
			angle=computeAngle(y,x);
		}
		void set(vect r)
		{
			initial=r.initial;
			x=r.x;
			y=r.y;
			z=r.z;
			angle=computeAngle(y,x);
		}
		void translate(vect r)
		{
			x+=r.x;
			y+=r.y;
			z+=r.z;
			angle=computeAngle(y,x);
		}
		float modulus()
		{
		//	return sqrt(x*x+y*y+z*z);
			return sqrt(x*x+y*y+z*z);
		}
		float dotProduct(vect s)
		{
//			return s.x*x+s.y*y+s.z*z;
			return s.x*x+s.y*y+s.z*z;
		}
		vect crossProduct(vect b)
		{
			vect c;
			c.initial=Point(0,0,0);
			c.x=y*b.z-z*b.y;
			c.y = z*b.x-x*b.z;
			c.z = x*b.y-y*b.x;
			c.angle=computeAngle(c.y,c.x);
			return c;
		}
   		float component_along(vect q)
		{
			float dot_product=dotProduct(q);
	//		cout<<"dot product is "<<dot_product<<endl;
			float mod=q.modulus();
			if(mod!=0)
			return dot_product/mod;
			return -1;

		}
		float component_perpendicular(vect q)
		{
			vect cross=crossProduct(q);
			float mod1=-cross.z;//since x cross y is z
			float mod=q.modulus();
			if(mod!=0)
				return mod1/mod;
			return -1;
		}
		float distance(Point p)
		{
			vect b=vect(Point(0,0,0),p.x-initial.x,p.y-initial.y,0); 
			vect w=crossProduct(b);
			float mod_cross=w.modulus();
			float mod=modulus();
			float dist=mod_cross/mod;
			return dist;
			
		}
		bool Point_in_segment(Point b)
		{
			vect v1=vect(Point(0,0,0),b.x-initial.x,b.y-initial.y,0);
			vect v2=vect(Point(0,0,0),b.x-(initial.x+x),b.y-(initial.y+y),0);
		//	cout<<"initial\n";
		//	cout<<initial.x<<" "<<initial.y<<" "<<initial.z<<endl;
		//	cout<<"dotProduct\n"<<endl;
		//	cout<<dotProduct(v1)<<endl;
		//	cout<<dotProduct(v2)<<endl;
			if(dotProduct(v1)*dotProduct(v2)>0)
				return false;
			return true;
		}
		
	

		
};
class Color
{
	public:
		float r,g,b;
	Color(int r1=0,int g1=0,int b1=0)
	{
		r=r1/255.0;
		g=g1/255.0;
		b=b1/255.0;
	
	}
	void print()
	{
		cout<<r<<" "<<g<<" "<<b<<endl;
	}
	

};
void debug(vect v)
{
	cout<<v.initial.x<<" "<<v.initial.y<<" "<<v.initial.z<<" "<<v.x<<" "<<v.y<<" "<<v.z<<endl;
}
void print(float a)
{
	cout<<a<<endl;
	
}
void print(Point a)
{
	cout<<a.x<<" "<<a.y<<" "<<a.z<<endl;
}
void print(vect a)
{
	cout<<a.initial.x<<" "<<a.initial.y<<" "<<a.initial.z<<" "<<a.x<<" "<<a.y<<" "<<a.z<<endl;
}
class Shape
{
	public :	
	vector <Point> initial;
	vector<float>vertex_buffer;
	vector <Point>cur;
	Point centroid_initial;
	Point centroid_cur;
	vector<vect>sides_initial;
	vector<vect>sides_cur;
	float rad;
	string type;
	Shape()
	{
		rad=-1;
		initial.clear();
		vertex_buffer.clear();
		cur.clear();
		type="polygon";
	}
	void push_point(float x1,float y1,float z1)
	{
		Point t(x1,y1,z1);;
		initial.push_back(t);
		cur.push_back(t);
	}
	void addToVertexBuffer(Point q)
	{
		vertex_buffer.push_back(q.x);
		vertex_buffer.push_back(q.y);
		vertex_buffer.push_back(q.z);
	}
	void ShapeInfo()
	{
		Point sum;
		for(int i=0;i<initial.size();i++)
		{
			sum.x+=initial[i].x;
			sum.y+=initial[i].y;
			sum.z+=initial[i].z;
			vect t;
			t.form(initial[i],initial[(i+1)%initial.size()]);
			sides_initial.push_back(t);
			sides_cur.push_back(t);
		}
		centroid_initial.x=sum.x/initial.size();
		centroid_initial.y=sum.y/initial.size();
		centroid_initial.z=sum.z/initial.size();
		centroid_cur=centroid_initial;
	}
	void form_polygon()
	{
		ShapeInfo();
		for(int i=0;i<initial.size();i++)
		{
			addToVertexBuffer(centroid_initial);
			addToVertexBuffer(initial[i]);
			addToVertexBuffer(initial[(i+1)%initial.size()]);
		}
	}
	void getArc(Point c,float radius,float start_angle,float end_angle)
	{
		type="polygon";
		push_point(c.x,c.y,c.z);
		for(float i=start_angle;i<end_angle;i=i+1)
		{
			float b=i*M_PI/180.0;
			float x1=c.x+radius*cos(b);
			float y1=c.y+radius*sin(b);
			float z1=c.z;
			push_point(x1,y1,z1);
		}
	}
	void getRegular(Point c,float radius,int numberOfVertices)
	{
		if(numberOfVertices>150)
			type="circle";
		else
			type="polygon";
		rad=radius;
		initial.clear();
		for(int i=0;i<numberOfVertices;i++)
		{
			float val=2*M_PI/(numberOfVertices);
			float x=c.x+radius*cos(i*val);
			float y=c.y+radius*sin(i*val);
			float z=c.z;
			push_point(x,y,z);
		}
	}
	void getRectangle(Point bottom_left,float wid=0,float ht=0)	
	{
		type="polygon";
		Point a[4];
		a[0].x=bottom_left.x;
		a[0].y=bottom_left.y;
		a[1].y=a[0].y;
		a[1].x=a[0].x+wid;
		a[2].x=a[1].x;
		a[2].y=a[1].y+ht;
		a[3].x=a[0].x;
		a[3].y=a[2].y;
		for(int i=0;i<4;i++)
		{
			a[i].z=bottom_left.z;
			push_point(a[i].x,a[i].y,a[i].z);
		}
	}
	void createVertexBuffer(vector<float>  vert)
	{
		for(int i=0;i<vert.size();i++)
		{
			vertex_buffer.push_back(vert[i]);
		
		}
		int k=0;
		for(int i=0;i<vertex_buffer.size();i+=3)
		{
			push_point(vert[i],vert[i+1],vert[i+2]);
		}
		ShapeInfo();

	}
	void drawCircle(Point c,float radius)
	{
		type="circle";
		rad=radius;
		getRegular(c,radius,360);
	}
	
};
class Figure
{
	public:
	Shape P;
	glm::mat4 transform;
	vect v,a;
	Point pos; //position  
	float e;
	vector<Color>color;
	vector<float>color_buffer;
	vector <float>texture_buffer;	
	VAO *obj;
	Color centre;
	bool gradient;
	double length,width,height;
	Figure()
	{
		transform=glm::mat4(1.0f);
		color.clear();
		vect v1(Point(0,0,0),0,0,0);
		v=v1;
		a=v1;;
		gradient=true;
		e=1;
		pos=Point(0,0,0);
	}
	void scale(vect r)
	{
		glm::mat4 scalefigure=glm::scale(glm::vec3(r.x,r.y,r.z));
		transform=scalefigure*transform;
		update(1,scalefigure);
	}
	void push_color(int r,int g,int b)
	{
		Color c(r,g,b);
		color.push_back(c);
	}
	void addToColorBuffer(Color c)
	{
		color_buffer.push_back(c.r);
		color_buffer.push_back(c.g);
		color_buffer.push_back(c.b);
	}
	void translateby(vect r)
	{
		glm::mat4 translatefigure = glm::translate (glm::vec3(r.x, r.y, r.z));
		transform=translatefigure*transform;
		update(1,translatefigure);
	}
	void translateTo(Point r)
	{


		glm::mat4 translatefigure = glm::translate (glm::vec3(-P.centroid_cur.x, -P.centroid_cur.y, -P.centroid_cur.z));
		glm::mat4 translatefigure1=glm::translate (glm::vec3(r.x, r.y,r.z));
		transform=translatefigure*transform;
		update(1,translatefigure);
		transform=translatefigure1*transform;
		update(1,translatefigure1);

	}
	void rotate_about_point(Point c,float angle)
	{
		glm::mat4 translatefigure1=glm::translate(glm::vec3(-c.x,-c.y,0));
  		glm::mat4 rotatefigure = glm::rotate((float)(angle*M_PI/180.0f), glm::vec3(0,0,1));
		glm::mat4 translatefigure2=glm::translate(glm::vec3(c.x,c.y,0));
		transform=translatefigure1*transform;
		update(1,translatefigure1);
		transform=rotatefigure*transform;
		update(0,rotatefigure);
		transform=translatefigure2*transform;
		update(1,translatefigure2);
	}
	void rotate_about_own(float angle)
	{
		
		glm::mat4 translatefigure1=glm::translate(glm::vec3(-P.centroid_cur.x,-P.centroid_cur.y,-P.centroid_cur.z));		 
		glm::mat4 rotatefigure = glm::rotate((float)(angle*M_PI/180.0f), glm::vec3(0,0,1));
		glm::mat4 translatefigure2=glm::translate(glm::vec3(P.centroid_cur.x,P.centroid_cur.y,P.centroid_cur.z));
		transform=translatefigure1*transform;
		update(1,translatefigure1);
		transform=rotatefigure*transform;
		update(0,rotatefigure);
		transform=translatefigure2*transform;
		update(1,translatefigure2);
	}
	void setCentreColor(int r,int g,int b)
	{
		Color t(r,g,b);
		centre=t;
		gradient=false;
	}
	void setFixedColor(int r,int g,int b)
	{
		for(int i=0;i<P.initial.size();i++)
		{
			push_color(r,g,b);
		}
		Color p(r,g,b);
		centre=p;
		gradient=false;
	}
	void createColorBuffer(vector<float> c)
	{
	/*	for(int i=0;i<c.size();i++)
		{
			color_buffer.push_back(c[i]/255.0);
			
		}*/
		int k=0;
		for(int i=0;i<c.size();i+=3,k+=1)
		{
			push_color(c[i],c[i+1],c[i+2]);
			addToColorBuffer(color[k]);
		}
	}
	void rectSetColor(int r1=0,int g1=0,int b1=0,int r2=0,int g2=0,int b2=0)
	{
		for(int i=0;i<4;i++)
		{
			if(i==0 ||i==1)
			{
				push_color(r1,g1,b1);
				
			}
			else
				push_color(r2,g2,b2);
		}
	}
	void formCentrecolor()
	{
		Color sum;
		for(int i=0;i<color.size();i++)
		{
			sum.r+=color[i].r;
			sum.g+=color[i].g;
			sum.b+=color[i].b;
		}
		if(gradient)
		{
			centre.r=sum.r/color.size();
			centre.g=sum.g/color.size();
			centre.b=sum.b/color.size();
		}
	}
	void form_color()
	{
	
		formCentrecolor();
		for(int i=0;i<color.size();i++)
		{
			addToColorBuffer(centre);
			addToColorBuffer(color[i]);
			addToColorBuffer(color[(i+1)%color.size()]);
		}

	}
	void createPolygonFigure()
	{
		P.form_polygon();
		pos=P.centroid_initial;
		form_color();
//		createFigure();
	}
	void createFigure(GLuint textureID)
	{
		const int sz1=P.vertex_buffer.size();
		const int sz2=texture_buffer.size();
		int size=P.vertex_buffer.size()/3;
		GLfloat v[sz1],t[sz2];
		for(int i=0;i<sz1;i++)
		{
			v[i]=P.vertex_buffer[i];

		
		}
		for(int i=0;i<sz2;i++)
		{
			t[i]=texture_buffer[i];
		}
		print("size");
		print(size);
		print("vertex");
		debug(P.vertex_buffer);
		print("color");
		debug(color_buffer);
		obj=create3DTexturedObject(GL_TRIANGLES,size,v,t,textureID, GL_FILL);

	}
	void createTextureBuffer()
	{
		float t[]={
		1,1,
		1,0,
		0,0,
		0,0,
		1,1,
		1,0,
		0,0,
		1,1,
		0,1,
		0,0,
		0,1,
		1,1,
		1,1,
		0,0,
		0,1,
		0,0,
		1,0,
		1,1,
		0,0,
		1,0,
		1,1,
		0,0,
		1,1,
		0,1,
		1,1,
		0,0,
		1,0,
		0,0,
		0,1,
		1,1,
		0,0,
		1,1,
		1,0,
		0,1,
		0,0,
		1,1
		};

/*		float t [] = {
		0,1, 
		1,1, 
		1,0, 

		0,1,  
		1,0, 
		0,0, 

		1,1, 
		0,1, 
		1,0, 

		0,1,  
		1,0, 
		0,0, 

		1,1, 
		0,1, 
		1,0, 

		0,1,  
		1,0, 
		0,0, 

		1,1, 
		0,1, 
		1,0, 

		0,1,  
		1,0, 
		0,0, 

		1,1, 
		0,1, 
		1,0, 

		0,1,  
		1,0, 
		0,0, 

		1,1, 
		0,1, 
		1,0, 

		0,1,  
		1,0, 
		0,0,
		
	};*/
	texture_buffer.clear();
		for(int i=0;i<72;i++)
		{
			texture_buffer.push_back(t[i]);
		}
		

	}
	void draw( )
	{
		Matrices.model=glm::mat4(1.0f);
		Matrices.model*=transform;
		MVP=VP*Matrices.model;
		glUniformMatrix4fv(Matrices.TexMatrixID, 1, GL_FALSE, &MVP[0][0]);
	//	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0/]);
	//	draw3DObject(obj);
		draw3DTexturedObject(obj);
		glUniform1i(glGetUniformLocation(textureProgramID, "texSampler"), 0);
		transform=glm::mat4(1.0f);
		for(int i=0;i<P.cur.size();i++)
		{
			P.cur[i]=P.initial[i];
		}
		for(int i=0;i<P.sides_initial.size();i++)
		{
			P.sides_cur[i]=P.sides_initial[i];
		}
		P.centroid_cur=P.centroid_initial;
	}	
	void update(int index,glm::mat4 trans)
	{
		Point sum;
		for(int i=0;i<P.initial.size();i++)
		{
			glm::vec4 myVector(P.cur[i].x,P.cur[i].y,P.cur[i].z,1);
			glm::vec4 transformedVector=trans*myVector;
			glm::mat4 m;
			m[0]=transformedVector;
			P.cur[i].x=m[0][0];
			P.cur[i].y=m[0][1];
			P.cur[i].z=m[0][2];
			sum.x+=P.cur[i].x;
			sum.y+=P.cur[i].y;
			sum.z+=P.cur[i].z;
		}
		for(int i=0;i<P.initial.size();i++)
		{

			vect t;
			t.form(P.cur[i],P.cur[(i+1)%P.initial.size()]);
			P.sides_cur[i]=t;
		}
		sum.x/=P.cur.size();
		sum.y/=P.cur.size();
		sum.z/=P.cur.size();
		P.centroid_cur=sum;
//		pos=P.centroid_cur;
	}
	void objectphysics()
	{
//		print("before");
//		print(pos);
		pos.update(Point(v.x,v.y,v.z));
//		print(v);
//		print(pos);
//		print("After");
		v.translate(a);
		
		translateTo(pos);
	}
	void physics()
	{

//		print(v);

		pos.update(Point(v.x,v.y,v.z));
		v.translate(a);
//		print(a);
//		print(pos);
		translateby(vect(Point(0,0,0),pos.x,pos.y,pos.z));
		
	}
};
Point eye;
Figure Ball;
Figure Ground;
vector<Figure>Circular_Obstacles;
Figure Sky;
Figure arc,pentagon,circle;
Figure rect;
vector <Figure> reg;
Point preveye;
Figure nightsky;
void printsides(Figure a)
{
	print("length");
	print(a.length);

	print("widthh");
	print(a.width);

	print("height");
	print(a.height);

	
}
void debug(vector<vect> p)
{
	for(int i=0;i<p.size();i++)
		debug(p[i]);
}
void createRect()
{

 	GLuint textureID = createTexture("beach2.png");
//	rect.P.getRectangle(Point(0,0,8),,2);
//	rect.setFixedColor(10,10,10);
//	rect.createFigure(textureID);
//	rect.e=1.1;
}
void createSky()
{
	Sky.P.getRectangle(Point(0,100,0),WIDTH_OF_WINDOW,HEIGHT_OF_WINDOW-100);
	Sky.rectSetColor(103,166,151,7,81,106);
	Sky.createPolygonFigure();
}
void createGround()
{
	Ground.P.getRectangle(Point(0,0,0),WIDTH_OF_WINDOW,100);
	Ground.rectSetColor(52,34,34,69,50,35);
	Ground.createPolygonFigure();
	Ground.e=1;
}
int numberOfObstacles;

float eyex=0,eyey=-40,eyez=40;
float targetx,targety,targetz;
int numberOfMovingObstacles;
vector <Figure>obstacle;
vector<Figure> moving_obstacle;
void createObstacles()
{

 GLuint textureID = createTexture("visible.jpeg");


float v[] = {
      -1.0f,-1.0f,-1.0f, // triangle 1 : begin
     -1.0f,-1.0f, 1.0f,
      -1.0f, 1.0f, 1.0f, // triangle 1 : end
      1.0f, 1.0f,-1.0f, // triangle 2 : begin
      -1.0f,-1.0f,-1.0f,
      -1.0f, 1.0f,-1.0f, // triangle 2 : end
     1.0f,-1.0f, 1.0f,
     -1.0f,-1.0f,-1.0f,
     1.0f,-1.0f,-1.0f,
     1.0f, 1.0f,-1.0f,
     1.0f,-1.0f,-1.0f,
     -1.0f,-1.0f,-1.0f,
     -1.0f,-1.0f,-1.0f,
    -1.0f, 1.0f, 1.0f,     
    -1.0f, 1.0f,-1.0f,
     1.0f,-1.0f, 1.0f,
     -1.0f,-1.0f, 1.0f,
     -1.0f,-1.0f,-1.0f,
     -1.0f, 1.0f, 1.0f,
     -1.0f,-1.0f, 1.0f,
     1.0f,-1.0f, 1.0f,
     1.0f, 1.0f, 1.0f,
     1.0f,-1.0f,-1.0f,
     1.0f, 1.0f,-1.0f,
     1.0f,-1.0f,-1.0f,
     1.0f, 1.0f, 1.0f,
     1.0f,-1.0f, 1.0f,
     1.0f, 1.0f, 1.0f,
     1.0f, 1.0f,-1.0f,
     -1.0f, 1.0f,-1.0f,
     1.0f, 1.0f, 1.0f,
     -1.0f, 1.0f,-1.0f,
     -1.0f, 1.0f, 1.0f,
     1.0f, 1.0f, 1.0f,
     -1.0f, 1.0f, 1.0f,
     1.0f,-1.0f, 1.0f
 };
 
 vector<float> vec (v, v + sizeof(v) / sizeof(v[0]) );
 vector <float> color;
 color.clear();
 for(int i=0;i<108;i++)
 {

	 if(i%3!=2)
		 color.push_back(255);
	 else 
		 color.push_back(0);
 }
 

 for(int i=0;i<numberOfObstacles;i++)
 {
	 
	 Figure t;


	 t.P.createVertexBuffer(vec);
//	 t.createColorBuffer(color);
	 t.createTextureBuffer();	

 	 t.createFigure(textureID);
	 obstacle.push_back(t);
	obstacle[i].length=10;
 	obstacle[i].width=6;;
 	obstacle[i].height=6;
 	obstacle[i].pos=Point(0,0,0);
	
	obstacle[i].v.set(vect(Point(0,0,0),0,0,-0.02*(i%5+1)));
	obstacle[i].a.set(vect(Point(0,0,0),0,0,0));
	

//	tiles[i].v.set(vect(Point(0,0,0),0,0,0));
  //      tiles[i].a.set(vect(Point(0,0,0),0,0,0));
  }
 color.clear();

 for(int i=0;i<108;i++)
 {

	 if(i%3==0)
		 color.push_back(210);
	 else if(i%3==1)
		 color.push_back(180);
	 else
		 color.push_back(140);

 }

 GLuint textureID2 = createTexture("moving.jpeg");
 for(int i=0;i<numberOfMovingObstacles;i++)
 {

	 Figure t;
	 t.P.createVertexBuffer(vec);
	 t.createTextureBuffer();
	 t.createColorBuffer(color);
 	 t.createFigure(textureID2);
	 moving_obstacle.push_back(t);
	moving_obstacle[i].length=4;
 	moving_obstacle[i].width=4;;
 	moving_obstacle[i].height=4;
 	moving_obstacle[i].pos=Point(0,0,0);
	moving_obstacle[i].v.set(vect(Point(0,0,0),0,0,0.8));
	moving_obstacle[i].a.set(vect(Point(0,0,0),0,0,-0.1));
  }
}
int numberOfTiles;
int mark[100][100];
int mov[100][100];
Figure player;
int index_mov[200];
int obs[100][100];
int jump[100][100];
void randomizeGrid()
{
	int cnt=0;
	int cnt3=0;
	int previ=0;
	int prevj=0;
//	int count=0;//cube is present if mark[i][j] is 1
	for(int i=0;i<12;i++)
	{

		for(int j=0;j<12;j++)
		{
			if(i==j|| i<j || i>j+2)

				mark[i][j]=0;
			else  mark[i][j]=rand()%2;;
		
			
			if(mark[i][j]%2==0 && i%5==0 && j%3==0)
			{
				print("wtf");
				index_mov[cnt]=1;
				
			}
			else if(mark[i][j]%2==0 && i%5==4 && j%3==2 && numberOfObstacles<=7)
			{
				obs[i][j]=1;
				numberOfObstacles++;
				
			}
			else if(mark[i][j]%2==0  && fabs(i-previ)>=2 && fabs(j-prevj)>=4 && numberOfMovingObstacles<=4)
			{
				jump[i][j]=1;
				numberOfMovingObstacles++;
				previ=i;
				prevj=j;

			}
			if(mark[i][j]==0)
			{
				cnt++;
				numberOfTiles++;
			}


			
		}
	}
};
vector <Figure> tiles ;
vector <Figure> death_tiles;


int cur_x=0;
int cur_y=0;
int xpos[100];
int ypos[100];
float angle=0;
bool CheckCubesCollision(Figure a, Figure b)
{
   //check the X axis
   if(fabs(a.P.centroid_cur.x - b.P.centroid_cur.x) <=a.length/2+ b.length/2)
   {
      //check the Y axis
      if(fabs(a.P.centroid_cur.y - b.P.centroid_cur.y) <= a.width/2 + b.width/2)
      {
          //check the Z axis
          if(fabs(a.P.centroid_cur.z - b.P.centroid_cur.z) <= a.height/2+b.height/2)
          {
             return true;
          }
      }
   }

   return false;
}
double tiles_length,tiles_width,tiles_height;
int MovingTiles=10;
Point surface_centre;
void computecentre()
{

	print(player.P.cur[0]);
	Point c=player.P.centroid_cur;


	for(int i=0;i<=33;i++)
	{
		if(i==2||i==3||i==5||i==33)
		{
			print(player.P.cur[i].x-player.P.centroid_cur.x);
	
		surface_centre.update(player.P.cur[i]);
		}
		
	}
	surface_centre.x/=4;
	surface_centre.y/=4;
	surface_centre.z/=4;
	eyex=4*surface_centre.x;

	eyey=4*surface_centre.y;
	eyez=4*surface_centre.z;
	targetx=7*surface_centre.x;
	targety=7*surface_centre.y;
	targetz=7*surface_centre.z;
	targetx-=6*player.P.centroid_cur.x;
	targety-=6*player.P.centroid_cur.y;
	targetz-=6*player.P.centroid_cur.z;
//	targetx/=3;
//	targety/=3;
//	targetz/=3;
	eyex-=3*player.P.centroid_cur.x;
	eyey-=3*player.P.centroid_cur.y;
	eyez-=3*player.P.centroid_cur.z;
	surface_centre.x=0;
	surface_centre.y=0;
	surface_centre.z=0;
	
	print("susadasrface");
	print(surface_centre);

}
void follow_on()
{
	print(player.P.cur[0]);
	Point c=player.P.centroid_cur;
	for(int i=0;i<=33;i++)
	{
		if(i==0||i==1||i==6||i==8)
		{
			print(player.P.cur[i].x-player.P.centroid_cur.x);
			surface_centre.update(player.P.cur[i]);
		}
	}
	surface_centre.x/=4;
	surface_centre.y/=4;
	surface_centre.z/=4;
	surface_centre.y;
	eyex=7*surface_centre.x;
	eyey=7*surface_centre.y;
	eyez=7*surface_centre.z;
	targetx=4*surface_centre.x;
	targety=4*surface_centre.y;
	targetz=4*surface_centre.z;
	targetx-=3*player.P.centroid_cur.x;
	targety-=3*player.P.centroid_cur.y;
	targetz-=3*player.P.centroid_cur.z;
//	targetx/=3;
//	targety/=3;
//	targetz/=3;
	eyex-=6*player.P.centroid_cur.x;
	eyey-=6*player.P.centroid_cur.y;
	eyez-=6*player.P.centroid_cur.z;
	surface_centre.x=0;
	surface_centre.y=0;
	surface_centre.z=0;
	
	print("susadasrface");
	print(surface_centre);

}
void makeGrid()
{
	surface_centre=Point(0,0,0);
	const int x=12;
	const int y=12;
	int cnt=0;
	int cnt1=0;
	int cnt2=0;
	int cnt3=0;
	vect scal=vect(Point(0,0,0),tiles_length/2,tiles_width/2,tiles_height/2);
	nightsky.scale(vect(Point(0,0,0),nightsky.length/2,nightsky.width/2,nightsky.height/2));

	vect scal1=vect(Point(0,0,0),moving_obstacle[0].length/2,moving_obstacle[0].width/2,moving_obstacle[0].height/2);
//	for(int i=0;i<player.P.cur.size();i++)
//		print(player.P.cur[i]);

//	print("surface");
//	print(surface_centre);
	player.scale(vect(Point(0,0,0),player.length/2,player.width/2,player.height/2));
	player.rotate_about_own(angle);


//	player.rotate_about_own(45,glm::vec3(0,0,1));
	for(int i=-x/2,t=0,m=0;t<x;i++,m++,t++)
	{
		for(int j=-y/2,n=0;n<y;j++,n++)
		{
			if(mark[m][n]%2==0)
			{
			
				tiles[cnt].scale(scal);
				tiles[cnt].translateby(vect(Point(0,0,0),i*scal.x*2,j*scal.y*2,0));
				cnt++;
			}
		
			else
			{

				death_tiles[cnt1].scale(scal);
				death_tiles[cnt1].translateby(vect(Point(0,0,0),i*scal.x*2,j*scal.y*2,0));
				cnt1++;
			}
			ypos[n]=j*scal.y*2;
			if(obs[m][n]==1)
			{

				obstacle[cnt2].scale(scal);
				obstacle[cnt2].translateby(vect(Point(0,0,0),i*scal.x*2,j*scal.y*2,tiles_height/2+obstacle[cnt2].height/2));
				cnt2++;
			}
			if(jump[m][n]==1)
			{
		
				moving_obstacle[cnt3].scale(scal1);
				moving_obstacle[cnt3].translateby(vect(Point(0,0,0),i*scal.x*2,j*scal.y*2,tiles_height/2+obstacle[cnt2].height/2));
				cnt3++;
			}
		}
		xpos[m]=i*scal.x*2;
	}
//	player.pos.x=xpos[cur_x];
//	player.pos.y=ypos[cur_y];
//	player.translateby(vect(Point(0,0,0),xpos[cur_x],ypos[cur_y],player.pos.z));
}
void drawGrid()
{
	for(int i=0;i<tiles.size();i++)
		tiles[i].draw();
	for(int i=0;i<death_tiles.size();i++)
		death_tiles[i].draw();
	for(int i=0;i<obstacle.size();i++)
		obstacle[i].draw();
	for(int i=0;i<moving_obstacle.size();i++)
		moving_obstacle[i].draw();
	nightsky.draw();
	
}
void createPlayer()
{
GLuint textureID = createTexture("block.jpeg");

if(textureID == 0 )
		cout << "SOIL loading error: '" << SOIL_last_result() << "'" << endl;
/*  float v[] = {
      -1.0f,-1.0f,-1.0f, // triangle 1 : begin
     -1.0f,-1.0f, 1.0f,
      -1.0f, 1.0f, 1.0f, // triangle 1 : end
      1.0f, 1.0f,-1.0f, // triangle 2 : begin
      -1.0f,-1.0f,-1.0f,
      -1.0f, 1.0f,-1.0f, // triangle 2 : end
     1.0f,-1.0f, 1.0f,
     -1.0f,-1.0f,-1.0f,
     1.0f,-1.0f,-1.0f,
     1.0f, 1.0f,-1.0f,
     1.0f,-1.0f,-1.0f,
     -1.0f,-1.0f,-1.0f,
     -1.0f,-1.0f,-1.0f,
    -1.0f, 1.0f, 1.0f,     
    -1.0f, 1.0f,-1.0f,
     1.0f,-1.0f, 1.0f,
     -1.0f,-1.0f, 1.0f,
     -1.0f,-1.0f,-1.0f,
     -1.0f, 1.0f, 1.0f,
     -1.0f,-1.0f, 1.0f,
     1.0f,-1.0f, 1.0f,
     1.0f, 1.0f, 1.0f,
     1.0f,-1.0f,-1.0f,
     1.0f, 1.0f,-1.0f,
     1.0f,-1.0f,-1.0f,
     1.0f, 1.0f, 1.0f,
     1.0f,-1.0f, 1.0f,
     1.0f, 1.0f, 1.0f,
     1.0f, 1.0f,-1.0f,
     -1.0f, 1.0f,-1.0f,
     1.0f, 1.0f, 1.0f,
     -1.0f, 1.0f,-1.0f,
     -1.0f, 1.0f, 1.0f,
     1.0f, 1.0f, 1.0f,
     -1.0f, 1.0f, 1.0f,
     1.0f,-1.0f, 1.0f
 };*/
float v[] = {

     -1.0f,-1.0f,-1.0f, // triangle 1 : begin

     -1.0f,-1.0f, 1.0f,

     -1.0f, 1.0f, 1.0f, // triangle 1 : end

     1.0f, 1.0f,-1.0f, // triangle 2 : begin

     -1.0f,-1.0f,-1.0f,

     -1.0f, 1.0f,-1.0f, // triangle 2 : end

     1.0f,-1.0f, 1.0f,

     -1.0f,-1.0f,-1.0f,

     1.0f,-1.0f,-1.0f,

     1.0f, 1.0f,-1.0f,

     1.0f,-1.0f,-1.0f,

     -1.0f,-1.0f,-1.0f,

     -1.0f,-1.0f,-1.0f,

     -1.0f, 1.0f, 1.0f,

     -1.0f, 1.0f,-1.0f,

     1.0f,-1.0f, 1.0f,

     -1.0f,-1.0f, 1.0f,

     -1.0f,-1.0f,-1.0f,

     -1.0f, 1.0f, 1.0f,

     -1.0f,-1.0f, 1.0f,

     1.0f,-1.0f, 1.0f,

     1.0f, 1.0f, 1.0f,

     1.0f,-1.0f,-1.0f,

     1.0f, 1.0f,-1.0f,

     1.0f,-1.0f,-1.0f,

     1.0f, 1.0f, 1.0f,

     1.0f,-1.0f, 1.0f,

     1.0f, 1.0f, 1.0f,

     1.0f, 1.0f,-1.0f,

     -1.0f, 1.0f,-1.0f,

     1.0f, 1.0f, 1.0f,

     -1.0f, 1.0f,-1.0f,

     -1.0f, 1.0f, 1.0f,

     1.0f, 1.0f, 1.0f,

     -1.0f, 1.0f, 1.0f,

     1.0f,-1.0f, 1.0f

 };
float c[] = {
    255.0,0.0,0.0,
    255.0,0.0,0.0,
    255.0,102.0,0.0,
    0.0,0.0,51.0,
    153.0,153.0,255.0,
    0.0,0.0,51.0,
    0.0,0.0,255.0,
    0.0,0.0,255.0,
    0.0,0.0,255.0,
    0.0,0.0,51.0,
    153.0,153.0,255.0,
    153.0,153.0,255.0,
    255.0,0.0,0.0,
    255.0,102.0,0.0,
    255.0,102.0,0.0,
    0.0,0.0,255.0,
    0.0,0.0,255.0,
    0.0,0.0,255.0,
    255.0,255.0,0.0,
    255.0,255.0,0.0,
    255.0,255.0,0.0,
    0.0,255.0,0.0,    
    0.0,255.0,0.0,    
    0.0,255.0,0.0,    
    0.0,255.0,0.0,    
    0.0,255.0,0.0,    
    0.0,255.0,0.0,    
    0.0,0.0,0.0,
    0.0,0.0,0.0,
    0.0,0.0,0.0,
    0.0,0.0,0.0,
    0.0,0.0,0.0,
    0.0,0.0,0.0,
    255.0,255.0,0.0,
    255.0,255.0,0.0,
    255.0,255.0,0.0,
 };


	


 player.length=4.5;
 player.width=4.5;
 player.height=4.5;
 vector<float> vec (v, v + sizeof(v) / sizeof(v[0]) );
 vector<float>color;
 int var=0;
 for(int i=0;i<108;i++)
{
	if(i%2==0)
	{
		var++;
	}
	color.push_back(var);

 }
 player.P.createVertexBuffer(vec);
 player.createTextureBuffer();
// player.createColorBuffer(color);
 player.createFigure(textureID);
 player.v.set(vect(Point(0,0,0),0,0,0));
 player.a.set(vect(Point(0,0,0),0,0,0));
 player.pos.z=player.height/2+tiles_height/2;
player.pos.x=-16*player.length;
 player.pos.y=8*player.width;
}

void createGrid()
{

GLuint textureID = createTexture("grid.jpg");
GLuint textureID1 = createTexture("moving.jpg");
 float v[] = {
      -1.0f,-1.0f,-1.0f, // triangle 1 : begin
     -1.0f,-1.0f, 1.0f,
      -1.0f, 1.0f, 1.0f, // triangle 1 : end
      1.0f, 1.0f,-1.0f, // triangle 2 : begin
      -1.0f,-1.0f,-1.0f,
      -1.0f, 1.0f,-1.0f, // triangle 2 : end
     1.0f,-1.0f, 1.0f,
     -1.0f,-1.0f,-1.0f,
     1.0f,-1.0f,-1.0f,
     1.0f, 1.0f,-1.0f,
     1.0f,-1.0f,-1.0f,
     -1.0f,-1.0f,-1.0f,
     -1.0f,-1.0f,-1.0f,
    -1.0f, 1.0f, 1.0f,     
    -1.0f, 1.0f,-1.0f,
     1.0f,-1.0f, 1.0f,
     -1.0f,-1.0f, 1.0f,
     -1.0f,-1.0f,-1.0f,
     -1.0f, 1.0f, 1.0f,
     -1.0f,-1.0f, 1.0f,
     1.0f,-1.0f, 1.0f,
     1.0f, 1.0f, 1.0f,
     1.0f,-1.0f,-1.0f,
     1.0f, 1.0f,-1.0f,
     1.0f,-1.0f,-1.0f,
     1.0f, 1.0f, 1.0f,
     1.0f,-1.0f, 1.0f,
     1.0f, 1.0f, 1.0f,
     1.0f, 1.0f,-1.0f,
     -1.0f, 1.0f,-1.0f,
     1.0f, 1.0f, 1.0f,
     -1.0f, 1.0f,-1.0f,
     -1.0f, 1.0f, 1.0f,
     1.0f, 1.0f, 1.0f,
     -1.0f, 1.0f, 1.0f,
     1.0f,-1.0f, 1.0f
 };
 float c[] = {
    255.0,0.0,0.0,
    255.0,0.0,0.0,
    255.0,102.0,0.0,
    0.0,0.0,51.0,
    153.0,153.0,255.0,
    0.0,0.0,51.0,
    0.0,0.0,255.0,
    0.0,0.0,255.0,
    0.0,0.0,255.0,
    0.0,0.0,51.0,
    153.0,153.0,255.0,
    153.0,153.0,255.0,
    255.0,0.0,0.0,
    255.0,102.0,0.0,
    255.0,102.0,0.0,
    0.0,0.0,255.0,
    0.0,0.0,255.0,
    0.0,0.0,255.0,
    255.0,255.0,0.0,
    255.0,255.0,0.0,
    255.0,255.0,0.0,
    0.0,255.0,0.0,    
    0.0,255.0,0.0,    
    0.0,255.0,0.0,    
    0.0,255.0,0.0,    
    0.0,255.0,0.0,    
    0.0,255.0,0.0,    
    102.0,.0,0.0,
    102.0,0.0,0.0,
    102.0,0.0,0.0,
    102.0,0.0,0.0,
    102.0,0.0,0.0,
    102.0,0.0,0.0,
    255.0,255.0,0.0,
    255.0,255.0,0.0,
    255.0,255.0,0.0,
 };
 tiles_length=10;
 tiles_width=6;
 tiles_height=6;

vector<float> vec (v, v + sizeof(v) / sizeof(v[0]) );
 vector <float> color (c,c+sizeof(c)/sizeof(c[0]));
 color.clear();
 for(int i=0;i<108;i++)
 {

	 if(i%3==1)
		 color.push_back(255);
	 else
		 color.push_back(0);
 }
 vector <float>color1;
 color1.clear();
 for(int i=0;i<108;i++)
 {
	 
	 if(i%3==0)
		 color1.push_back(255);
	 else
		 color1.push_back(0);
 }
 print("size");
 int rd=0;
 for(int i=0;i<numberOfTiles;i++)
 {
	 
	 Figure t;
/*	for(int i=0;i<108;i++)
 	{
		color.push_back(rd);
 	}
	rd+=10;
	color.clear();*/

	 t.P.createVertexBuffer(vec);
	 
	 	t.createTextureBuffer();
	 if(index_mov[i]==1)
	 
 		 t.createColorBuffer(color1);
	 else
		 t.createColorBuffer(color);
	 if(index_mov[i]==0)
 	 t.createFigure(textureID1);
	else
		t.createFigure(textureID);
	 tiles.push_back(t);

 	tiles[i].length=tiles_length;
 	tiles[i].width=tiles_width;
 	tiles[i].height=tiles_height;
 	tiles[i].pos=Point(0,0,0);
	if(index_mov[i]==1)
	{
	tiles[i].v.set(vect(Point(0,0,0),0,0,0.3));
	tiles[i].a.set(vect(Point(0,0,0),0,0,0));
	}
	else
	tiles[i].v.set(vect(Point(0,0,0),0,0,0));
        tiles[i].a.set(vect(Point(0,0,0),0,0,0));
}

 GLuint 
 textureID3 = createTexture("water.jpg");

 color.clear();
 for(int i=0;i<108;i++)
 {
	 if(i%3==2)
		 color.push_back(255);
	 else
		 color.push_back(0);
 }
 for(int i=0;i<144-numberOfTiles;i++)
 {
	 Figure t;
	 t.P.createVertexBuffer(vec);
	t.createTextureBuffer();
	 t.createColorBuffer(color);
 	 t.createFigure(textureID3);
	 death_tiles.push_back(t);
	
	 death_tiles[i].length=tiles_length;
	death_tiles[i].width=tiles_width;
	death_tiles[i].height=tiles_height;

 }


 textureID3 = createTexture("nightsky.jpg");
 nightsky.P.createVertexBuffer(vec);
 nightsky.createTextureBuffer();
// nightsky.createColorBuffer(color);
 nightsky.createFigure(textureID3);
 nightsky.length=200;
 nightsky.width=200;
 nightsky.height=200;


}
void createCircle()
{
	circle.P.drawCircle(Point(700,799,0),20);
	circle.setFixedColor(125,124,124);
	circle.createPolygonFigure();
}
void checkCirclePolygon(Figure &circle,Figure &polygon)
{
	
	Point centre=circle.P.centroid_cur;
	for(int i=0;i<polygon.P.sides_cur.size();i++)
	{
		if(polygon.P.sides_cur[i].distance(centre)<circle.P.rad && polygon.P.sides_cur[i].Point_in_segment(centre)==true)
		{
			vect t;
			float a=circle.v.angle;
			float b=polygon.P.sides_cur[i].angle;
			float e=polygon.e;
			float along_line=circle.v.component_along(polygon.P.sides_cur[i]);
			float per_line=circle.v.component_perpendicular(polygon.P.sides_cur[i]);
			float x=along_line*cos(b)+e*per_line*sin(b);
			float y=along_line*sin(b)-e*per_line*cos(b);
			circle.v.set(vect(Point(0,0,0),x,y,0));
			break;
		}
	}
}
void CheckCollision(Figure &a,Figure &b)
{
	if((a.P.type=="circle" && b.P.type=="polygon")||(a.P.type=="polygon" && b.P.type=="circle"))
	{
		if(a.P.type=="circle")
		checkCirclePolygon(a,b);
		else
			checkCirclePolygon(b,a);
	}

	
	//Check if rectangle is solidGround
/*	if(rect.e==-1)
	{
		if(intersect_Circle_UnRotatedRectangle(circle,rect)==true)
		{

		}
	}*/

}
bool intersect_Circle_UnRotatedRectangle(Figure circle,Figure rect) //Circle and Polygon
{
    Point rect_centre=rect.P.centroid_cur;
    Point circleDistance;
    circleDistance.x = fabs(circle.P.rad - rect_centre.x);
    circleDistance.y = fabs(circle.P.rad - rect_centre.y);
    float width=rect.P.cur[1].x-rect.P.cur[0].x;
    float height=rect.P.cur[2].y-rect.P.cur[1].y;

    if (circleDistance.x > (width/2 + circle.P.rad)) { return false; }
    if (circleDistance.y > (height/2 + circle.P.rad)) { return false; }

    if (circleDistance.x <= (width/2)) { return true; } 
    if (circleDistance.y <= (height/2)) { return true; }

    float x=(circleDistance.x -width/2);
    float y=(circleDistance.y - height/2);
    float cornerDistance_sq=x*x+y*y;
    return (cornerDistance_sq <= (circle.P.rad*circle.P.rad));
}




/**************************
 * Customizable functions *
 **************************/
float triangle_rot_dir = 1;
float rectangle_rot_dir = 1;
bool triangle_rot_status = true;
bool rectangle_rot_status = true;
/*void drawCloud()
{
	for(int i=0;i<5;i++)
	{
		cloud.push_point(100,600,0);
		cloud.push_point(140,640,0);
		cloud.push_point(110,680,0);
		cloud.push_point(70,660,0);
		cloud.push_point(
	}
* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */

bool space=false;
int increments=1;
float dx,dy;
float f=1;
bool adventure=false;
bool helicopter=false;
bool follow=false;
bool rotation=false;
int sign=0;
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
     // Function is called first on GLFW_PRESS.
float val1=0.3,val2=0.4,val3=0.1,val4=3;
if(adventure)
{
	val1=0.05;
	val2=0.2;
	val3=0.03;
	val4=2;
}

    if (action == GLFW_RELEASE) {
        switch (key) {
            case GLFW_KEY_C:
                rectangle_rot_status = !rectangle_rot_status;
                
		break;
            case GLFW_KEY_P:
                triangle_rot_status = !triangle_rot_status;
                break;
         //   case GLFW_KEY_X:
	    case GLFW_KEY_SPACE:
		space=false;
		
			    if(increments>30)
				    increments=3;
			    else if(increments>20)
				    increments=2;
			   else
				   increments=1;
//		if(0)
//		{
//			print("wtf");
			if(player.a.z==0)
			{
				player.v.set(vect(Point(0,0,0),0,0,0.6*increments));
				player.a.set(vect(Point(0,0,0),0,0,-val3));
			}
//		}
/*		print(increments);
		increments/=10;
		print("inc");
		print(increments);
		player.v.set(vect(Point(0,0,0),0,0,2*increments));
		player.a.set(vect(Point(0,0,0),0,0,-0.4));
*/
                // do something ..
                break;
	case GLFW_KEY_R:
		rotation=false;
		break;
	case GLFW_KEY_L:
		rotation=false;
		break;
	case GLFW_KEY_RIGHT:
		dx=0;
		break;
	case GLFW_KEY_LEFT:
		dx=0;
		break;
	case GLFW_KEY_UP:
		dy=0;
		break;

	case GLFW_KEY_DOWN:
		dy=0;
		break;
            default:
                break;
        }
    }
    else if (action == GLFW_PRESS) {
        switch (key) {

            case GLFW_KEY_RIGHT:
		    if(space==true)
		  {
			//  print(increments);
			    if(increments>30)
				    increments=3;
			    else if(increments>20)
				    increments=2;
			   else
				   increments=1;
			   

			player.v.set(vect(Point(0,0,0),val1*increments*cos(angle*M_PI/180),val1*increments*sin(angle*M_PI/180),val2*increments));
			player.a.set(vect(Point(0,0,0),0,0,-val3));

		      // player.pos.x+=3*increments;;
		       increments=0;
		 }
		    player.pos.x+=val4*f*cos(angle*M_PI/180);
		    player.pos.y+=val4*f*sin(angle*M_PI/180);

		    dx=1;
			break;
	    case GLFW_KEY_DOWN:
		
		    if(space==true)
		    {

			    if(increments>30)
				    increments=3;
			    else if(increments>20)
				    increments=2;
			   else
				   increments=1;
			
			player.v.set(vect(Point(0,0,0),val1*increments*sin(angle*M_PI/180),-val1*increments*cos(angle*M_PI/180),val2*increments));
			player.a.set(vect(Point(0,0,0),0,0,-val3));

		      // player.pos.x+=3*increments;;
		       increments=0;
		    }
		    else
		    {

			    player.pos.x+=val4*f*sin(angle*M_PI/180);
			    player.pos.y-=val4*f*cos(angle*M_PI/180);
		    
		    }
			dy=-1;
			break;

	    case GLFW_KEY_UP:

		    if(space==true)
		    {

			    if(increments>30)
				    increments=3;
			    else if(increments>20)
				    increments=2;
			   else
				   increments=1;
			player.v.set(vect(Point(0,0,0),-val1*increments*sin(angle*M_PI/180),val1*increments*cos(angle*M_PI/180),val2*increments));
			player.a.set(vect(Point(0,0,0),0,0,-val3));

		      // player.pos.x+=3*increments;;
		       increments=0;
		       dy=1;
		    }

		    else
		    {
			    print(angle);
			    player.pos.x-=val4*f*sin(angle*M_PI/180);
			    player.pos.y+=val4*f*cos(angle*M_PI/180);
		    }
			dy=1;
			 break;
	    case GLFW_KEY_LEFT:

		    if(space==true)
		    {

			    if(increments>30)
				    increments=3;
			    else if(increments>20)
				    increments=2;
			   else
				   increments=1;
			player.v.set(vect(Point(0,0,0),-val1*increments*cos(angle*M_PI/180),-val1*sin(angle*M_PI/180),val2*increments));
			player.a.set(vect(Point(0,0,0),0,0,-val3));

		      // player.pos.x+=3*increments;r
		       increments=0;
		       dx=-1;
		    }
		    else
		    {
					print(angle);
			    player.pos.x-=val4*f*cos(angle*M_PI/180);
		    player.pos.y-=val4*f*sin(angle*M_PI/180);

//		    	 player.pos.x-=3*f;
		    }


			dx=-1;
			 break;
	    
//			 quit(window);
                break;
	   case GLFW_KEY_B:
		follow=true;
		adventure=false;
		break;

	   case GLFW_KEY_R:
		rotation=true;
		sign=1;
		break;
	   case GLFW_KEY_L:
		rotation=true;
		sign=-1;
		break;
            case GLFW_KEY_F:
		f=2;
		break;
	    case GLFW_KEY_T:
		follow=false;
		adventure=false;
		eyex=0;
		eyey=0;
		eyez=40;
		targetx=0.1;
		targety=0.1;
		targetz=0;
		break;
	    case GLFW_KEY_M:
		follow=false;
		adventure=false;
		eyex=0;
		eyez=40;
		eyey=-40;
		targetx=0;
		targety=0;
		targetz=0;
		break;
	    case GLFW_KEY_A:
	
		follow=false;
		adventure=true;
		break;
	    case GLFW_KEY_SPACE:
		if(dx!=0 || dy!=0)
		{
//			print("dsas");
//			print(dx);
//			print(dy);
			if(fabs(dx)>20)
			{

			
				if(dx<0)
					dx=-1;
				else
					dx=1;
			}
			else if(fabs(dx)>10)
			{
				if(dx<0)
					dx=-0.5;
				else
					dx=0.5;

			}
			else dx=0;

			if(fabs(dy)>20)
			{

				if(dy<0)
					dy=-1;
				else
					dy=1;
			}
			else if(fabs(dy)>10)
			{
				if(dy>0)
					dy=1;
				else
					dy=-1;
			}
			else dy=0;
			player.v.set(vect(Point(0,0,0),1.5*dx,1.5*dy,fabs((dx+dy)*3.4)));
			player.a.set(vect(Point(0,0,0),0,0,-0.8));
			dx=0;
			dy=0;
		}
		else if(player.a.z==0)

		{	player.v.set(vect(Point(0,0,0),0,0,1));
			player.a.set(vect(Point(0,0,0),0,0,-0.1));
		}
		space=true;
		break;
	    default:
                break;

        }
    }
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
	switch (key) {
		case 'Q':
		case 'q':
            quit(window);
            break;
		default:
			break;
	}
}
bool in_rect=false;
Point mouse_prev,mouse_cur;
/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
    switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            if (action == GLFW_RELEASE)
	    {
		    cout<<"no\n";
		    helicopter=false;;
 		rectangle_rot_status=false;
	      in_rect=false;
	    }
	    else
	    {
		    follow=false;
		    adventure=false;

		    helicopter=true;
		    double x,y;
		    glfwGetCursorPos(window,&x,&y);
		    mouse_prev.x=x;
		    mouse_prev.y=800-y;

	    }
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            if (action == GLFW_RELEASE) {
                rectangle_rot_dir *= -1;
            }
	    
	    
		    helicopter=true;
		    double x,y;
		    glfwGetCursorPos(window,&x,&y);
		    mouse_prev.x=x;
		    mouse_prev.y=800-y;
		   
	    
            break;
	   
        default:
            break;
    }
}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
    int fbwidth=width, fbheight=height;
    /* With Retina display on Mac OS X, GLFW's FramebufferSize
     is different from WindowSize */
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);

	GLfloat fov = 90.0f;

	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

	// set the projection matrix as perspective
	/* glMatrixMode (GL_PROJECTION);
	   glLoadIdentity ();
	   gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
	// Store the projection matrix in a variable for future use
    // Perspective projection for 3D views
     Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

    // Ortho projection for 2D views
//    Matrices.projection = glm::ortho(0.0f, 1000.0f, 0.0f, 800.0f, 0.1f, 500.0f);
}
VAO *triangle, *rectangle;
// Creates the triangle object used in this sample code
  /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

  /* Define vertex array as used in glBegin (GL_TRIANGLES) */
/*  static const GLfloat vertex_buffer_data [] = {
    0, 1,0, // vertex 0
    -1,-1,0, // vertex 1
    1,-1,0, // vertex 2
  };

  static const GLfloat color_buffer_data [] = {
    1,0,0, // color 0
    0,1,0, // color 1
    0,0,1, // color 2
  };*/
  

  // create3DObject creates and returns a handle to a VAO that can be used later
void createRectangle (float x1,float y1,float horiz,float vert)
{
	GLfloat x2,y2,x3,y3,x4,y4;
	x2=x1;
	y2=y1+vert;
	x3=x2+horiz;
	y3=y2;
	x4=x1+horiz;
  // GL3 accepts only Triangles. Quads are not supported
  static const GLfloat vertex_buffer_data [] = {
    x1,y1,0,
    x2,y2,0,// vertex 1
    x4,y4,0, // vertex 2
    x4,y4,0, // vertex 3
    x3,y3,0, // vertex 4
    x2,y2,0 
  };

  static const GLfloat color_buffer_data [] = {
    1,0,0, // color 1
    0,0,1, // color 2
    0,1,0, // color 3

    0,1,0, // color 3
    0.3,0.3,0.3, // color 4
    
    1,0,0  // color 1
  };



  // create3DObject creates and returns a handle to a VAO that can be used later
}

float camera_rotation_angle = 45;
float rectangle_rotation = 0;
float triangle_rotation = 0;
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if(yoffset==1)
	{
		preveye.x*=0.9;
		preveye.y*=0.9;
		preveye.z*=0.9;

		eyex=preveye.x;
		eyey=preveye.y;
		eyez=preveye.z;
	}
	else
	{
		preveye.x*=1.1;
		preveye.y*=1.1;
		preveye.z*=1.1;
		eyex=preveye.x;
		eyey=preveye.y;
		eyez=preveye.z;


	}
		

}    
void draw ()
{


  // clear the color and depth in the frame buffer
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // use the loaded shader program
  // Don't change unless you know what you are doing
  glUseProgram (programID);

   glUseProgram(textureProgramID);
  // Eye - Location of camera. Don't change unless you are sure!!
//  glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 5*sin(camera_rotation_angle*M_PI/180.0f) ,5);
  glm::vec3 eye(eyex,eyey,eyez);
// Target - Where is the camera looking at.  Don't change unless you are sure!!
  glm::vec3 target (targetx, targety, targetz);
  glm::vec3 up (0, 0, 1);
  if(player.pos.x>=6*player.width && player.pos.y<=ypos[0] && player.pos.z==player.height/2+tiles_height/2)
	  exit(0);
   if(helicopter)
   {
	   float angle1=0,angle2=0;
	   if(mouse_cur.x>mouse_prev.x)
	   	angle1+=0.8;
	   else if(mouse_cur.x<mouse_prev.x)
		   angle1-=0.8;
	   if(mouse_cur.y>mouse_prev.y)
		   angle2+=0.8;
	   else  angle2-=0.8;
	   mouse_prev=mouse_cur;
	   print(preveye);
	 //  angle2=0;
	glm::vec4 eye1(preveye.x,preveye.y,preveye.z,0);
	glm::mat4 rotatefigure1 = 
		glm::rotate((float)(angle1*M_PI/180.0f), glm::vec3(0,0,1));
	glm::vec4 n(1,0,0,0);
	glm::mat4 rotatefigure2=glm::rotate((float)(angle2*M_PI/180.0f), glm::vec3(1,0,0));
	glm::vec4 eyenew=rotatefigure1*rotatefigure2*eye1;
	print(eyenew.x);
	print(eyenew.y);
	print(eyenew.z);
	eyex=eyenew.x;
	eyey=eyenew.y;
	eyez=eyenew.z;
	preveye.x=eyex;
	preveye.y=eyey;
	preveye.z=eyez;
  }

/*  if(eyey==0)
  {
	  up.x=0;
	  up.y=-1;
	  up.z=0;
  }*/
  int x=0,y=0,z=0;
  int tx=0,ty=0,tz=0;
  x=0;
  y=0;
  z=1;
  tx=1;
  ty=1;
  tz=0;
  if(rotation)
  {
	  angle+=sign;
  }

  // Compute Camera matrix (view)
   Matrices.view = glm::lookAt( eye,target, up ); // Rotating Camera for 3D
  //  Don't change unless you are sure!!
//  Matrices.view = glm::lookAt(glm::vec3(x,y,z), glm::vec3(tx,ty,tz), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

  // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
  //  Don't change unless you are sure!!
   VP = Matrices.projection * Matrices.view;
 //  print("velocity");
 //  print(player.v);
   makeGrid();

 //  print(player.v);
 //  print(player.a);
   player.objectphysics();
   for(int i=0;i<numberOfTiles;i++)
   {
	   tiles[i].physics();
	   if(fabs(tiles[i].pos.z)>15)
	   {
		   
//		   if(player.pos.z-tiles[i].P.centroid_cur.z==player.height/2+tiles[i].height/2)
//		player.v.z*=-1;
		   
		   	tiles[i].v.z=-tiles[i].v.z;


	   }
	  
   }
   for(int i=0;i<numberOfObstacles;i++)
   {
	   obstacle[i].physics();
	   if(obstacle[i].pos.z<-10)
	   {
		   obstacle[i].v.z=-obstacle[i].v.z;
		
	   }
	   else if(obstacle[i].pos.z>0)
	   {

		   obstacle[i].v.z=-obstacle[i].v.z;
	   }

	   
   }
   for(int i=0;i<moving_obstacle.size();i++)
   {

   	   moving_obstacle[i].physics();
	   if(moving_obstacle[i].pos.z<0)
	   {
		 //  moving_obstacle[i].pos.z=tiles_height/2+moving_obstacle[i].height/2;;

		   moving_obstacle[i].translateTo(Point(moving_obstacle[i].P.centroid_cur.x,moving_obstacle[i].P.centroid_cur.y,tiles_height/2+moving_obstacle[i].height/2));
		  moving_obstacle[i].pos.z=0;
		  moving_obstacle[i].v.z=0.5*(i%2+0.8);
		  moving_obstacle[i].a.z=-0.02;
		  
	   }
   }
   for(int i=0;i<moving_obstacle.size();i++)
   {
	//   print(i);
	//   print(moving_obstacle[i].P.centroid_cur);
	//   print(player.P.centroid_cur);

	   if(CheckCubesCollision(player,moving_obstacle[i])==true)
	   {
		//   print("true");
		   player.pos.x=xpos[0];
		   player.pos.y=ypos[11];
		   player.pos.z=player.height/2+tiles_height/2;
		   player.translateTo(player.pos);
		   break;
	   }

   }
   
   for(int i=0;i<numberOfObstacles;i++)
   {
	   if(CheckCubesCollision(player,obstacle[i])==true)
	   {
		   player.pos.x=xpos[0];
		   player.pos.y=ypos[11];
		   player.pos.z=player.height/2+tiles_height/2;
		   player.translateTo(player.pos);
	   }

   }
   
  
 //  print("player1");
 //  print(player.pos);

  // for(int i=0;i<tiles.size();i++)
  // {
//	   print(i);
  // 	print(tiles[i].P.centroid_cur);
  // }2
   int flag=0;
   for(int i=0;i<numberOfTiles;i++)
   {
	if(CheckCubesCollision(player,tiles[i])==true && tiles[i].v.z!=0)
	{
//		print(i);
//		printsides(player);

//		print("tiles");
//		printsides(tiles[i]);
//		print("centroid_tiles");
//		print(tiles[i].P.centroid_cur);
//		print("centroid_player");
//		print(player.P.centroid_cur);
//	
		if(player.pos.z-tiles[i].P.centroid_cur.z>=tiles[i].height/2)
		{
			player.translateTo(Point(player.pos.x,player.pos.y,tiles[i].P.centroid_cur.z+tiles[i].height/2+player.height/2));
			player.pos=player.P.centroid_cur;
			player.v.x=0;
			player.a.x=0;
			player.v.y=0;
			player.a.y=0;
			player.v.z=tiles[i].v.z;
			player.a.z=0;
			if(tiles[i].v.z>0)
				flag=2;
			
//			print("he");


		}
		else if( player.pos.z-tiles[i].P.centroid_cur.z<-tiles[i].height/2)
		{
			
			print("dead");
			if(fabs(player.pos.y-tiles[i].P.centroid_cur.y)>tiles[i].width/2)
			{
				if(player.pos.y>tiles[i].P.centroid_cur.y)
					player.pos.y+=3;
				else
					player.pos.y-=3;
			}

			else if(fabs(player.pos.x-tiles[i].P.centroid_cur.x)>tiles[i].length/2)
			{
				if(player.pos.x>tiles[i].P.centroid_cur.x)
					player.pos.x+=3;
				else
					player.pos.x-=3;
			}
			else 
			{
				print("wtf");
				player.pos.x=xpos[0];
				player.pos.y=ypos[11];
				player.pos.z=tiles[i].height/2+player.height/2;
				player.translateTo(player.pos);
			}
			flag=1;

		}
		else
		{
//			print("ds'");
				player.v.x=-player.v.x;
		
				player.v.y=-player.v.y;
			
		
		
		flag=1;

		}
		
		break;
		
	}
  }

   if(player.pos.z<-100)
   {
	   player.pos.x=xpos[0];
	   player.pos.y=ypos[11];
	   player.pos.z=tiles_height/2+player.height/2;
	   player.translateTo(player.pos);
   }
   for(int i=0;i<numberOfTiles;i++)
   {
	   if(CheckCubesCollision(player,tiles[i]) && tiles[i].v.z==0)
	   {
		   if(flag==2)
			   break;
		   
	/*	   if(player..v.z>0)
		   {

			   break;

			   
		   }*/
		   player.v.z=0;
		   player.a.z=0;
		   player.a.x=0;
		   player.v.x=0;
		   player.v.y=0;
		   player.a.y=0;
		   player.translateTo(Point(player.pos.x,player.pos.y,player.height/2+tiles[i].height/2));
		   player.pos.z=player.height/2+tiles[i].height/2;
	   	   flag=1;
		   break;
	   }

   }
    if(flag==0&& player.a.z==0)
    {

	    player.v.z=-1;
	    player.a.z=0;

    }
   for(int i=0;i<death_tiles.size();i++)
   {
	if(CheckCubesCollision(player,death_tiles[i])==true)
	{
//		printsides(player);
//		printsides(tiles[i]);
			
	//   	player.pos.x=xpos[0];
	  // 	player.pos.y=ypos[11];
	   	player.pos.z=-5;
		player.v.z=-2;
	
	  // 	player.translateTo(player.pos);
//		player.objectphysics();
		

		break;
	}
  }
	if(space)
		increments++;
   if(adventure)
  computecentre();
   if(follow)
	   follow_on();
   
player.draw();

   drawGrid();
   if(dx!=0)
   {
	   if(dx>0)
		   dx++;
	   else
		   dx--;
   }
	   
   if(dy!=0)
   {
	   if(dy>0)
		   dy++;
	   else
		   dy--;
   }

	   

 

  //camera_rotation_angle++; // Simulating camera rotation
}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
    GLFWwindow* window; // window desciptor/handle

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval( 1 );

    /* --- register callbacks with GLFW --- */

    /* Register function to handle window resizes */
    /* With Retina display on Mac OS X GLFW's FramebufferSize
     is different from WindowSize */
    glfwSetFramebufferSizeCallback(window, reshapeWindow);
    glfwSetWindowSizeCallback(window, reshapeWindow);

    /* Register function to handle window close */
    glfwSetWindowCloseCallback(window, quit);

    /* Register function to handle keyboard input */
    glfwSetKeyCallback(window, keyboard);      // general keyboard input
    glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

    /* Register function to handle mouse click */
    glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks
	
    return window;
}


/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
	glActiveTexture(GL_TEXTURE0);
	// load an image file directly as a new OpenGL texture
	// GLuint texID = SOIL_load_OGL_texture ("beach.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_TEXTURE_REPEATS); // Buggy for OpenGL3
//	GLuint textureID = createTexture("beach2.png");
	// check for an error during the load process
	 
	
	textureProgramID = LoadShaders( "TextureRender.vert", "TextureRender.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.TexMatrixID = glGetUniformLocation(textureProgramID, "MVP");
	
	randomizeGrid();
	// check for an error during the load process
//	if(textureID == 0 )
//		cout << "SOIL loading error: '" << SOIL_last_result() << "'" << endl;;
	createObstacles();
	createGrid();
	createPlayer();
	createRect();




	
	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");

	
	reshapeWindow (window, width, height);

    // Background color of the scene
	glClearColor (0.3f, 0.3f, 0.3f, 0.0f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

    cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
    cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
    cout << "VERSION: " << glGetString(GL_VERSION) << endl;
    cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main (int argc, char** argv)
{

	int width = 1250;
	int height = 800;
	WIDTH_OF_WINDOW=width;
	HEIGHT_OF_WINDOW=height;
	preveye.x=0;
	preveye.y=-40;
	preveye.z=40;

    GLFWwindow* window = initGLFW(width, height);

	initGL (window, width, height);

    double last_update_time = glfwGetTime(), current_time;

    /* Draw in loop */
    while (!glfwWindowShouldClose(window)) {


        draw();
	double x,y;
	glfwGetCursorPos(window,&x,&y);
	mouse_cur.x=x;
	mouse_cur.y=800-y;

	glfwSetScrollCallback 	( 	window,
					scroll_callback
						); 	
        // Swap Frame Buffer in double buffering
        glfwSwapBuffers(window);

        // Poll for Keyboard and mouse events
        glfwPollEvents();

        // Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
        current_time = glfwGetTime(); // Time in seconds
        if ((current_time - last_update_time) >= 0.5) { // atleast 0.5s elapsed since last frame
            // do something every 0.5 seconds ..
            last_update_time = current_time;
        }
    }
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
