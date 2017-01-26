// Name: Arber Muharemi
#include "myLib.h"

void drawImageFullScreen(int x, int y, int width, int height, const unsigned short *image) {
	REG_DISPCTL = MODE3 | BG2_ENABLE;
	DMA[3].src = (u32)image;
	DMA[3].dst = (u32)&videoBuffer[OFFSET(y, x, 240)];
	DMA[3].cnt = (width * height) | DMA_ON | DMA_SOURCE_INCREMENT;
}

void drawImageSprite(int x, int y, int width, int height, const unsigned short *image) {
	for (int i = 0; i < height; ++i) {
		DMA[3].src = (u32)(image + i * width);
		DMA[3].dst = (u32)&videoBuffer[OFFSET(y + i, x, 240)];
		DMA[3].cnt = (width) | DMA_ON | DMA_SOURCE_INCREMENT;
	}
}