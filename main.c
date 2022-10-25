/* Pong game made with C and SDL by Henry Fielding */

/*----------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_image.h>

/*----------------------------------------------------------------------------*/
#define WINDOW_WIDTH (640)      // width of drawn window
#define WINDOW_HEIGHT (480)     // height of drawn window

#define SPEED (240)             // initial speed of ball
#define BALLSPEED (100)

#define COLLISION (1)           // collision found
#define NO_COLLISION (-1)       // no collision found

#define PRESSED (1)             // key has been pressed
#define NOT_PRESSED (0)         // key hasn't been pressed 

/*----------------------------------------------------------------------------*/
/* Movement details of an object */
typedef struct {
    float xVel;                 // velocity in x direction
    float yVel;                 // velocity in y direction
    float xPos;                 // current x coordinate
    float yPos;                 // current y coordinate
} movement_t;

/* Paddle object*/
typedef struct {
    SDL_Texture* texture;       // texture object 
    SDL_Rect drawn_object;      // what gets drawn to the window
    movement_t movement;        // current movement details of object
} paddle_t;

/* Ball object */
typedef struct {
    SDL_Texture* texture;       // texture object 
    SDL_Rect drawn_object;      // what gets drawn to the window
    movement_t movement;        // current movement details of object
} ball_t;

/* Possible key options for the game */
typedef struct {
    int left_up;                // to move left paddle up
    int left_down;              // to move left paddle down
    int right_up;               // to move right paddle up
    int right_down;             // to move right paddle down
    int close_request;          // has the close button been clicked?
} keys_t;

