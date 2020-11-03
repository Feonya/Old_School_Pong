#include <iostream>
#include <SDL2/SDL.h>

#define FPS 20.0f  // We use low FPS to emulate old PC performance
float g_targetFrameTicks = 1000.0f / FPS;  // How many ticks between every frame we expect
float g_lastFrameTicks   = 0.0f;	   // Store the previous frame ticks number

bool g_isGameRunning = true;  // Represent the game is running or not

/*
 * SDL Window parameters
 */
#define WINDOW_TITLE  "Old School Pong"
#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 600
#define WINDOW_FLAGS  0

SDL_Window* g_window = NULL;

/*
 * SDL Renderer parameters
 */
#define DRIVER_INDEX    -1  // -1 means to initialize the first one supporting the request renderer flags
#define RENDERER_FLAGS  0   // 0 means to try using hardware acceleration first

SDL_Renderer* g_renderer = NULL;

/*
 * GameObject data structure
 */
struct GameObject {
    SDL_Rect rect;
    int velX, velY;
};

GameObject g_ball, g_leftPaddle, g_rightPaddle;

/*
 * Initialize the SDL library
 */
int initializeSDL() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
	std::cout << "Initialize SDL library failed!\n";
	return 1;
    }

    return 0;
}

/*
 * Create a SDL Window with specified parameters
 */
int createSDLWindow() {
    g_window = SDL_CreateWindow(
	WINDOW_TITLE,
	SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
	WINDOW_WIDTH, WINDOW_HEIGHT,
	WINDOW_FLAGS
    );

    if (g_window == NULL) {
	std::cout << "Create SDL Window failed!\n";
	return 1;
    }

    return 0;
}


/*
 * Create a SDL 2D rendering context for a SDL Window
 */
int createSDLRenderer() {
    g_renderer = SDL_CreateRenderer(g_window, DRIVER_INDEX, RENDERER_FLAGS);

    if (g_renderer == NULL) {
	std::cout << "Create SDL Renderer failed!\n";
	return 1;
    }

    return 0;
}

/*
 * Destroy SDL Renderer and Window, then quit
 */
void QuitSDL() {
    SDL_DestroyRenderer(g_renderer);
    SDL_DestroyWindow(g_window);
    SDL_Quit();
}

/*
 * Handle user input on game loop
 */
void input() {
    SDL_Event event;
    // Remove the next event from the event-queue and store it in the event varialbe
    while (SDL_PollEvent(&event)) {
	switch (event.type) {
	    // When we click the close button on the right top window
	    case SDL_QUIT:
		g_isGameRunning = false;
		break;

	    // When we press a key down on the keyboard
	    case SDL_KEYDOWN:
		if (event.key.keysym.sym == SDLK_ESCAPE) g_isGameRunning = false;

		if (event.key.keysym.sym == SDLK_w)    g_leftPaddle.velY  = -200;
		if (event.key.keysym.sym == SDLK_s)    g_leftPaddle.velY  =  200;
		if (event.key.keysym.sym == SDLK_UP)   g_rightPaddle.velY = -200;
		if (event.key.keysym.sym == SDLK_DOWN) g_rightPaddle.velY =  200;

		break;

	    // When we release a key up on the keyboard
	    case SDL_KEYUP:
		if (event.key.keysym.sym == SDLK_w)    g_leftPaddle.velY  = 0;
		if (event.key.keysym.sym == SDLK_s)    g_leftPaddle.velY  = 0;
		if (event.key.keysym.sym == SDLK_UP)   g_rightPaddle.velY = 0;
		if (event.key.keysym.sym == SDLK_DOWN) g_rightPaddle.velY = 0;

		break;

	    default:
		break;
	}
    }
}

/*
 * Process game logic and update game data
 */
