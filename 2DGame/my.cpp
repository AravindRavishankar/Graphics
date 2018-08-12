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
using namespace std;
vector<int>sevensegdecoder[10];

struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;

    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
} Matrices;

GLuint programID;

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
void print(string a)
{
	cout<<a<<endl;
}
glm::mat4 MVP;
glm::mat4 VP; //MVP = Projection * View * Model
class Point
{
	public:
	double x,y,z;
	Point(double x1=0,double y1=0,double z1=0)
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
		double x,y,z;
		double angle;
		//angle made by the vector with origin in radians
		vect(Point p=Point(0,0,0),double x1=0,double y1=0,double z1=0)
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
		vect rotateVector(float angle)
		{
			angle=(angle*M_PI)/180.0f;
			vect t;
			t.initial=initial;
			t.x=x*cos(angle)-y*sin(angle);
			t.y=x*sin(angle)+y*cos(angle);
			t.z=z;
			t.angle=computeAngle(y,x);
			return t;
		}


		
	

		
};
class Color
{
	public:
		double r,g,b;
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
string mouse_or_keyboard="keyboard";

class Polygon
{
	public :	
	vector <Point> initial;
	vector<float>vertices;
	vector <Point>cur;
	Point centroid_initial;
	Point centroid_cur;
	vector<vect>sides_initial;
	vector<vect>sides_cur;
	float rad;
	string type;
	Polygon()
	{
		rad=-1;
		initial.clear();
		vertices.clear();
		cur.clear();
		type="polygon";
	}
	void push_point(double x1,double y1,double z1)
	{
		Point t(x1,y1,z1);;
		initial.push_back(t);
		cur.push_back(t);
	}
	void add_vertices(Point q)
	{
		vertices.push_back(q.x);
		vertices.push_back(q.y);
		vertices.push_back(q.z);
	}
	void form_polygon()
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
			if(initial.size()==4)
			{
		//		print(i);
		//		print(initial[i]);
		//		print(initial[i+1]);
		//		print("wtf\n");
		//		print(sides_cur[i].angle*180/M_PI);
			}
		}
		centroid_initial.x=sum.x/initial.size();
		centroid_initial.y=sum.y/initial.size();
		centroid_initial.z=sum.z/initial.size();
		centroid_cur=centroid_initial;
		for(int i=0;i<initial.size();i++)
		{
			add_vertices(centroid_initial);
			add_vertices(initial[i]);
			add_vertices(initial[(i+1)%initial.size()]);
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
//		form_polygon();
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
	Polygon P;
	glm::mat4 transform;
	vect v,a;//position vector of Figure relative to initial coordinates
	Point pos;
	float e;
	vector<Color>color;
	vector<float>col;
	VAO *obj;
	Color centre;
	bool gradient;
	string fuck;
	Figure()
	{
		transform=glm::mat4(1.0f);
		color.clear();
		vect v1(Point(0,0,0),0,0,0);
		v=v1;
		a=v1;;
		gradient=true;
		e=1;
		fuck="fuck";
		pos=Point(0,0,0);
	}
	void push_color(int r,int g,int b)
	{
		Color c(r,g,b);
		color.push_back(c);
	}
	void add_color(Color c)
	{
		col.push_back(c.r);
		col.push_back(c.g);
		col.push_back(c.b);
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
	void setColor(int r,int g,int b)
	{
		for(int i=0;i<P.initial.size();i++)
		{
			push_color(r,g,b);
		}
		Color p(r,g,b);
		centre=p;
		gradient=false;
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
	void form_color()
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
		for(int i=0;i<color.size();i++)
		{
			add_color(centre);
			add_color(color[i]);
			add_color(color[(i+1)%color.size()]);
		}

	}
	void create()
	{
		P.form_polygon();
		pos=P.centroid_initial;
		form_color();
		const int sz1=P.vertices.size();
		const int sz2=col.size();
		int size=P.vertices.size()/3;
		GLfloat vertex_buffer[sz1],color_buffer[sz2];
		for(int i=0;i<sz1;i++)
		{
			vertex_buffer[i]=P.vertices[i];
		}
		for(int i=0;i<sz2;i++)
		{
			color_buffer[i]=col[i];
		}
		obj=create3DObject(GL_TRIANGLES,size ,vertex_buffer,color_buffer, GL_FILL);

	}
	void draw()
	{
		Matrices.model=glm::mat4(1.0f);
		Matrices.model*=transform;
		MVP=VP*Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
		draw3DObject(obj);
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
			glm::vec4 myVector(P.cur[i].x,P.cur[i].y,P.cur[i].z,index);
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
		pos=P.centroid_cur;
	}
	void objectphysics()
	{
		pos.update(Point(v.x,v.y,v.z));
//		print("pos_initial");
//		print(pos);
		v.translate(a);
//		print("pos_final");
//		print(pos);
		translateTo(pos);
	}
};
Point mouse_cur;
bool mouse_press=false;
bool space_press=false;
bool up=false;
bool down=false;
void convertMouse()
{
	mouse_cur.y=HEIGHT_OF_WINDOW-mouse_cur.y;
}

vector<Figure>sevenseg;
vector<Figure> Ball;
int thrownBalls=0;
Figure Ground;
vector<Figure>Circular_Obstacles;
Figure Sky;
Figure arc,hexagon,circle;
Figure topleft_rect;
vector <Figure> reg;
void debug(vector<vect> p)
{
	for(int i=0;i<p.size();i++)
		debug(p[i]);
}
void createTopLeftRect()
{
	topleft_rect.P.getRectangle(Point(30,700,0),100,40);
	topleft_rect.setColor(255,0,0);
	topleft_rect.create();
	topleft_rect.e=0.9;
}
void createSky()
{
	Sky.P.getRectangle(Point(0,0,0),WIDTH_OF_WINDOW,HEIGHT_OF_WINDOW);
	Sky.rectSetColor(103,166,151,7,81,106);
	Sky.create();
}
void createGround()
{
	Ground.P.getRectangle(Point(0,-10,0),WIDTH_OF_WINDOW,10);
	Ground.rectSetColor(52,34,34,69,50,35);
	Ground.create();
	Ground.e=0.9;
	Ground.fuck="ground";
}
vector<Figure>squares;
void createSquares()
{
	for(float i=0;i<9;i+=1)
	{
		Figure u;
		
		if(i==0)
		{

			u.P.getRegular(Point(150+200*i,300+100*i),40,6);
			u.setColor(20,36,69);
			u.setCentreColor(91,109,153);
		}

		else	if(i==1)
		{
			u.P.getRegular(Point(150+200*i,300+100*i),40,8);
			u.setCentreColor(178,71,16);
			u.setColor(218,104,12);
		}
		else if(i==2)
		{
			u.P.getRegular(Point(50+200*(i-1),170+100*(i-1),0),40,6);

			u.setColor(0,0,255);
		}
		else if(i==3)
		{
			int z=i-3;
			u.P.getRegular(Point(300+100*z,150+200*z),40,6);
			u.setColor(94,33,30);
			u.setCentreColor(255,104,125);
		}
		else  	u.P.getRegular(Point(300+100*i,150+200*i),30,6);
		if(i==4)
		{

			u.setColor(94,33,30);
			u.setCentreColor(255,104,125);
		}
		if(i==5)
		{
			u.setColor(255,0,0);
		}
		if(i==6)
		{	
			u.setCentreColor(178,71,16);
			u.setColor(218,104,12);
		}
		if(i==7)
		{
			u.setColor(20,36,69);
			u.setCentreColor(91,109,153);
		}
		if(i==8)
		{
			u.setColor(87,94,185);
			u.setCentreColor(4,8,94);

		}



		
	
		
			
		
		u.create();
		squares.push_back(u);
	}
}
void createObstacles()
{
	for(int i=0;i<1;i++)
	{
		Figure t;
		t.P.getArc(Point(100+200*i,100,1),40,0,180);
		t.setColor(139,0,0);
		t.setCentreColor(255,51,51);
		t.create();
		Circular_Obstacles.push_back(t);
	}
}
Figure Boundary1,Boundary2,Boundary3;
void drawBoundaryRectangles()
{
	Boundary1.P.getRectangle(Point(WIDTH_OF_WINDOW-250,0,0),100,HEIGHT_OF_WINDOW);
	Boundary1.setColor(0,0,0);
	Boundary1.create();
	Boundary1.e=1;

	Boundary1.e=1;
	Boundary2.P.getRectangle(Point(-50,0,0),50,HEIGHT_OF_WINDOW);
	Boundary2.setColor(0,0,0);
	Boundary2.create();

	Boundary2.e=1;
	Boundary3.P.getRectangle(Point(0,HEIGHT_OF_WINDOW,0),WIDTH_OF_WINDOW,50);
	Boundary3.setColor(0,0,0);
	Boundary3.create();
	Boundary3.e=1;
	
}
void createBall()
{
	for(int i=0;i<40;i++)
	{
		Figure r;
		r.P.drawCircle(Point(210,560,0),8);
		r.setColor(0,51,0);
		r.setCentreColor(0,255,0);
		r.v.set(vect(Point(0,0,0),0,0,0));
		r.a.set(vect(Point(0,0,0),0,0,0));
		r.create();
		Ball.push_back(r);
	}
}

vector <Figure> targets;
void createTargets()
{
	targets.clear();
	for(int i=0;i<10;i++)
	{
		Figure t;
		if(i>=4&& i<=8)

		t.P.drawCircle(Point(20,20,0),6);
		else
		t.P.drawCircle(Point(20,20,0),10);
		t.setColor(0,0,0);
		t.create();
		targets.push_back(t);
		
	}

}

void createhexagon()
{
	hexagon.P.getRegular(Point(WIDTH_OF_WINDOW/2-100,HEIGHT_OF_WINDOW/2+50),60,50);
	cout<<"adsa\n";

		hexagon.setColor(139,0,0);
		hexagon.setCentreColor(255,51,51);
//	hexagon.setCentreColor(0,51,0);
	cout<<"sada\n";
	arc.create();
	hexagon.create();
//	hexagon.v.set(vect(Point(0,0,0),0,0,0));
//	hexagon.a.set(vect(Point(0,0,0),0,-0.1,0));
	hexagon.e=1;
	cout<<"sadas\n";
}
void createReg()
{
	int ret=3;
	int middle=HEIGHT_OF_WINDOW/2;
	int xgap=200;
	int sign=1;
	for(int i=0;i<6;i++)
	{
		Figure t;
		if(i==0)
		t.P.getRectangle(Point(710,550,0),200,50);
		if(i==1)

		t.P.getRectangle(Point(290,150,0),500,50);
		if(i==2)
			t.P.getRectangle(Point(210,650,0),200,40);
		if(i==3)
		t.P.getRegular(Point(910,250,0),80,4);
		if(i==4)
		t.P.getRegular(Point(40,250,0),150,5);
		if(i==5)

			t.P.getRegular(Point(980,525,0),60,6);
		
		t.setColor(0,0,255);
		t.setCentreColor(0,51,0);
		t.e=1;
		t.create();
		reg.push_back(t);
	//	if(i>=3)
	//		sign=-1;
//		ret--;
		


	}
}
void createSevenseg()
{
		Figure t;
		t.P.getRectangle(Point(20,20,0),30,10);
		t.setColor(0,0,0);
		t.create();
		sevenseg.push_back(t);
		Figure u;
		u.P.getRectangle(Point(40,20,0),10,30);
		u.setColor(0,0,0);
		u.create();
		sevenseg.push_back(u);
		Figure z;
		z.P.getRectangle(Point(40,40,0),10,30);
		z.setColor(0,0,0);
		z.create();
		sevenseg.push_back(z);
		Figure y;
		y.P.getRectangle(Point(20,60,0),30,10);
		y.setColor(0,0,0);
		y.create();
		sevenseg.push_back(y);
		Figure w;
		w.P.getRectangle(Point(20,40,0),10,30);
		w.setColor(0,0,0);
		w.create();
		sevenseg.push_back(w);
		Figure p;
		p.P.getRectangle(Point(20,20,0),10,20);
		p.setColor(0,0,0);
		p.create();
		sevenseg.push_back(p);
		Figure m;
		m.P.getRectangle(Point(20,40,0),20,10);
		m.setColor(0,0,0);
		m.create();
		sevenseg.push_back(m);
}
Figure platform;
vector<Figure>cannon;
void drawCannon()
{
	Figure wheel;
	wheel.P.drawCircle(Point(20,190,0),20);
	wheel.setColor(28,28,28);

	wheel.setCentreColor(116,116,116);
	wheel.create();
	cannon.push_back(wheel);
	Figure cannonhead;
	cannonhead.P.getRectangle(Point(20,190,0),80,30);
	cannonhead.rectSetColor(27,65,57,83,157,142);
	cannonhead.create();
	cannon.push_back(cannonhead);
//	cannonhead.P.drawCircle(Point(40,120,0),30);




	
}
void convertToRadians(float &angle)
{
	angle=angle*M_PI/180.0;
	
}
void convertToDegrees(float &angle)
{
	angle=angle*180.0/M_PI;
}
float rotateCannonMouse()
{
	float angle=atan2(mouse_cur.y-cannon[0].P.centroid_cur.y,mouse_cur.x-cannon[0].P.centroid_cur.x);
	convertToDegrees(angle);
	if(angle<=0)
	{
		cannon[1].rotate_about_point(cannon[0].P.centroid_cur,0);
		angle=0;
	}
	else if(angle>=85)
	{
		cannon[1].rotate_about_point(cannon[0].P.centroid_cur,85);
		angle=85;
	}
	else 
		cannon[1].rotate_about_point(cannon[0].P.centroid_cur,angle);
	return angle;
}
float keyboard_angle=0;
float rotateCannonKeyBoard(float &angle)
{
	if(up==true)
		angle+=2;
	if(down==true)
		angle-=2;


	if(angle<=0)
		cannon[1].rotate_about_point(cannon[0].P.centroid_cur,0);
	else if(angle>=85)
		cannon[1].rotate_about_point(cannon[0].P.centroid_cur,85);
	else

		cannon[1].rotate_about_point(cannon[0].P.centroid_cur,angle);
	return angle;
	
	

}
float computeDistanceBetweenPoints(Point a,Point b)
{
	float ans1=0,ans2=0,ans3=0;
	ans1=a.x-b.x;
	ans1*=ans1;
	ans2=a.y-b.y;
	ans2*=ans2;
	ans3=a.z-b.z;
	ans3*=ans3;
	return sqrt(ans1+ans2+ans3);
}

vector<Figure>bar;
bool space_released=false;
float increments=0;
void Ballrotation(float angle)
{
	if((mouse_press==true&& mouse_or_keyboard=="mouse")||(space_released==true&& mouse_or_keyboard=="keyboard"))
	{
//		print("wyeah");
		vect v;
		v.initial=Point(0,0,0);
		float distance;
		if(mouse_or_keyboard=="mouse")
		 distance=computeDistanceBetweenPoints(mouse_cur,cannon[0].P.centroid_cur);
		else
			distance=50*increments;
//		distance=distance/2;
		print("wyeah");
		print(distance);
		increments=0;	
		v.x=(3*distance)/200;
		convertToRadians(angle);
		v.y=v.x*tan(angle);
		convertToDegrees(angle);
		v.z=0;
		Point t(cannon[1].P.initial[1].x,cannon[1].P.initial[1].y+Ball[thrownBalls].P.rad,0);
		Ball[thrownBalls].translateTo(t);
		if(angle<=0)
			Ball[thrownBalls].rotate_about_point(cannon[0].P.centroid_cur,0);
		else if(angle>=85)
			Ball[thrownBalls].rotate_about_point(cannon[0].P.centroid_cur,85);
		else 
			Ball[thrownBalls].rotate_about_point(cannon[0].P.centroid_cur,angle);
//		print(Ball[thrownBalls].P.centroid_cur);
		Ball[thrownBalls].v.set(v);
		Ball[thrownBalls].a.set(vect(Point(0,0,0),0,-0.3,0));
		thrownBalls++;
		mouse_press=false;
		space_released=false;
	}
}
void createBar()
{
   for(int i=0;i<700;i++)
   {
        Figure b;
        if(i==0)
	{
		b.P.getRectangle(Point(2,20,0),2,10);
		b.setColor(173,255,47);
		b.create();

	}
	else if(i<100)
	{

		b.P.getRectangle(Point(bar[i-1].P.initial[0].x+2,20,0),2,10);
		b.setColor(173,255,47);
		b.create();
		
	}
        else if(i>=100 && i<200)
	{
			
		b.P.getRectangle(Point(bar[i-1].P.initial[0].x+2,20,0),2,10);
		b.setColor(255,255,102);
		b.create();
	}
        else if(i>=200 && i<300)
	{

		b.P.getRectangle(Point(bar[i-1].P.initial[0].x+2,20,0),2,10);
		b.setColor(255,255,0);
		b.create();
	}

        else if(i>=300 && i<400)
	{

		b.P.getRectangle(Point(bar[i-1].P.initial[0].x+2,20,0),2,10);
		b.setColor(204,0,0);
		b.create();
	}
	else 
	{

		b.P.getRectangle(Point(bar[i-1].P.initial[0].x+2,20,0),2,10);
		b.setColor(255,0,0);
		b.create();

	}

        bar.push_back(b);
    }

}
void createPlatform()
{
	platform.P.getRectangle(Point(0,150,0),50,30);
	platform.rectSetColor(190,85,17,99,62,24);
	platform.create();
}
vector<Figure>octagon;
void createCircle()
{
	circle.P.drawCircle(Point(700,799,0),20);
	circle.setColor(125,124,124);
	circle.create();
}
bool  checkCirclePolygon(Figure &circle,Figure &polygon)
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
			along_line=e*along_line;
			float per_line=circle.v.component_perpendicular(polygon.P.sides_cur[i]);
			float x=along_line*cos(b)+e*per_line*sin(b);
			float y=along_line*sin(b)-e*per_line*cos(b);
			circle.v.set(vect(Point(0,0,0),x,y,0));
			return true;
		}

	}
	return false;
