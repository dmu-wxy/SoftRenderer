#include "graphics.h"
#include "conio.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "iostream"
#include "vector"

struct Vertex {
	glm::vec3 position;
	float color;
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

void DrawShadedTriangle(Vertex v0,Vertex v1,Vertex v2,COLORREF color) {
	if (v0.position.y > v1.position.y) std::swap(v0, v1);
	if (v0.position.y > v2.position.y) std::swap(v0, v2);
	if (v1.position.y > v2.position.y) std::swap(v1, v2);

	BYTE r = GetRValue(color);
	BYTE g = GetGValue(color);
	BYTE b = GetBValue(color);

	std::vector<float> x01 = Interpolate(v0.position.y, v0.position.x, v1.position.y, v1.position.x);
	std::vector<float> x12 = Interpolate(v1.position.y, v1.position.x, v2.position.y, v2.position.x);
	std::vector<float> x02 = Interpolate(v0.position.y, v0.position.x, v2.position.y, v2.position.x);

	std::vector<float> h01 = Interpolate(v0.position.y, v0.color, v1.position.y, v1.color);
	std::vector<float> h12 = Interpolate(v1.position.y, v1.color, v2.position.y, v2.color);
	std::vector<float> h02 = Interpolate(v0.position.y, v0.color, v2.position.y, v2.color);

	x01.insert(x01.end(), x12.begin(), x12.end());
	h01.insert(h01.end(), h12.begin(), h12.end());

	std::vector<float> x012(x01);
	std::vector<float> h012(h01);

	std::vector<float> x_left, x_right, h_left, h_right;

	float mid = glm::floor(x012.size() / 2);
	if (x02[mid] < x012[mid]) {
		x_left = x02;
		x_right = x012;
		h_left = h02;
		h_right = h012;
	}
	else {
		x_left = x012;
		x_right = x02;
		h_left = h012;
		h_right = h02;
	}

	for (int i = v0.position.y; i < v2.position.y; i++) {
		std::vector<float> h_segment = Interpolate(x_left[i - v0.position.y], h_left[i - v0.position.y], x_right[i - v0.position.y], h_right[i - v0.position.y]);
		for (int j = x_left[i - v0.position.y]; j < x_right[i - v0.position.y]; j++) {
			COLORREF shaded_color = RGB(r * h_segment[j - x_left[i - v0.position.y]],
				g * h_segment[j - x_left[i - v0.position.y]],
				b * h_segment[j - x_left[i - v0.position.y]]);
			putpixel(j, i, shaded_color);
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
	Vertex v0;
	v0.position = { 400, 100 ,0 };
	v0.color = 1.0f;
	Vertex v1;
	v1.position = { 600, 100 ,0 };
	v1.color = 0.5f;
	Vertex v2;
	v2.position = { 600, 300 ,0 };
	v2.color = 0.0f;

	DrawShadedTriangle(v0, v1, v2, RGB(255, 0, 0));
}