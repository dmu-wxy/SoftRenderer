#include "graphics.h"
#include "conio.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "iostream"
#include "vector"

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

void testDrawLine();
void testDrawTriangle();

int main() {
	initgraph(640, 640);
	putpixel(100, 100, RED);

	testDrawLine();
	testDrawTriangle();

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