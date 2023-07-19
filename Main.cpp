#include "graphics.h"
#include "conio.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "iostream"
#include "vector"

int screenWidth = 640, screenHeight = 640;

struct Vertex {
	glm::vec4 position;
	COLORREF color;
	BYTE r, g, b;

	Vertex() {}
	Vertex(glm::vec4 position, COLORREF color) {
		this->position = position;
		this->color = color;
		this->r = GetRValue(color);
		this->g = GetGValue(color);
		this->b = GetBValue(color);
	}
};

struct Triangle {
	glm::vec3 index;
	COLORREF color;
};

struct Model {
	std::string name;
	std::vector<Vertex> vertices;
	std::vector<Triangle> triangles;
};

struct Transform {
	glm::vec3 translate;
	glm::vec4 rotation;
	glm::vec3 scale3D;
};

// 一个model 包含【转换】初始化条件
struct Instance {
	Model model;
	Transform transform;  // 在世界坐标的转换
};

// 场景包括许多 模型
struct Scene {
	std::vector<Instance> instance;
};

/**
* 插值算法
* (i0,d0),(i1,d1)
* i0到i1每一个标准步长对应的值，对应在d0到d1之间的值[i0,i1)
* 返回d0到d1之间值的集合
*/
std::vector<float> Interpolate(float i0, float d0, float i1, float d1) {
	std::vector<float> vec;
	if (glm::abs(i0 - i1) < 1e-6) {
		vec.push_back(d0);
		return vec;
	}
	float a = (d1 - d0) / (i1 - i0);
	float d = d0;
	for (float i = i0; i < i1; i++) {
		vec.push_back(d);
		d += a;
	}
	return vec;
}

void DrawLine(glm::vec2 P0, glm::vec2 P1, COLORREF color) {
	// 插值比每次计算tx,ty更快
	float dy = P0.y - P1.y;
	float dx = P0.x - P1.x;

	// 有两种情况需要注意：1.垂直的线；2.从右往左/从下往上的线
	if (dx == 0) {
		// 竖直
		if (P0.y > P1.y) std::swap(P0, P1);
		std::vector<float> xvec = Interpolate(P0.y, P0.x, P1.y, P1.x);
		for (float ty = P0.y; ty < P1.y; ty++) {
			putpixel(xvec[ty - P0.y], ty, color);
		}
	}
	else {
		if (P0.x > P1.x) std::swap(P0, P1);
		std::vector<float> yvec = Interpolate(P0.x, P0.y, P1.x, P1.y);
		for (float tx = P0.x; tx < P1.x; tx++) {
			putpixel(tx, yvec[tx - P0.x], color);
		}
	}
}

void DrawWireframeTriangle(glm::vec2 P0, glm::vec2 P1, glm::vec2 P2, COLORREF color) {
	DrawLine(P0, P1, color);
	DrawLine(P1, P2, color);
	DrawLine(P2, P0, color);
}

void DrawFilledTriangle(glm::vec2 P0, glm::vec2 P1, glm::vec2 P2, COLORREF color) {
	// 排序顶点：P0.y <= P1.y <= P2.y
	if (P0.y > P1.y) std::swap(P0, P1);
	if (P0.y > P2.y) std::swap(P0, P2);
	if (P1.y > P2.y) std::swap(P1, P2);

	//------------------P2|\
	//--------------------| \
	//--------------------|  \ P1
	//--------------------|  /
	//--------------------| /
	//------------------P0|/	
	// 对每条边的x求插值，因为起点(P0)终点(P1)相同，所以求出的插值(x)个数x02与x01/x12个数之和相同(x01/x12多一个重复的P1)
	std::vector<float> x01 = Interpolate(P0.y, P0.x, P1.y, P1.x);
	std::vector<float> x02 = Interpolate(P0.y, P0.x, P2.y, P2.x);
	std::vector<float> x12 = Interpolate(P1.y, P1.x, P2.y, P2.x);

	// 将x01和x12中的点放在一起
	x01.insert(x01.end(), x12.begin(), x12.end());
	std::vector<float> x012(x01);

	// 对于两种情况插值
	float mid = glm::floor(x012.size() / 2);
	std::vector<float> x_left;
	std::vector<float> x_right;

	//-------第一种情况
	//---------P2|\
	//-----------| \
	//-----------|  \ P1
	//-----------|  /
	//-----------| /
	//---------P0|/	
	if (x012[mid] > x02[mid]) {
		x_left = x02;
		x_right = x012;
	}
	//-------第二种情况
	//----------/|P2
	//---------/ | 
	//------p1/  | 
	//--------\	 |
	//---------\ |
	//----------\|P0	
	else {
		x_left = x012;
		x_right = x02;
	}

	// 从下到上，从左往右绘制
	for (int i = P0.y; i < P2.y; i++) {
		for (int j = x_left[i - P0.y]; j < x_right[i - P0.y]; j++) {
			putpixel(j, i, color);
		}
	}
}

