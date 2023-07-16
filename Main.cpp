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
* i0��i1ÿһ����׼������Ӧ��ֵ����Ӧ��d0��d1֮���ֵ
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
	DrawWireframeTriangle(glm::vec2(100,400),glm::vec2(100,600),glm::vec2(300,600), YELLOW);
}