#include "graphics.h"
#include "conio.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "iostream"

void DrawLine(glm::vec2 P0, glm::vec2 P1, COLORREF color) {
	// 插值比每次计算tx,ty更快
	float dy = P0.y - P1.y;
	float dx = P0.x - P1.x;

	// 有两种情况需要注意：1.垂直的线；2.从右往左/从下往上的线
	if (dx == 0) {
		// 竖直
		if (P0.y > P1.y) std::swap(P0, P1);
		float tx = P0.x;
		for (float ty = P0.y; ty <= P1.y; ty++) {
			putpixel(tx, ty, color);
		}
	}
	else {
		if (P0.x > P1.x) std::swap(P0, P1);
		float ty = P0.y;
		float a = dy / dx;
		for (float tx = P0.x; tx <= P1.x; tx++) {
			ty = ty + a;
			putpixel(tx, ty, color);
		}
	}
}
int main() {
	initgraph(640, 640);
	putpixel(100, 100, RED);

	DrawLine(glm::vec2(100, 100), glm::vec2(100, 300), RED);
	DrawLine(glm::vec2(100, 300), glm::vec2(300, 300), GREEN);
	DrawLine(glm::vec2(300, 300), glm::vec2(300, 100), BLUE);
	DrawLine(glm::vec2(300, 100), glm::vec2(100, 100), WHITE);

	_getch();
	closegraph();
	return 0;
}