void DrawShadedTriangle(Vertex v0, Vertex v1, Vertex v2) {
	if (v0.position.y > v1.position.y) std::swap(v0, v1);
	if (v0.position.y > v2.position.y) std::swap(v0, v2);
	if (v1.position.y > v2.position.y) std::swap(v1, v2);

	std::vector<float> x01 = Interpolate(v0.position.y, v0.position.x, v1.position.y, v1.position.x);
	std::vector<float> x12 = Interpolate(v1.position.y, v1.position.x, v2.position.y, v2.position.x);
	std::vector<float> x02 = Interpolate(v0.position.y, v0.position.x, v2.position.y, v2.position.x);

	std::vector<float> r01 = Interpolate(v0.position.y, v0.r, v1.position.y, v1.r);
	std::vector<float> r12 = Interpolate(v1.position.y, v1.r, v2.position.y, v2.r);
	std::vector<float> r02 = Interpolate(v0.position.y, v0.r, v2.position.y, v2.r); 

	std::vector<float> g01 = Interpolate(v0.position.y, v0.g, v1.position.y, v1.g);
	std::vector<float> g12 = Interpolate(v1.position.y, v1.g, v2.position.y, v2.g);
	std::vector<float> g02 = Interpolate(v0.position.y, v0.g, v2.position.y, v2.g);

	std::vector<float> b01 = Interpolate(v0.position.y, v0.b, v1.position.y, v1.b);
	std::vector<float> b12 = Interpolate(v1.position.y, v1.b, v2.position.y, v2.b);
	std::vector<float> b02 = Interpolate(v0.position.y, v0.b, v2.position.y, v2.b);

	x01.insert(x01.end(), x12.begin(), x12.end());
	r01.insert(r01.end(), r12.begin(), r12.end());
	g01.insert(g01.end(), g12.begin(), g12.end());
	b01.insert(b01.end(), b12.begin(), b12.end());

	std::vector<float> x012(x01), r012(r01), g012(g01), b012(b01);
	std::vector<float> x_left, x_right, r_left, r_right, g_left, g_right, b_left, b_right;
	float mid = glm::floor(x012.size() / 2);
	if (x02[mid] < x012[mid]) {
		x_left = x02; x_right = x012;
		r_left = r02; r_right = r012;
		g_left = g02; g_right = g012;
		b_left = b02; b_right = b012;
	}
	else {
		x_left = x012; x_right = x02;
		r_left = r012; r_right = r02;
		g_left = g012; g_right = g02;
		b_left = b012; b_right = b02;
	}

	for (int y = v0.position.y; y < v2.position.y; y++) {
		int i = y - v0.position.y;
		std::vector<float> rv = Interpolate(x_left[i], r_left[i], x_right[i], r_right[i]);
		std::vector<float> gv = Interpolate(x_left[i], g_left[i], x_right[i], g_right[i]);
		std::vector<float> bv = Interpolate(x_left[i], b_left[i], x_right[i], b_right[i]);
		
		for (int x = x_left[i]; x < x_right[i]; x++) {
			int j = x - x_left[i];
			COLORREF c = RGB(rv[j], gv[j], bv[j]);
			putpixel(x, y, c);
		}
	}
}

Vertex transform(Vertex v,glm::mat4 model,glm::mat4 view,glm::mat4 perspective,int ScreenWidth = screenWidth,int ScreenHeight = screenHeight) {
	// 缩放-旋转-平移
	glm::mat4 mvp = perspective * view * model;
	v.position = mvp * v.position;

	// 视口变换
	float reciprocalW = 1 / v.position.w;
	v.position.x = (v.position.x * reciprocalW + 1.0f) * 0.5f * ScreenWidth;
	v.position.y = (1.0f - v.position.y * reciprocalW) * 0.5f * ScreenHeight;  // window y轴坐标反着

	return v;
}

