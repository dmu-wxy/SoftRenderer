#include "graphics.h"
#include "conio.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "iostream"
#include "vector"

struct Vertex {
	glm::vec3 position;
	COLORREF color;
	BYTE r, g, b;

	Vertex(glm::vec3 position, COLORREF color) {
		this->position = position;
		this->color = color;
		this->r = GetRValue(color);
		this->g = GetGValue(color);
		this->b = GetBValue(color);
	}
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
void testDrawLine();
void testDrawTriangle();
void testDrawShadedTriangle();

int main() {
	initgraph(640, 640);
	putpixel(100, 100, RED);

	testDrawLine();
	testDrawTriangle();
	testDrawShadedTriangle();

	_getch();
	closegraph();
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

void testDrawShadedTriangle() {
	Vertex v0({ 400, 100 ,0 },RED);
	Vertex v1({ 600, 100 ,0 },BLUE);
	Vertex v2({ 600, 300 ,0 },GREEN);

	DrawShadedTriangle(v0, v1, v2);
}