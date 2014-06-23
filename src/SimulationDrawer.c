#include <allegro.h>
#include "SimulationDrawer.h"
#include "InputThread.h"
#include "Utils.h"
#include "Simulation.h"

#define PERCENT_OF(val, per) (SCREEN_W / (100 / per))
#define WHITE 0
#define BLACK 1
#define BLUE 2
#define RED 3
#define GREEN 4
#define YELLOW 5

#define BOX_BORDER_WIDTH 2

static BITMAP *dblbuffer;

int initSimulationDrawer(int p_width, int p_height)
{
	int ret;
	RGB colour;
	
	if (set_gfx_mode(GFX_AUTODETECT_WINDOWED, p_width, p_height, 0, 0) < 0) {
		PRINT_ERR("Couldn't set gfx mode: %s\n", allegro_error);
		return -1;
	}
	
	// create colours
	colour.r = 255;
	colour.g = 255;
	colour.b = 255;
	set_color(WHITE, &colour);
	
	colour.r = 0;
	colour.g = 0;
	colour.b = 0;
	set_color(BLACK, &colour);
	
	colour.r = 0;
	colour.g = 0;
	colour.b = 255;
	set_color(BLUE, &colour);
	
	colour.r = 255;
	colour.g = 0;
	colour.b = 0;
	set_color(RED, &colour);
	
	colour.r = 0;
	colour.g = 255;
	colour.b = 0;
	set_color(GREEN, &colour);
	
	colour.r = 255;
	colour.g = 255;
	colour.b = 0;
	set_color(YELLOW, &colour);
	
	// create double buffering bit map to draw on
	dblbuffer = create_bitmap(SCREEN_W, SCREEN_H);
	clear(dblbuffer);
	
	// show double buffer content pn screen
    blit(dblbuffer, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
	
	return 0;
}

int destroySimulationDrawer()
{
	destroy_bitmap(dblbuffer);
	
	return 0;
}

int drawCar(unsigned int p_carPosition, unsigned int p_hlineOffset)
{
	unsigned int carWidth = PERCENT_OF(SCREEN_H, 3);
	unsigned int carLength = (unsigned int) (carWidth * 1.5);
	unsigned int wheelLength = carLength / 6;
	unsigned int wheelWidth = (wheelLength * 2) / 3;
	unsigned int carFront = p_hlineOffset + p_carPosition;
	unsigned int carBack = (p_carPosition - carLength) + p_hlineOffset;
	
	//draw Car
	rectfill(dblbuffer, carBack, (SCREEN_H / 2) - (carWidth / 2), carFront, (SCREEN_H / 2) + (carWidth / 2), BLUE);
	//top left wheel
	rectfill(dblbuffer, carBack + wheelLength, (SCREEN_H / 2) - (carWidth / 2) - wheelWidth - 1, carBack + 2 * wheelLength, (SCREEN_H / 2) - (carWidth / 2) - 1, BLACK);
	//bot left wheel
	rectfill(dblbuffer, carBack + wheelLength, (SCREEN_H / 2) + (carWidth / 2) + 1, carBack + 2 * wheelLength, (SCREEN_H / 2) + (carWidth / 2) + wheelWidth + 1, BLACK);
	//top right wheel
	rectfill(dblbuffer, carFront - 2 * wheelLength , (SCREEN_H / 2) - (carWidth / 2) - wheelWidth - 1, carFront - wheelLength, (SCREEN_H / 2) - (carWidth / 2) - 1, BLACK);
	//bot right wheel
	rectfill(dblbuffer, carFront - 2 * wheelLength, (SCREEN_H / 2) + (carWidth / 2) + 1, carFront - wheelLength, (SCREEN_H / 2) + (carWidth / 2) + wheelWidth + 1, BLACK);
	
	return 0;
}

void drawSpeechState(int p_listenState)
{
	char *text;
	int color;
	if(p_listenState == INPUT_LISTENING) {
		color = GREEN;
		text = "Listening";
	} else if(p_listenState == INPUT_WAITING) {
		color = RED;
		text = "Waiting";
	} else if(p_listenState == INPUT_PROCESSING) {
		color = YELLOW;
		text = "Processing";
	} else {
		return;
	}
	
	unsigned int boxLeftOffset = PERCENT_OF(SCREEN_W, 1);
	unsigned int boxTopOffset = PERCENT_OF(SCREEN_H, 1);
	unsigned int boxWidth = PERCENT_OF(SCREEN_W, 12);
	unsigned int boxHeight = PERCENT_OF(SCREEN_W, 6);
	
	rectfill(dblbuffer, boxLeftOffset - BOX_BORDER_WIDTH, boxTopOffset - BOX_BORDER_WIDTH, boxLeftOffset + boxWidth + BOX_BORDER_WIDTH, boxTopOffset + boxHeight + BOX_BORDER_WIDTH, BLACK);
	rectfill(dblbuffer, boxLeftOffset, boxTopOffset, boxLeftOffset + boxWidth, boxTopOffset + boxHeight, color);
	textout_centre_ex(dblbuffer, font, text, boxLeftOffset + (boxWidth / 2), boxTopOffset + (boxHeight / 2) - 2, BLACK, -1);
}

int drawSimulation(int p_position, int p_distance, int *p_topObstacles, int *p_botObstacles, int p_state, int p_listenState)
{
	unsigned int hlineOffset = PERCENT_OF(SCREEN_W, 5);
	unsigned int hlineLength = SCREEN_W - hlineOffset * 2;
	unsigned int hlineYbot = SCREEN_H - PERCENT_OF(SCREEN_H, 10);
	unsigned int hlineYtop = hlineYbot - PERCENT_OF(SCREEN_H, 30);
	unsigned int obstacleSize = PERCENT_OF(SCREEN_H, 2);
	unsigned int playerX = hlineOffset;
	unsigned int playerY;
	unsigned int segmentLength = hlineLength / p_distance;
	
	clear(dblbuffer);
	
	hline(dblbuffer, hlineOffset, hlineYtop, hlineOffset + hlineLength, BLACK);
	hline(dblbuffer, hlineOffset, hlineYbot, hlineOffset + hlineLength, BLACK);
	
	if(p_position == TOP_POSITION)
		playerY = hlineYtop;
	else if (p_position == BOT_POSITION)
		playerY = hlineYbot;
	else
		return;
	
	// draw player
	rectfill(dblbuffer, playerX - obstacleSize, playerY - obstacleSize, playerX + obstacleSize, playerY + obstacleSize, BLUE);
	
	int i;
	unsigned int obstacelX, obstacleY;
	
	// draw obstacles
	for(i = 0; i < OBSTACLE_COUNT; ++i) {
		if(p_topObstacles[i] >= 0 && p_topObstacles[i] <= p_distance) {
			obstacelX = hlineOffset + hlineLength - p_topObstacles[i] * segmentLength;
			obstacleY = hlineYtop;
			rectfill(dblbuffer, obstacelX - obstacleSize, obstacleY - obstacleSize, obstacelX + obstacleSize, obstacleY + obstacleSize, RED);
		}
		
		if(p_botObstacles[i] >= 0 && p_botObstacles[i] <= p_distance) {
			obstacelX = hlineOffset + hlineLength - p_botObstacles[i] * segmentLength;
			obstacleY = hlineYbot;
			rectfill(dblbuffer, obstacelX - obstacleSize, obstacleY - obstacleSize, obstacelX + obstacleSize, obstacleY + obstacleSize, RED);
		}
	}
	
	// -1 for background to be invisible
	if(p_state != 0)
		textout_centre_ex(dblbuffer, font, "Crashed!", SCREEN_W / 2, 0, RED, -1);
	
	drawSpeechState(p_listenState);
	
    blit(dblbuffer, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
	
	return 0;
}