void update() {
    /* Fixing time step */
    float actualFrameTicks = SDL_GetTicks() - g_lastFrameTicks;
    if (actualFrameTicks < g_targetFrameTicks)
	// Use SDL_Delay instead of return, so we'll get a good performance
	SDL_Delay(g_targetFrameTicks - actualFrameTicks);

    float deltaTime = (SDL_GetTicks() - g_lastFrameTicks) * 0.001f;

    g_lastFrameTicks = SDL_GetTicks();

    /* Game logic */
    /* Update ball position */
    g_ball.rect.x += g_ball.velX * deltaTime;
    g_ball.rect.y += g_ball.velY * deltaTime;

    /* Update paddles position*/
    g_leftPaddle.rect.y  += g_leftPaddle.velY  * deltaTime;
    g_rightPaddle.rect.y += g_rightPaddle.velY * deltaTime;

    /* Check for ball collision with the wall */
    if (g_ball.rect.y <= 0 || g_ball.rect.y >= WINDOW_HEIGHT - g_ball.rect.h)
	g_ball.velY = -g_ball.velY;

    /* Check for ball collision with paddles */
    if (g_ball.rect.x <= g_leftPaddle.rect.x + g_leftPaddle.rect.w &&
	g_ball.rect.x >  g_leftPaddle.rect.x &&
	g_ball.rect.y >= g_leftPaddle.rect.y - g_ball.rect.h &&
	g_ball.rect.y <= g_leftPaddle.rect.y + g_leftPaddle.rect.h + g_ball.rect.h) {
	g_ball.velX = -g_ball.velX;
    }

    if (g_ball.rect.x >= g_rightPaddle.rect.x - g_ball.rect.w &&
	g_ball.rect.x <  g_rightPaddle.rect.x + g_rightPaddle.rect.w &&
	g_ball.rect.y >= g_rightPaddle.rect.y - g_ball.rect.h &&
	g_ball.rect.y <= g_rightPaddle.rect.y + g_rightPaddle.rect.h + g_ball.rect.h) {
	g_ball.velX = -g_ball.velX;
    }

    /* Prevent paddles from moving outside the boundaries of the window */
    g_leftPaddle.rect.y = g_leftPaddle.rect.y < 0 ? 0 : g_leftPaddle.rect.y;
    int leftMaxHeight = WINDOW_HEIGHT - g_leftPaddle.rect.h;
    g_leftPaddle.rect.y = g_leftPaddle.rect.y > leftMaxHeight ? leftMaxHeight : g_leftPaddle.rect.y;

    g_rightPaddle.rect.y = g_rightPaddle.rect.y < 0 ? 0 : g_rightPaddle.rect.y;
    int rightMaxHeight = WINDOW_HEIGHT - g_rightPaddle.rect.h;
    g_rightPaddle.rect.y = g_rightPaddle.rect.y > rightMaxHeight ? rightMaxHeight : g_rightPaddle.rect.y;

    /* Check for game over */
    if (g_ball.rect.x <= 0 || g_ball.rect.x >= WINDOW_WIDTH - g_ball.rect.w) {
	g_isGameRunning = false;
	std::cout << "\n";
	std::cout << "**************************\n";
	std::cout << "        Game Over!        \n";
	std::cout << "**************************\n";
	std::cout << "\n";
    }
}

/*
 * Draw GameObjects on the screen
 */
void render() {
    SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255);
    SDL_RenderClear(g_renderer);  // Clear background to black color

    /* Draw GameObjects */
    SDL_SetRenderDrawColor(g_renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(g_renderer, &g_ball.rect);
    SDL_RenderFillRect(g_renderer, &g_leftPaddle.rect);
    SDL_RenderFillRect(g_renderer, &g_rightPaddle.rect);

    SDL_RenderPresent(g_renderer);
}

/*
 * Create and initialize GameObjects
 */
GameObject createGameObject(int f_x, int f_y, int f_w, int f_h, int f_vx, int f_vy) {
    GameObject go = { { f_x, f_y, f_w, f_h }, f_vx, f_vy };
    return go;
}

/*
 * Program entry point
 */
int main() {
    if (initializeSDL()     == 1) return 1;
    if (createSDLWindow()   == 1) return 1;
    if (createSDLRenderer() == 1) return 1;

    g_ball        = createGameObject(WINDOW_WIDTH / 2 - 10, WINDOW_HEIGHT / 2 - 10, 20, 20, 150, 150);
    g_leftPaddle  = createGameObject(0, 		    WINDOW_HEIGHT / 2 - 35, 20, 70, 0,   0);
    g_rightPaddle = createGameObject(WINDOW_WIDTH - 20,     WINDOW_HEIGHT / 2 - 35, 20, 70, 0,   0);

    // The game loop
    while (g_isGameRunning) {
	input();
	update();
	render();
    }

    QuitSDL();
    return 0;
}
