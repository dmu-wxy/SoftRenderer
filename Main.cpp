#include "graphics.h"
#include "conio.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "iostream"
#include "vector"

/**
* ��ֵ�㷨
* (i0,d0),(i1,d1)
* i0��i1ÿһ����׼������Ӧ��ֵ����Ӧ��d0��d1֮���ֵ[i0,i1)
* ����d0��d1֮��ֵ�ļ���
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
	// ��ֵ��ÿ�μ���tx,ty����
	float dy = P0.y - P1.y;
	float dx = P0.x - P1.x;

	// �����������Ҫע�⣺1.��ֱ���ߣ�2.��������/�������ϵ���
	if (dx == 0) {
		// ��ֱ
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
	// ���򶥵㣺P0.y <= P1.y <= P2.y
	if (P0.y > P1.y) std::swap(P0, P1);
	if (P0.y > P2.y) std::swap(P0, P2);
	if (P1.y > P2.y) std::swap(P1, P2);

	//------------------P2|\
	//--------------------| \
	//--------------------|  \ P1
	//--------------------|  /
	//--------------------| /
	//------------------P0|/	
	// ��ÿ���ߵ�x���ֵ����Ϊ���(P0)�յ�(P1)��ͬ����������Ĳ�ֵ(x)����x02��x01/x12����֮����ͬ(x01/x12��һ���ظ���P1)
	std::vector<float> x01 = Interpolate(P0.y, P0.x, P1.y, P1.x);
	std::vector<float> x02 = Interpolate(P0.y, P0.x, P2.y, P2.x);
	std::vector<float> x12 = Interpolate(P1.y, P1.x, P2.y, P2.x);

	// ��x01��x12�еĵ����һ��
	x01.insert(x01.end(), x12.begin(), x12.end());
	std::vector<float> x012(x01);

	// �������������ֵ
	float mid = glm::floor(x012.size() / 2);
	std::vector<float> x_left;
	std::vector<float> x_right;

	//-------��һ�����
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
	//-------�ڶ������
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

	// ���µ��ϣ��������һ���
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