/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
int
main(int argc, char* argv[]) {

     /* Initialise SDL */
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER)) {
        printf("Error init: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }



    /* Create a window */
    SDL_Window* window = SDL_CreateWindow("Shit Pong",
                                        SDL_WINDOWPOS_CENTERED,
                                        SDL_WINDOWPOS_CENTERED,
                                        WINDOW_WIDTH,
                                        WINDOW_HEIGHT,
                                        0);
    
    /* Check if the window was created successfully */
    if (!window) {
        printf("Error window init: %s\n", SDL_GetError());
        /* Exit out */
        SDL_Quit();
        return EXIT_FAILURE;
    }



    /* Initialise game objects */
    paddle_t leftpaddle;
    paddle_t rightpaddle;
    ball_t ball;
    keys_t actions;


    /* Create renderer */
    SDL_Renderer* render = SDL_CreateRenderer(
                                window, -1, SDL_RENDERER_ACCELERATED);

    /* Check renderer was created successfully*/
    if (!render) {
        printf("Error render init: %s\n", SDL_GetError());
        /* Exit out */
        SDL_DestroyWindow(window);
        SDL_Quit();
        return EXIT_FAILURE;
    }



    /* Load image surfaces */
    SDL_Surface* blue_paddle_surface = IMG_Load("resources/bluebar.png");
    SDL_Surface* red_paddle_surface = IMG_Load("resources/redbar.png");
    SDL_Surface* ball_surface = IMG_Load("resources/whiteball.png");

    /* Check surfaces loaded correctly */
    if (!blue_paddle_surface || !red_paddle_surface || !ball_surface) {
        printf("Error surface init: %s\n", SDL_GetError());
        SDL_DestroyRenderer(render);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return EXIT_FAILURE;
    }

    /* Load surfaces onto each texture */
    leftpaddle.texture = SDL_CreateTextureFromSurface(
                                                render, blue_paddle_surface);
    rightpaddle.texture = SDL_CreateTextureFromSurface(
                                                render, red_paddle_surface);
    ball.texture = SDL_CreateTextureFromSurface(render, ball_surface);

    /* Free surface memory */
    SDL_FreeSurface(blue_paddle_surface);
    SDL_FreeSurface(red_paddle_surface);
    SDL_FreeSurface(ball_surface);

    /* Check textures loaded correctly */
    if (!leftpaddle.texture|| !rightpaddle.texture || !ball.texture) {
        printf("Error texture init: %s\n", SDL_GetError());
        SDL_DestroyRenderer(render);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return EXIT_FAILURE;
    }



    /* Scale textures to appropriate size */
    SDL_QueryTexture(leftpaddle.texture, NULL, NULL,
                     &leftpaddle.drawn_object.w, &leftpaddle.drawn_object.h);
    SDL_QueryTexture(rightpaddle.texture, NULL, NULL,
                     &rightpaddle.drawn_object.w, &rightpaddle.drawn_object.h);
    SDL_QueryTexture(ball.texture, NULL, NULL,
                     &ball.drawn_object.w, &ball.drawn_object.h);
    
    leftpaddle.drawn_object.h /= 4;
    leftpaddle.drawn_object.w /= 4;
    rightpaddle.drawn_object.h /= 4;
    rightpaddle.drawn_object.w /= 4;
    ball.drawn_object.h /= 8;
    ball.drawn_object.w /= 8;




    /* Put sprites into starting positions */
    leftpaddle.movement.xPos = 100;
    leftpaddle.movement.yPos = (
                            WINDOW_HEIGHT - leftpaddle.drawn_object.h) / 2;

    rightpaddle.movement.xPos = (
                            WINDOW_WIDTH - 100 - rightpaddle.drawn_object.w);
    rightpaddle.movement.yPos = (
                            WINDOW_HEIGHT - rightpaddle.drawn_object.h) / 2;

    ball.movement.xPos = (WINDOW_WIDTH - ball.drawn_object.w) / 2;
    ball.movement.yPos = (WINDOW_HEIGHT - ball.drawn_object.h) / 2;



    /* Initialise starting velocities */
    leftpaddle.movement.xVel = 0;
    leftpaddle.movement.yVel = 0;

    rightpaddle.movement.xVel = 0;
    rightpaddle.movement.yVel = 0;

    ball.movement.xVel = SPEED;
    ball.movement.yVel = -SPEED;

    

    /* Initialise all keystrokes to unpressed */
    actions.left_down = 0;
    actions.left_up = 0;
    actions.right_down = 0;
    actions.right_up = 0;
    actions.close_request = 0;







    /* ============ ANIMATION LOOP ============ */
    while (actions.close_request == NOT_PRESSED) {
        /* Process incoming actions */
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                
                /* Pressed `X` to close the program */
                case SDL_QUIT:
                    actions.close_request = PRESSED;
                    break;
                
                /* A key has been clicked */
                case SDL_KEYDOWN:
                    switch (event.key.keysym.scancode) {
                        
                        /* Left paddle first */
                        case SDL_SCANCODE_W:
                            actions.left_up = PRESSED;
                            break;
                        case SDL_SCANCODE_S:
                            actions.left_down = PRESSED;
                            break;

                        /* Right paddle */
                        case SDL_SCANCODE_UP:
                            actions.right_up = PRESSED;
                            break;
                        case SDL_SCANCODE_DOWN:
                            actions.right_down = PRESSED;
                            break;

                        default :
                            ;
                    }
                    break;
                /* Keys not pressed */
                case SDL_KEYUP:
                    switch (event.key.keysym.scancode) {
                        /* Left paddle first */
                        case SDL_SCANCODE_W:
                            actions.left_up = NOT_PRESSED;
                            break;
                        case SDL_SCANCODE_S:
                            actions.left_down = NOT_PRESSED;
                            break;

                        /* Right paddle */
                        case SDL_SCANCODE_UP:
                            actions.right_up = NOT_PRESSED;
                            break;
                        case SDL_SCANCODE_DOWN:
                            actions.right_down = NOT_PRESSED;
                            break;

                        default :
                            ;
                    }
                    break;
            }
        }

        /* Determine velocity changes */
        /* Reset paddle velocity each frame */
        leftpaddle.movement.yVel = 0;
        rightpaddle.movement.yVel = 0;
        
        /* Keystroke logic */
        if (actions.left_up == PRESSED && actions.left_down == NOT_PRESSED) {
            leftpaddle.movement.yVel = -SPEED;
        }
        if (actions.left_down == PRESSED && actions.left_up == NOT_PRESSED) {
            leftpaddle.movement.yVel = SPEED;
        }

        if (actions.right_up == PRESSED && actions.right_down == NOT_PRESSED) {
            rightpaddle.movement.yVel = -SPEED;
        }
        if (actions.right_down == PRESSED && actions.right_up == NOT_PRESSED) {
            rightpaddle.movement.yVel = SPEED;
        }

        

        /* Update object positions */
        leftpaddle.movement.yPos += leftpaddle.movement.yVel / 60;
        rightpaddle.movement.yPos += rightpaddle.movement.yVel / 60;
        ball.movement.xPos += ball.movement.xVel / 60;
        ball.movement.yPos += ball.movement.yVel / 60;



        /* Check for collisions with ball and paddle */
        if (SDL_HasIntersection(&ball.drawn_object, &leftpaddle.drawn_object) == SDL_TRUE && ball.movement.xVel < 0) {
            ball.movement.xVel = -ball.movement.xVel;
        }
        if (SDL_HasIntersection(&ball.drawn_object, &rightpaddle.drawn_object) == SDL_TRUE && ball.movement.xVel > 0) {
            ball.movement.xVel = -ball.movement.xVel;
        }

        /* Check paddle collision with walls */
        if (leftpaddle.movement.yPos >= WINDOW_HEIGHT - leftpaddle.drawn_object.h) {
            leftpaddle.movement.yPos = WINDOW_HEIGHT - leftpaddle.drawn_object.h;
        }
        if (rightpaddle.movement.yPos >= WINDOW_HEIGHT - rightpaddle.drawn_object.h) {
            rightpaddle.movement.yPos = WINDOW_HEIGHT - rightpaddle.drawn_object.h;
        }
        if (leftpaddle.movement.yPos <= 0) {
            leftpaddle.movement.yPos = 0;
        }
        if (rightpaddle.movement.yPos <= 0) {
            rightpaddle.movement.yPos = 0;
        }

        /* Ball collision with roof and floor */
        if (ball.movement.yPos <= 0) {
            ball.movement.yPos = 0;
            ball.movement.yVel = -ball.movement.yVel;
        }
        if (ball.movement.yPos >= WINDOW_HEIGHT - ball.drawn_object.h) {
            ball.movement.yPos = WINDOW_HEIGHT - ball.drawn_object.h;
            ball.movement.yVel = -ball.movement.yVel;
        }

        /* Ball gets past a paddle */
        if (ball.movement.xPos <= 0 || ball.movement.xPos >= WINDOW_WIDTH - ball.drawn_object.w) {
            ball.movement.xPos = (WINDOW_WIDTH - ball.drawn_object.w) / 2;
            ball.movement.yPos = (WINDOW_HEIGHT - ball.drawn_object.h) / 2;
        }



        /* Update positions */
        leftpaddle.drawn_object.x = (int) leftpaddle.movement.xPos;
        leftpaddle.drawn_object.y = (int) leftpaddle.movement.yPos;

        rightpaddle.drawn_object.x = (int) rightpaddle.movement.xPos;
        rightpaddle.drawn_object.y = (int) rightpaddle.movement.yPos;

        ball.drawn_object.x = (int) ball.movement.xPos;
        ball.drawn_object.y = (int) ball.movement.yPos;



        /* Clear window for double buffer */
        SDL_RenderClear(render);

        /* Draw new images */
        SDL_RenderCopy(render, leftpaddle.texture, NULL, &leftpaddle.drawn_object);
        SDL_RenderCopy(render, rightpaddle.texture, NULL, &rightpaddle.drawn_object);
        SDL_RenderCopy(render, ball.texture, NULL, &ball.drawn_object);
        SDL_RenderPresent(render);

        /* 60 Fps */
        SDL_Delay(1000/60);
    }   



    /* Clean up */
    SDL_DestroyTexture(leftpaddle.texture);
    SDL_DestroyTexture(rightpaddle.texture);
    SDL_DestroyTexture(ball.texture);
    SDL_DestroyRenderer(render);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}