glm::mat4 GetModel(glm::vec3 s = { 1.0f, 1.0f, 1.0f }, glm::vec4 r = { 0,1,0,60.0f }, glm::vec3 t = { 0, 0, 5 })
{
	glm::mat4 sm = glm::scale(glm::mat4(1.0f), s);
	glm::mat4 rm = glm::rotate(glm::mat4(1.0f), glm::radians(r.w), glm::vec3(r));
	glm::mat4 tm = glm::translate(glm::mat4(1.0f), t);
	//构造模型矩阵
	return tm * rm * sm;
}

glm::mat4 GetView(glm::vec3 eye = glm::vec3(0, 0, 0), glm::vec3 center = glm::vec3(0, 0, 1), glm::vec3 up = glm::vec3(0, 1, 0))
{
	return glm::lookAt(eye, center, up);
}

glm::mat4 GetPerspective(float fov = glm::radians(90.0f), float aspect = screenWidth / screenHeight, float n = 0.1f, float f = 100.0f)
{
	return glm::perspective(fov, aspect, n, f);
}

void RenderTriangle(Triangle triangle, std::vector<Vertex> vertices) {
	DrawWireframeTriangle(vertices[triangle.index.x].position,
		vertices[triangle.index.y].position,
		vertices[triangle.index.z].position,
		triangle.color);
}

void RenderObject(std::vector<Vertex> vertices, std::vector<Triangle> triangle) {
	for (int i = 0; i < triangle.size(); i++) {
		RenderTriangle(triangle[i], vertices);
	}
}

void RenderInstance(Instance instances) {
	Model model = instances.model;
	glm::mat4 worldM = GetModel(instances.transform.scale3D, instances.transform.rotation, instances.transform.translate);

	for (int i = 0; i < model.vertices.size(); i++) {
		std::cout << "RenderInstance::transform: " << i << ",before position: " << model.vertices[i].position.x << "," << model.vertices[i].position.y << "," << model.vertices[i].position.z << std::endl;
		model.vertices[i].position = worldM * model.vertices[i].position;
		std::cout << "RenderInstance::transform: " << i << ",after position: "<<model.vertices[i].position.x<<","<< model.vertices[i].position.y<<","<< model.vertices[i].position.z<<std::endl;
	}
	for (int i = 0; i < model.triangles.size(); i++) {
		std::cout << "RenderInstance::RenderTriangle: " << i << std::endl;
		RenderTriangle(model.triangles[i], model.vertices);
	}
}

void RenderScene(Scene s) {
	for (int i = 0; i < s.instance.size(); i++) {
		std::cout << "RenderScene::RenderInstance: " << i << std::endl;
		RenderInstance(s.instance[i]);
	}
}

void testDrawLine();
void testDrawTriangle();
void testDrawShadedTriangle(int screenWidth,int screenHeight);
Vertex testTransform(Vertex v,int screenWidth,int screenHeight);
void testRenderObject();
void testRenderScene();
void testNoteBook();

int main() {
	
	initgraph(screenWidth, screenHeight);
	putpixel(100, 100, RED);

	// testDrawLine();
	// testDrawTriangle();
	// testDrawShadedTriangle(screenWidth,screenHeight);
	// testRenderObject();
	testRenderScene();

	_getch();
	closegraph();
	// testNoteBook();
	return 0;
}

void testDrawLine() {
	DrawLine(glm::vec2(100, 100), glm::vec2(100, 300), RED);
	DrawLine(glm::vec2(100, 300), glm::vec2(300, 300), GREEN);
	DrawLine(glm::vec2(300, 300), glm::vec2(300, 100), BLUE);
	DrawLine(glm::vec2(300, 100), glm::vec2(100, 100), WHITE);
}

void testDrawTriangle() {
	DrawWireframeTriangle(glm::vec2(100,400), glm::vec2(100,600), glm::vec2(300,600), YELLOW);
	DrawFilledTriangle(glm::vec2(400, 600), glm::vec2(600, 600), glm::vec2(600, 400), CYAN);
}