/*	for(int i=0;i<polygon.P.initial.size();i++)
	{
		if(computeDistanceBetweenPoints(polygon.P.cur[i],centre)<circle.P.rad)
		{
			print("corner");

			float a=circle.v.angle;
			vect t;
			t.form(centre,polygon.P.cur[i]);
			vect q=t.rotateVector(90);
			float b=q.angle;
			float e=polygon.e;
			float along_line=circle.v.component_along(q);
			float per_line=circle.v.component_perpendicular(q);
			float x=along_line*cos(b)+e*per_line*sin(b);
			float y=along_line*sin(b)-e*per_line*cos(b);
			circle.v.set(vect(Point(0,0,0),x,y,0));
			
		}
	}*/
}
bool CheckCollision(Figure &a,Figure &b)
{
//	if((a.P.type=="circle" && b.P.type=="polygon")||(a.P.type=="polygon" && b.P.type=="circle"))
//	{
//		if(a.P.type=="circle")
	return	checkCirclePolygon(a,b);
//		else
//			checkCirclePolygon(b,a);
//	}

	
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

    double x=(circleDistance.x -width/2);
    double y=(circleDistance.y - height/2);
    double cornerDistance_sq=x*x+y*y;
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
		cloud.piush_point(140,640,0);
		cloud.push_point(110,680,0);
		cloud.push_point(70,660,0);
		cloud.push_point(
	}
}*/
int zi=0,pi=0;
/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
     // Function is called first on GLFW_PRESS.

    if (action == GLFW_RELEASE) {
        switch (key) {
            case GLFW_KEY_C:
                rectangle_rot_status = !rectangle_rot_status;
                break;
            case GLFW_KEY_P:
                triangle_rot_status = !triangle_rot_status;
                break;
            case GLFW_KEY_X:
                // do something ..
                break;
	case GLFW_KEY_UP:
		up=false;
		break;
	case GLFW_KEY_DOWN:
		down=false;
		break;
	case GLFW_KEY_SPACE:
	{
		space_press=false;
		space_released=true;
	}
		break;
            default:
                break;
        }
    }
    else if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                quit(window);
                break;
		case GLFW_KEY_SPACE:
		space_press=true;
		break;
	    case GLFW_KEY_UP:
		if(mouse_or_keyboard=="keyboard")
		up=true;
	     break;
	    case GLFW_KEY_DOWN:
	     if(mouse_or_keyboard=="keyboard")
	    	down=true;
	     	break;
	    case GLFW_KEY_Z:
	if(mouse_or_keyboard=="keyboard")
		     mouse_or_keyboard="mouse";
	else
	{
		cannon[1].transform=glm::mat4(1.0f);
		mouse_or_keyboard="keyboard";
	}
	     break;
	case GLFW_KEY_W:
		zi+=1;
		zi=min(zi,2);
		break;
	case GLFW_KEY_S:
		zi-=1;
		zi=max(zi,-2);
		break;
	case GLFW_KEY_D:
		pi-=1;
		pi=max(-2,pi);
		break;
	case GLFW_KEY_A:
		pi+=1;
		pi=min(2,pi);
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
bool mouse_release=false;
bool gameRunning=false;
Point old;
/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
//	print("wtf");
    switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            if (action == GLFW_RELEASE)
	    {
		    cout<<"no\n";
 		 rectangle_rot_status=false;
	         in_rect=false;
		 gameRunning=true;
		mouse_press=false;;
		mouse_release=true;
		}
	    else
	    {

//		    print("yeah");
		    mouse_press=true;
		    
	    }
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            if (action == GLFW_RELEASE) {
                rectangle_rot_dir *= -1;
            }
	    else if(action==GLFW_PRESS)
	    {
	    }
            break;
        default:
            break;
    }
}
bool CheckCirclesCollision(Figure a,Figure b)
{
	float ans=computeDistanceBetweenPoints(a.P.centroid_cur,b.P.centroid_cur);
	if(ans<a.P.rad+b.P.rad)
		return true;
	return false;

}

