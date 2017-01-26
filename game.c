#include <stdlib.h>
#include <stdio.h>
#include "myLib.h"
#include "text.h"
#include "titlescreen.h"
#include "ball.h"
#include "paddle.h"
#include "winScreen.h"
#include "loseScreen.h"

// Start message
char startGameMessage[] = "Press START to being!";
char promptStartPlayingMessage[] = "Press A to launch the ball!";
char gameOverMessage[] = "Press A or SELECT to play again!";

// State enum definition
enum GBAState {
	START,
	GAMEPLAY,
	PLAYING,
	GAME_OVER_LOSE,
	GAME_OVER_WIN
};

const int GBA_HEIGHT = 160;
const int GBA_WIDTH = 240;
const int START_BRICK_NUM = 32;
const u16 colors[] = {RED, GREEN, BLUE, MAGENTA, CYAN, YELLOW, GREY};
int selectWasPressed = 0;
int numBricksLeft = 0;
char scoreString[] = "Score:";
int score = 0;
char buffer[50];
// extern int sprintf ( char * str, const char * format, ... );

MOVOBJ ballObj;
MOVOBJ paddleObj;
BRICK brickArr[32];
enum GBAState state = START;

int main() {
	REG_DISPCTL = MODE3 | BG2_ENABLE;

	while(1) {
		switch (state) {
			case START:
				waitForVblank();
				drawStartScreen();
				state = GAMEPLAY;
				break;
			case GAMEPLAY:
				if (KEY_DOWN_NOW(BUTTON_START)) {
					drawGameStage();
					state = PLAYING;
				}
				break;
			case PLAYING:
				if (!goBackToStartScreen() && KEY_DOWN_NOW(BUTTON_A)) {
					startPlaying();
					// drawString(120, 70, gameOverMessage, WHITE);
					// state = GAME_OVER;
				}
				break;
			case GAME_OVER_LOSE:
				if (goBackToStartScreen() || KEY_DOWN_NOW(BUTTON_A)) {
					state = START;
				}
				break;
			case GAME_OVER_WIN:
				if (goBackToStartScreen() || KEY_DOWN_NOW(BUTTON_A)) {
					state = START;
				}
				break;
		}
	}

	// for (int i = 0; i < GIPHY_FRAMES; ++i)
	// {
	// 	REG_DISPCTL = MODE3 | BG2_ENABLE;
	// 	REG_DMA3SAD = (u32)giphy_frames[i];
	// 	REG_DMA3DAD = (u32)videoBuffer;
	// 	REG_DMA3CNT = 38400 | DMA_ON | DMA_SOURCE_INCREMENT;
	// 	if (i == GIPHY_FRAMES - 1)
	// 	{
	// 		i = 0;
	// 	}
	// }
}

	int goBackToStartScreen() {
		if (KEY_DOWN_NOW(BUTTON_SELECT)) {
			selectWasPressed = 1;
			state = START;
			return 1;
		} else {
			selectWasPressed = 0;
			return 0;
		}
	}

	void drawStartScreen() {
		drawImageFullScreen(0, 0, BRICKBREAKER_WIDTH, BRICKBREAKER_HEIGHT, brickbreaker);
		drawString(120, GBA_WIDTH/4, startGameMessage, WHITE);
	}

	void drawGameStage() {
		drawRect(0, 0, GBA_HEIGHT, GBA_WIDTH, BLACK); // draw a black background
		initializeAndDrawObjects();
		drawString(100, GBA_WIDTH/6, promptStartPlayingMessage, WHITE);
	}

	void startPlaying() {
		drawString(100, GBA_WIDTH/6, promptStartPlayingMessage, BLACK);
		drawString(153, 0, scoreString, WHITE);
		while(!gameOver() && !goBackToStartScreen()) {
			ballObj.prevRow = ballObj.row;
			ballObj.prevCol = ballObj.col;
			ballObj.col += ballObj.cd;
			ballObj.row += ballObj.rd;
			drawString(153, 0, scoreString, WHITE);					
			sprintf(buffer, "%s%d", buffer, score);
			drawString(153, 36, buffer, WHITE);
			buffer[0] = '\0';
			if (ballHitsLeftSide() || ballHitsRightSide() || ballHitsBrickSide()) {
				ballObj.cd *= -1;
			} else if (ballHitsTop() || ballHitsPad() || ballHitsBrickTopBot()) {
				ballObj.rd *= -1;
			}
			waitForVblank();
			drawRect(ballObj.prevRow, ballObj.prevCol, ballObj.height, ballObj.width, BLACK);
			drawImageSprite(ballObj.col, ballObj.row, ballObj.width, ballObj.height, ball);
			paddleObj.prevRow = paddleObj.row;
			paddleObj.prevCol = paddleObj.col;
			if (KEY_DOWN_NOW(BUTTON_LEFT)) {
				padMovesLeft();
				drawRect(paddleObj.prevRow, paddleObj.prevCol, paddleObj.height, paddleObj.width, BLACK);
				drawImageSprite(paddleObj.col, paddleObj.row, paddleObj.width, paddleObj.height, paddle);
			} else if (KEY_DOWN_NOW(BUTTON_RIGHT)) {
				padMovesRight();
				drawRect(paddleObj.prevRow, paddleObj.prevCol, paddleObj.height, paddleObj.width, BLACK);
				drawImageSprite(paddleObj.col, paddleObj.row, paddleObj.width, paddleObj.height, paddle);					
			}
		}
	}

	void initializeAndDrawObjects() {
		paddleObj.row = GBA_WIDTH/2 +20;
		paddleObj.col = GBA_HEIGHT/2 + 20;
		paddleObj.rd = 0;			
		paddleObj.cd = 5;			
		paddleObj.width = PADDLE_WIDTH;
		paddleObj.height = PADDLE_HEIGHT;
		drawImageSprite(paddleObj.col, paddleObj.row, paddleObj.width, paddleObj.height, paddle);

		ballObj.row = paddleObj.row - BALL_HEIGHT;
		ballObj.col = paddleObj.col + BALL_WIDTH;
		ballObj.rd = -2;
		ballObj.cd = 2;
		ballObj.width = BALL_WIDTH;
		ballObj.height = BALL_HEIGHT;
		drawImageSprite(ballObj.col, ballObj.row, ballObj.width, ballObj.height, ball);

		for (int i = 0; i < START_BRICK_NUM; i++) {
			brickArr[i].width = 30;
			brickArr[i].height = 15;
			brickArr[i].row = i / 8 * brickArr[i].height;  
			brickArr[i].col = i % 8 * brickArr[i].width;
			brickArr[i].onScreen = 1;
			brickArr[i].color = colors[i * 2 % sizeof(colors)/sizeof(colors[0])];
			drawRect(brickArr[i].row, brickArr[i].col, brickArr[i].height, brickArr[i].width, brickArr[i].color);
			numBricksLeft++;
		}


	}


	int ballHitsLeftSide() {
 		if (ballObj.col <= 0) {
 			ballObj.col = 0;
 			return 1;
 		}
 		return 0;
	}

	int ballHitsRightSide() {
 		if (ballObj.col + ballObj.width >= 239) {
 			ballObj.col = 239 - ballObj.width;
 			return 1;
 		}
 		return 0;
	}

	int ballHitsTop() {
		if (ballObj.row <= 0) {
			ballObj.row = 0;
			return 1;
		}
		return 0;
	}

	int ballHitsPad() {
		if (ballObj.row + ballObj.height == paddleObj.row) { // hits from top
			if (ballObj.col + ballObj.width >= paddleObj.col
					&& ballObj.col <= paddleObj.col + paddleObj.width) {
				return 1;
			}
		}
		if (ballObj.row + ballObj.height <= paddleObj.row + paddleObj.height / 2
			&& ballObj.row + ballObj.height >= paddleObj.row) { // perfect side hitting
			if (ballObj.col + ballObj.width == paddleObj.col
				|| ballObj.col == paddleObj.col + paddleObj.width) {
				return 1;
			}
		}
		if (ballObj.row + ballObj.height > paddleObj.row
		&& ballObj.col + ballObj.width >= paddleObj.col
		&& ballObj.col <= paddleObj.col + paddleObj.width) { // edge cases (ball enters paddle)
			ballObj.row = paddleObj.row - ballObj.height;
			ballObj.cd *= -1;
			return 1;
		}
		return 0;
	}

	int ballHitsBrickSide() {
		for (int i = 0; i < START_BRICK_NUM; i++) {
			if (brickArr[i].onScreen) {
				if ((ballObj.row + ballObj.height / 2 >= brickArr[i].row 							// ball within brick top
					&& ballObj.row + ballObj.height / 2 <= brickArr[i].row + brickArr[i].height)	// ball within brick bot
					&& ballObj.col + ballObj.width >= brickArr[i].col 								// ball hits left
					&& ballObj.col <= brickArr[i].col + brickArr[i].width) {						// ball hits right
					brickArr[i].onScreen = 0;
					brickArr[i].color = BLACK;
					numBricksLeft--;
					score++;
					drawRect(brickArr[i].row, brickArr[i].col, brickArr[i].height, brickArr[i].width, brickArr[i].color);
					drawRect(153, 36, 8, 12, BLACK);
					sprintf(buffer, "%s%d", buffer, score);
					drawString(153, 36, buffer, WHITE);
					buffer[0] = '\0';
					return 1;
				}
			}
		}
		return 0;
	}

	int ballHitsBrickTopBot() {
		for (int i = 0; i < START_BRICK_NUM; i++) {
			if (brickArr[i].onScreen) {
				if ((ballObj.row + ballObj.height == brickArr[i].row 	// ball hits top brick 
					|| ballObj.row == brickArr[i].row + brickArr[i].height)	// ball hits bot brick
					&& ballObj.col + ballObj.width >= brickArr[i].col 		// ball within brick start and end
					&& ballObj.col <= brickArr[i].col + brickArr[i].width) {
					brickArr[i].onScreen = 0;
					brickArr[i].color = BLACK;
					numBricksLeft--;
					score++;
					drawRect(brickArr[i].row, brickArr[i].col, brickArr[i].height, brickArr[i].width, brickArr[i].color);
					drawRect(153, 36, 8, 12, BLACK);
					sprintf(buffer, "%s%d", buffer, score);
					drawString(153, 36, buffer, WHITE);
					buffer[0] = '\0';
					return 1;
				}
			}
		}
		return 0;
	}

	void padMovesLeft() {
		if (paddleObj.col - paddleObj.cd >= 0) {
			paddleObj.col -= paddleObj.cd;
		} else {
			paddleObj.col = 0;
		}
	}

	void padMovesRight() {
		if (paddleObj.col + paddleObj.cd + paddleObj.width < 239) {
			paddleObj.col += paddleObj.cd;
		} else {
			paddleObj.col = 239 - paddleObj.width;
		}
	}

	int stageClear() {
		return numBricksLeft == 0;
	}

	int ballBelowPad() {
		return ballObj.row > paddleObj.row + paddleObj.height / 2;
	}

	int gameOver() {
		if (stageClear()) {
			state = GAME_OVER_WIN;
			drawWinScreen();
			return 1;
		} else if (ballBelowPad()) {
			state = GAME_OVER_LOSE;
			drawLoseScreen();
			return 1;
		} else {
			return 0;
		}
	}

	void drawWinScreen() {
		drawImageFullScreen(0, 0, WINSCREEN_WIDTH, WINSCREEN_HEIGHT, winScreen);
		drawString(20, GBA_WIDTH/8, gameOverMessage, BLACK);
	}

	void drawLoseScreen() {
		drawImageFullScreen(0, 0, LOSESCREEN_WIDTH, LOSESCREEN_HEIGHT, loseScreen);
		drawString(140, GBA_WIDTH/8, gameOverMessage, WHITE);
	}