void testDrawShadedTriangle(int screenWidth,int screenHeight) {
	// 跟随OpenGL的规范，要求经过mvp转换后只能看到[0,1]之间的点，外面的点被剪裁
	Vertex v0({ -0.5f, -0.5f ,0 ,1},RED);
	Vertex v1({ 0.5f, -0.5f ,0 ,1},BLUE);
	Vertex v2({ 0.0f, 0.5f ,0 ,1},GREEN);

	v0 = testTransform(v0, screenWidth, screenHeight);
	v1 = testTransform(v1, screenWidth, screenHeight);
	v2 = testTransform(v2, screenWidth, screenHeight);

	DrawShadedTriangle(v0, v1, v2);
}

Vertex testTransform(Vertex v,int screenWidth,int screenHeight) {

	return transform(v,
		GetModel(),
		GetView(),
		GetPerspective(),
		screenWidth, screenHeight);
}

void testRenderObject() {
	std::vector<Vertex> Vertices;
	Vertices.resize(8);
	Vertices[0].position = { 1,1,1 ,1 };
	Vertices[1].position = { -1,1,1,1 };
	Vertices[2].position = { -1,-1,1 ,1 };
	Vertices[3].position = { 1,-1,1 ,1 };
	Vertices[4].position = { 1,1,-1 ,1 };
	Vertices[5].position = { -1,1,-1 ,1 };
	Vertices[6].position = { -1,-1,-1 ,1 };
	Vertices[7].position = { 1,-1,-1 ,1 };

	//Vertices[0].color = 1.0f;
	//Vertices[1].color = 1.0f;
	//Vertices[2].color = 0.5f;
	//Vertices[3].color = 0.5f;
	//Vertices[4].color = 0.5f;
	//Vertices[5].color = 0.5f;
	//Vertices[6].color = 1.0f;
	//Vertices[7].color = 1.0f;

	std::vector<Triangle> triangles;
	triangles.resize(12);
	triangles[0].index = { 0,1,2 };
	triangles[0].color = RED;
	triangles[1].index = { 0,2,3 };
	triangles[1].color = RED;
	triangles[2].index = { 4,0,3 };
	triangles[2].color = GREEN;
	triangles[3].index = { 4,3,7 };
	triangles[3].color = GREEN;
	triangles[4].index = { 5,4,7 };
	triangles[4].color = BLUE;
	triangles[5].index = { 5,7,6 };
	triangles[5].color = BLUE;
	triangles[6].index = { 1,5,6 };
	triangles[6].color = YELLOW;
	triangles[7].index = { 1,6,2 };
	triangles[7].color = YELLOW;
	triangles[8].index = { 4,5,1 };
	triangles[8].color = WHITE;
	triangles[9].index = { 4,1,0 };
	triangles[9].color = WHITE;
	triangles[10].index = { 2,6,7 };
	triangles[10].color = CYAN;
	triangles[11].index = { 2,7,3 };
	triangles[11].color = CYAN;

	// 转换
	{
		glm::mat4 model = GetModel();
		glm::mat4 view = GetView();
		glm::mat4 perspective = GetPerspective();

		for (int i = 0; i < Vertices.size(); i++) {
			Vertices[i] = transform(Vertices[i], model, view, perspective);
		}

		RenderObject(Vertices, triangles);
	}
}

