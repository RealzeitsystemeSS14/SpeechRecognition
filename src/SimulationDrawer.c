#include <allegro.h>
#include "SimulationDrawer.h"
#include "Utils.h"

#define PERCENT_OF(val, per) (SCREEN_W / (100 / per))
#define WHITE 0
#define BLACK 1
#define BLUE 2
#define RED 3
#define GREEN 4

static BITMAP *dblbuffer;

int initSimulationDrawer(unsigned int p_width, unsigned int p_height)
{
	int ret;
	RGB colour;
	allegro_init();
	
	ret = set_gfx_mode(GFX_AUTODETECT_WINDOWED, p_width, p_height, 0, 0);
	if (ret < 0) {
		PRINT_ERR("Couldn't set gfx mode: %s\n", allegro_error);
		return ret;
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
	
	// create double buffering bit map to draw on
	dblbuffer = create_bitmap(SCREEN_W, SCREEN_H);
	clear(dblbuffer);
	
	// show double buffer content pn screen
	vsync();
    blit(dblbuffer, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
	
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

int drawSimulation(unsigned int p_carPosition, unsigned int p_distance, int p_status)
{
	unsigned int hlineOffset = PERCENT_OF(SCREEN_W, 5);
	unsigned int hlineLength = SCREEN_W - hlineOffset * 2;
	unsigned int vlineOffset = PERCENT_OF(SCREEN_H, 4);
	unsigned int vlineLength = SCREEN_H - vlineOffset * 2;
	
	unsigned int carX = (((1000 * p_carPosition) / p_distance) * hlineLength) / 1000;
	
	clear(dblbuffer);
	
	hline(dblbuffer, hlineOffset, SCREEN_H / 2, hlineOffset + hlineLength, BLACK);
	vline(dblbuffer, hlineOffset + hlineLength, vlineOffset, vlineOffset + vlineLength, BLACK);
	drawCar(carX, hlineOffset);
	
	// -1 for background to be invisible
	if(p_status == -1)
		textout_centre_ex(dblbuffer, font, "Car crashed!", SCREEN_W / 2, 0, RED, -1);
	else if(p_status == 1)
		textout_centre_ex(dblbuffer, font, "Car stopped!", SCREEN_W / 2, 0, GREEN, -1);
	
	vsync();
    blit(dblbuffer, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
	
	return 0;
}

int destroySimulationDrawer()
{
	destroy_bitmap(dblbuffer);
	
	allegro_exit();
	
	return 0;
}