/* Executewhen window is resized to 'width' and 'height' */
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
    // Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

    // Ortho projection for 2D views
    Matrices.projection = glm::ortho(0.0f, 1000.0f, 0.0f, 800.0f, 0.1f, 500.0f);
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

float camera_rotation_angle = 90;
float rectangle_rotation = 0;
float triangle_rotation = 0;
float angle2=0;
float angle=20;
float angle1=0;
float angle3=0;
float factor=1;
float state=1;
float state1=1;
bool visible[100];
int score=0;
void printscore(int score)
{
	int temp=score;
	int gap=0;
	while(1)
	{
		print(temp);
		for(int i=0;i<sevensegdecoder[temp%10].size();i++)
		{
			
			sevenseg[sevensegdecoder[temp%10][i]].translateby(vect(Point(0,0,0),900+gap,0,0));
			sevenseg[sevensegdecoder[temp%10][i]].draw();
		}
		temp/=10;
		gap-=40;
		if(!temp)

			break;
	}

}

void draw ()
{

    Matrices.projection = glm::ortho(0.0f+zi*30-pi*15, 1000.0f-zi*30-pi*15, 0.0f+zi*30, 800.0f-zi*30, 0.1f, 500.0f);

  // clear the color and depth in the frame buffer
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // use the loaded shader program
  // Don't change unless you know what you are doing
  glUseProgram (programID);

  // Eye - Location of camera. Don't change unless you are sure!!
  glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
  // Target - Where is the camera looking at.  Don't change unless you are sure!!
  glm::vec3 target (0, 0, 0);
  // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
  glm::vec3 up (0, 1, 0);

  // Compute Camera matrix (view)
  // Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
  //  Don't change unless you are sure!!
  Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

  // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
  //  Don't change unless you are sure!!
   VP = Matrices.projection * Matrices.view;

  // Send our transformation to the currently bound shader, in the "MVP" uniform
  // For each model you render, since the MVP will be different (at least the M part)
  //  Don't change unless you are sure!!

  // Load identity to model matrix
  
 /*  Matrices.model = glm::mat4(1.0f);


  MVP = VP * Matrices.model; //MVP = p * V * M

  //  Don't change unless you are sure!!
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);*/
//	draw3DObject(square.obj);
	
  
//  square.draw()
   
 // pentagon.rotate_about_point(angle,100,100);
 /* pentagon.rotate_about_own(angle);
  //rect.rotate_about_point(angle,pentagon.P.centroid_cur.x,pentagon.P.centroid_cur.y);
  pentagon.translate(pentagon.p);
  pentagon.update_position();
  pentagon.update_velocity();*/
 // print("dsaa");
//  print(atan2(0,400));
  Ground.draw();
  Sky.draw();
  for(int i=0;i<thrownBalls;i++)
  {
	  Ball[i].draw();
	
  }
  for(int i=0;i<targets.size();i++)
	  targets[i].draw();
  
  platform.draw();
  for(int i=0;i<cannon.size();i++)
  	 cannon[i].draw();

  float rotatedangle;
  if(mouse_or_keyboard=="mouse")
  {
	rotatedangle=rotateCannonMouse();  
	Ballrotation(rotatedangle);
  }
  else if(mouse_or_keyboard=="keyboard");
  {
  	rotatedangle=rotateCannonKeyBoard(keyboard_angle);
	Ballrotation(rotatedangle);
  }
 
/*  for(int i=0;i<thrownBalls;i++)
  {
	  if(abs(Ball[i].v.x)<0.2 && Ball[i].P.centroid_cur.y<=105+Ball[i].P.rad)
	  {
	  	Ball.erase(Ball.begin()+i);
		thrownBalls--;
	  }
  }*/
  for(int i=0;i<thrownBalls;i++)
  {
    Ball[i].objectphysics();
  }
/*Boundary collison*/
  for(int i=0;i<thrownBalls;i++)
  {
  	CheckCollision(Ball[i],Boundary1);
 	CheckCollision(Ball[i],Boundary2);
	CheckCollision(Ball[i],Boundary3);
	CheckCollision(Ball[i],Ground);
  } 
  for(int i=0;i<thrownBalls;i++)
  {
  	CheckCollision(Ball[i],hexagon);
  }
  for(int i=0;i<squares.size();i++)
  {
	  for(int j=0;j<thrownBalls;j++)
	  {
		  CheckCollision(Ball[j],squares[i]);
	  }
  }
 // for(int i=0;i<Circular_Obstacles.size();i++)
  //{
//	  for(int j=0;j<Ball.size();j++)
//	  CheckCollision(Ball[j],Circular_Obstacles[i]);

 // }
//  for(int i=0;i<Circular_Obstacles.size();i++)
//	 Circular_Obstacles[i].draw();

  for(int i=0;i<squares.size();i++)
  {
	 squares[i].draw();
  }
  for(int i=0;i<targets.size();i++)
	  if(visible[i])
	  targets[i].draw();

  
  hexagon.draw();
  hexagon.rotate_about_own(-angle);
  for(int i=0;i<4;i++)
  {
	  squares[i].rotate_about_point(hexagon.P.centroid_cur,(i%3+1)*angle);
	  squares[i].rotate_about_own((i%3+3)*angle);

  }
  float rad=30;

  for(int i=4;i<squares.size();i++)
  {
	  float x=hexagon.P.centroid_cur.x;
	  float y=hexagon.P.centroid_cur.y;
	  if(i>=7)

	  squares[i].translateTo(Point(x-50-100*(i-3),y+30*(3-i),0));
	  else
	  squares[i].translateTo(Point(x-50-100*(i-3),y+30*(i-3),0));
	  if(i==8)
	  {
		  squares[i].translateTo(Point(x+30*(i-3),y+40*(i-3),0));
	  }

	  squares[i].rotate_about_point(hexagon.P.centroid_cur,(i%3+1)*(-angle1));
	  squares[i].rotate_about_own((i%3+1)*(-angle1));
	  
  }

	  angle3=factor*2*angle;
  for(int i=0;i<targets.size()-1;i++)
  {
	  targets[i].translateTo(Point(squares[i].P.centroid_cur.x+squares[i].P.rad,squares[i].P.centroid_cur.y+squares[i].P.rad,0));
	  int z=i;
	  if(i>=4)
		  z=i%4;
	  targets[i].rotate_about_point(squares[i].P.centroid_cur,angle*(20*(z+1)+2));
	  targets[i].rotate_about_own(angle3);
  }
	  targets[targets.size()-1].translateTo(Point(hexagon.P.centroid_cur.x+hexagon.P.rad,hexagon.P.centroid_cur.y+hexagon.P.rad,0));

	  targets[targets.size()-1].rotate_about_point(hexagon.P.centroid_cur,angle*(20));
	 
	  targets[targets.size()-1].rotate_about_own(2*angle3);
	 
  for(int i=0;i<squares.size();i++)
  {
//	  print(i);
	  float a=computeDistanceBetweenPoints(targets[i].P.centroid_cur,squares[i].P.centroid_cur);
  //		print(a);
  }

  
  for(int i=0;i<targets.size();i++)
  {
	  int flag=0;
	  for(int j=0;j<thrownBalls;j++)
	  {
		  if(CheckCollision(Ball[j],targets[i])==true)
		  {
		//	  targets.erase(targets.begin()+i);
			  score++;
				factor+=2;


			  print("score");
			  print(score);
			  break;

		  }

	  }
	  if(flag)
		  break;
  }

			  printscore(score);
  
  topleft_rect.rotate_about_own(4*angle);

  float d=computeDistanceBetweenPoints(hexagon.P.cur[4],hexagon.P.centroid_cur);
  if(space_press==true && space_released==false)
  {
	  
	for(int i=0;i<20*increments;i++)
	{
		bar[i].draw();
	}
	increments++;
  }
  
 if(angle>=90)
{
	state=-1;
}
if(angle1>=30)
	state1=-1;
angle+=0.1*state;;
if(angle1<=-20)
	state1=1;
	angle1+=0.1*state1;

if(angle<=30)
	state=1;
  
 // CheckCollision(Ball,rect);
  

  //for(int i=0;i<reg.size();i++)
  //{
//	  CheckCollision(Ball,reg[i]);
//  }
 

  //rect2.draw();
  // draw3DObject draws the VAO given to it using current MVP matrix
 // //draw3DObject(tri.obj);
  //square.drawRegular();
  //
  // Pop matrix to undo transformations till last push matrix instead of recomputing model matrix
  // glPopMatrix ();
 

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
	createGround();
	createSky();
	createObstacles();
	createBall();
	createTopLeftRect();
	createhexagon();
//	createReg();
	createPlatform();
//	createRotatingSquare();
	drawCannon();
	drawBoundaryRectangles();
	createSquares();
	createTargets();
	createBar();
	for(int i=0;i<100;i++)
		visible[i]=true;
	

	for(int i=0;i<6;i++)
		sevensegdecoder[0].push_back(i);
	sevensegdecoder[1].push_back(4);
	sevensegdecoder[1].push_back(5);
	
	sevensegdecoder[2].push_back(0);
	sevensegdecoder[2].push_back(5);
	sevensegdecoder[2].push_back(6);
	sevensegdecoder[2].push_back(2);
	sevensegdecoder[2].push_back(3);

	sevensegdecoder[3].push_back(6);
	sevensegdecoder[3].push_back(3);
	sevensegdecoder[3].push_back(2);
	sevensegdecoder[3].push_back(1);
	sevensegdecoder[3].push_back(0);

	sevensegdecoder[4].push_back(1);
	sevensegdecoder[4].push_back(2);
	sevensegdecoder[4].push_back(6);
	sevensegdecoder[4].push_back(4);


	sevensegdecoder[5].push_back(0);
	sevensegdecoder[5].push_back(1);
	sevensegdecoder[5].push_back(6);
	sevensegdecoder[5].push_back(4);
	sevensegdecoder[5].push_back(3);
	
	
	sevensegdecoder[6].push_back(0);
	sevensegdecoder[6].push_back(1);
	sevensegdecoder[6].push_back(6);
	sevensegdecoder[6].push_back(4);
	sevensegdecoder[6].push_back(3);
	sevensegdecoder[6].push_back(5);
	
	sevensegdecoder[7].push_back(1);
	sevensegdecoder[7].push_back(2);
	sevensegdecoder[7].push_back(3);

	for(int i=0;i<7;i++)
		sevensegdecoder[8].push_back(i);
	
	sevensegdecoder[9].push_back(2);
	sevensegdecoder[9].push_back(3);
	sevensegdecoder[9].push_back(4);
	sevensegdecoder[9].push_back(6);
	sevensegdecoder[9].push_back(0);

	sevensegdecoder[9].push_back(1);
	createSevenseg();
	


	
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
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    if(yoffset==1)
	
		zi++;
    if(yoffset==-1)
	zi--;
}
int main (int argc, char** argv)
{

	int width = 1250;
	int height = 800;
	WIDTH_OF_WINDOW=width;
	HEIGHT_OF_WINDOW=height;

    GLFWwindow* window = initGLFW(width, height);

	initGL (window, width, height);

    double last_update_time = glfwGetTime(), current_time;

    /* Draw in loop */
    int flag=0;
    while (!glfwWindowShouldClose(window)) {


        draw();
	glfwGetCursorPos(window,&mouse_cur.x,&mouse_cur.y);
	convertMouse();
	// Swap Frame Buffer in double buffering
        glfwSwapBuffers(window);

        // Poll for Keyboard and mouse events
        glfwPollEvents();

glfwSetScrollCallback(window, scroll_callback);




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