void testRenderScene() {
	std::vector<Vertex> Vertices;
	Vertices.resize(8);
	Vertices[0].position = { 1,1,1 ,1 };
	Vertices[1].position = { -1,1,1,1 };
	Vertices[2].position = { -1,-1,1 ,1 };
	Vertices[3].position = { 1,-1,1 ,1 };
	Vertices[4].position = { 1,1,-1 ,1 };
	Vertices[5].position = { -1,1,-1 ,1 };
	Vertices[6].position = { -1,-1,-1 ,1 };
	Vertices[7].position = { 1,-1,-1 ,1 };

	//Vertices[0].color = 1.0f;
	//Vertices[1].color = 1.0f;
	//Vertices[2].color = 0.5f;
	//Vertices[3].color = 0.5f;
	//Vertices[4].color = 0.5f;
	//Vertices[5].color = 0.5f;
	//Vertices[6].color = 1.0f;
	//Vertices[7].color = 1.0f;

	std::vector<Triangle> triangles;
	triangles.resize(12);
	triangles[0].index = { 0,1,2 };
	triangles[0].color = RED;
	triangles[1].index = { 0,2,3 };
	triangles[1].color = RED;
	triangles[2].index = { 4,0,3 };
	triangles[2].color = GREEN;
	triangles[3].index = { 4,3,7 };
	triangles[3].color = GREEN;
	triangles[4].index = { 5,4,7 };
	triangles[4].color = BLUE;
	triangles[5].index = { 5,7,6 };
	triangles[5].color = BLUE;
	triangles[6].index = { 1,5,6 };
	triangles[6].color = YELLOW;
	triangles[7].index = { 1,6,2 };
	triangles[7].color = YELLOW;
	triangles[8].index = { 4,5,1 };
	triangles[8].color = WHITE;
	triangles[9].index = { 4,1,0 };
	triangles[9].color = WHITE;
	triangles[10].index = { 2,6,7 };
	triangles[10].color = CYAN;
	triangles[11].index = { 2,7,3 };
	triangles[11].color = CYAN;

	glm::mat4 model = GetModel();
	glm::mat4 view = GetView();
	glm::mat4 perspective = GetPerspective();

	for (int i = 0; i < Vertices.size(); i++) {
		Vertices[i] = transform(Vertices[i], model, view, perspective);
	}

	Scene s;
	Instance instance1;
	instance1.model.name = "1";
	instance1.model.vertices = Vertices;
	instance1.model.triangles = triangles;
	instance1.transform.translate = { 30,0,1 };
	instance1.transform.rotation = { 0,1,0,0.0f };
	instance1.transform.scale3D = { 1,1,1 };

	Instance instance2;
	instance2.model.name = "2";
	instance2.model.vertices = Vertices;
	instance2.model.triangles = triangles;
	instance2.transform.translate = { -30,0,1 };
	instance2.transform.rotation = { 0,1,0,-0.0f };
	instance2.transform.scale3D = { 1,1,1 };

	s.instance.push_back(instance1);
	s.instance.push_back(instance2);

	RenderScene(s);
}








void testNoteBook() {
	int ScreenWidth = 800;
	int ScreenHeight = 600;

	initgraph(ScreenWidth, ScreenHeight);	// 创建绘图窗口，大小为 640x480 像素

	Vertex v0({ -0.5f, -0.5f ,0.0f,1.0f },RED);
	Vertex v1({ 0.5f, -0.5f,0.0f,1.0f },GREEN);
	Vertex v2({ 0.0f, 0.5f ,0.0f,1.0f },BLUE);

	{
		glm::mat4 sm = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
		glm::mat4 rm = glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(0, 0, 1));
		glm::mat4 tm = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 1));
		//构造模型矩阵
		glm::mat4 model = tm * rm * sm;
		//构造视图矩阵
		glm::mat4 view = glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 0, 1), glm::vec3(0, 1, 0));
		//构造透视投影矩阵
		glm::mat4 perspective = glm::perspective(glm::radians(90.0f), (float)ScreenWidth / (float)ScreenHeight, 0.1f, 100.0f);
		//构造MVP矩阵
		glm::mat4 mvp = perspective * view * model;
		//对顶点进行MVP矩阵变换
		v0.position = mvp * v0.position;
		v1.position = mvp * v1.position;
		v2.position = mvp * v2.position;

		//透视除法
		float reciprocalW0 = 1 / v0.position.w;
		float reciprocalW1 = 1 / v1.position.w;
		float reciprocalW2 = 1 / v2.position.w;

		//屏幕映射
		v0.position.x = (v0.position.x * reciprocalW0 + 1.0f) * 0.5f * ScreenWidth;
		v0.position.y = (1.0f - v0.position.y * reciprocalW0) * 0.5f * ScreenHeight;
		v1.position.x = (v1.position.x * reciprocalW1 + 1.0f) * 0.5f * ScreenWidth;
		v1.position.y = (1.0f - v1.position.y * reciprocalW1) * 0.5f * ScreenHeight;
		v2.position.x = (v2.position.x * reciprocalW2 + 1.0f) * 0.5f * ScreenWidth;
		v2.position.y = (1.0f - v2.position.y * reciprocalW2) * 0.5f * ScreenHeight;
	}

	DrawShadedTriangle(v0, v1, v2);

	_getch();
	closegraph();			// 关闭绘图窗口
}