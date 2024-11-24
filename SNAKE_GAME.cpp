#include <SDL2/SDL.h>
#include "SDL_ttf.h"
#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <string>
using namespace std;



const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int SNAKE_SIZE = 25;
const int OBSTACLE_SIZE = 50;

enum Direction { UP, DOWN, LEFT, RIGHT };

enum GameState { MENU, PLAYING, GAME_OVER, PAUSED, LEVEL_UP, COUNTDOWN };

struct SnakeSegment {
    int x, y;
};
struct Color {
    Uint8 r, g, b, a;
};

struct RandomSnake {
    vector<SnakeSegment> segments;
    Direction direction;
    Uint32 lastMoveTime;
    int moveInterval;
};

SDL_Texture* backgroundTexture = nullptr;
SDL_Texture* appleTexture = nullptr;
SDL_Texture* gameOverBackgroundTexture = nullptr;
SDL_Texture* pauseBackgroundTexture = nullptr;
SDL_Texture* startBackgroundTexture = nullptr;
SDL_Texture* stoneTexture = nullptr;
SDL_Texture* bananaTexture = nullptr;

Color startColor = {0, 204, 0, 255};
Color endColor = {0, 102, 0, 255};

Color calculateGradientColor(const Color& start, const Color& end, float t) {
    Color result;
    result.r = start.r + t * (end.r - start.r);
    result.g = start.g + t * (end.g - start.g);
    result.b = start.b + t * (end.b - start.b);    //for snake body
    result.a = start.a + t * (end.a - start.a);
    return result;
}

inline int customMax(int a, int b) {
    return (a > b) ? a : b;
}

bool init(SDL_Window*& window, SDL_Renderer*& renderer, TTF_Font*& font) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << endl;
        return false;
    }

    if (TTF_Init() == -1) {
        cerr << "SDL_ttf could not initialize! TTF_Error: " << TTF_GetError() << endl;
        SDL_Quit();
        return false;
    }

    window = SDL_CreateWindow("Snake Game by sumuuu", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                              SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << endl;
        SDL_Quit();
        TTF_Quit();
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        TTF_Quit();
        return false;
    }

    font = TTF_OpenFont("/Library/Fonts/Arial Unicode.ttf", 24);
    if (!font) {
        cerr << "Failed to load font! TTF_Error: " << TTF_GetError() << endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        TTF_Quit();
        return false;
    }

    return true;
}

SDL_Texture* loadTexture(SDL_Renderer* renderer, const string& path) {
    SDL_Surface* loadedSurface = SDL_LoadBMP(path.c_str());
    if (!loadedSurface) {                 //here we are keeping the photo in surface and thn converting it into texture
        cerr << "Unable to load image " << path << "! SDL Error: " << SDL_GetError() << endl;
        return nullptr;
    }
    SDL_Texture* newTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
    SDL_FreeSurface(loadedSurface);
    return newTexture;
}

bool loadMedia(SDL_Renderer* renderer) {
    
    backgroundTexture = loadTexture(renderer, "background.bmp");
    if (!backgroundTexture) {
        return false;
    }
                               //pic show korar jonno .
    appleTexture = loadTexture(renderer, "apple.bmp");
    if (!appleTexture) {
        return false;
    }
   
    gameOverBackgroundTexture = loadTexture(renderer, "background2.bmp");
    if (!gameOverBackgroundTexture) {
        return false;
    }
   
    pauseBackgroundTexture = loadTexture(renderer, "background2.bmp");
    if (!pauseBackgroundTexture) {
        return false;
    }
   
    startBackgroundTexture = loadTexture(renderer, "background2.bmp");
    if (!startBackgroundTexture) {
        return false;
    }
   
    stoneTexture = loadTexture(renderer, "stone.bmp");
    if (!stoneTexture) {
        return false;
    }
    
    bananaTexture = loadTexture(renderer, "banana.bmp");
    if (!bananaTexture) {
        return false;
    }
    return true;
}

void close(SDL_Window* window, SDL_Renderer* renderer, TTF_Font* font) {
    TTF_CloseFont(font);
    SDL_DestroyTexture(backgroundTexture);
    SDL_DestroyTexture(appleTexture);
    SDL_DestroyTexture(gameOverBackgroundTexture);
    SDL_DestroyTexture(pauseBackgroundTexture);
    SDL_DestroyTexture(startBackgroundTexture);
    SDL_DestroyTexture(stoneTexture);
    SDL_DestroyTexture(bananaTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
}

void renderSnake(SDL_Renderer* renderer, const vector<SnakeSegment>& snake) {
    int numSegments = snake.size();
    for (int i = 0; i < numSegments; ++i) {
        float t = static_cast<float>(i) / (numSegments - 1);
        Color currentColor = calculateGradientColor(startColor, endColor, t);

        
        SDL_SetRenderDrawColor(renderer, currentColor.r, currentColor.g, currentColor.b, currentColor.a);
        SDL_Rect fillRect = { snake[i].x, snake[i].y, SNAKE_SIZE, SNAKE_SIZE };
        SDL_RenderFillRect(renderer, &fillRect);

         SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // for border (black colour)
        SDL_RenderDrawRect(renderer, &fillRect);

        
        if (i == 0) {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); //  for eye red colour .
            SDL_Rect eyeRect = { snake[i].x + SNAKE_SIZE / 4, snake[i].y + SNAKE_SIZE / 4, SNAKE_SIZE / 5, SNAKE_SIZE / 5 };
            SDL_RenderFillRect(renderer, &eyeRect);

          
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // for tongue red colour
            SDL_Rect tongueRect = { snake[i].x + SNAKE_SIZE / 2, snake[i].y + SNAKE_SIZE, SNAKE_SIZE / 5, SNAKE_SIZE / 2 };
            SDL_RenderFillRect(renderer, &tongueRect);
        }
    }
}

void renderFood(SDL_Renderer* renderer, int foodX, int foodY) {
    SDL_Rect destRect = { foodX, foodY, SNAKE_SIZE, SNAKE_SIZE };
    SDL_RenderCopy(renderer, appleTexture, nullptr, &destRect);
}
void renderBanana(SDL_Renderer* renderer, int bananaX, int bananaY) {
    SDL_Rect destRect = { bananaX, bananaY, SNAKE_SIZE, SNAKE_SIZE };
    SDL_RenderCopy(renderer, bananaTexture, nullptr, &destRect);
}
void renderObstacles(SDL_Renderer* renderer, const vector<SDL_Rect>& obstacles) {
    for (const auto& obstacle : obstacles) {
        SDL_RenderCopy(renderer, stoneTexture, nullptr, &obstacle);
    }
}
void renderScore(SDL_Renderer* renderer, TTF_Font* font, int score) {
    SDL_Color textColor = { 0, 0, 0, 255 }; // Black color
    string scoreText = "Score: " + to_string(score);
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, scoreText.c_str(), textColor);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    int textWidth = textSurface->w;
    int textHeight = textSurface->h;
    SDL_FreeSurface(textSurface);
    SDL_Rect renderQuad = { 10, 10, textWidth, textHeight };
    SDL_RenderCopy(renderer, textTexture, nullptr, &renderQuad);
    SDL_DestroyTexture(textTexture);
}
void renderGameOver(SDL_Renderer* renderer, TTF_Font* font, int score) {
    SDL_Color textColor = { 0, 0, 0, 255 }; // Black color for text
    string gameOverText = "Game Over!! Final Score: " + to_string(score);

   
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, gameOverText.c_str(), textColor);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    int textWidth = textSurface->w;
    int textHeight = textSurface->h;
    SDL_FreeSurface(textSurface);

  
    int boxWidth = textWidth + 60;
    int boxHeight = textHeight + 60;
    SDL_Rect backgroundQuad = { (SCREEN_WIDTH - boxWidth) / 2, (SCREEN_HEIGHT - boxHeight) / 2, boxWidth, boxHeight };


    SDL_Rect renderQuad = { (SCREEN_WIDTH - textWidth) / 2, (SCREEN_HEIGHT - textHeight) / 2, textWidth, textHeight };

   
    SDL_RenderCopy(renderer, gameOverBackgroundTexture, nullptr, &backgroundQuad);

    
    SDL_RenderCopy(renderer, textTexture, nullptr, &renderQuad);

  
    SDL_DestroyTexture(textTexture);
}
void renderPause(SDL_Renderer* renderer, TTF_Font* font) {
    SDL_Color textColor = { 0, 0, 0, 255 }; // Black color for text
    string pauseText = "Game Paused. Press 'P' to resume.";

   
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, pauseText.c_str(), textColor);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    int textWidth = textSurface->w;
    int textHeight = textSurface->h;
    SDL_FreeSurface(textSurface);


    int boxWidth = textWidth + 76;
    int boxHeight = textHeight + 76;
    SDL_Rect backgroundQuad = { (SCREEN_WIDTH - boxWidth) / 2, (SCREEN_HEIGHT - boxHeight) / 2, boxWidth, boxHeight };

  
    SDL_Rect renderQuad = { (SCREEN_WIDTH - textWidth) / 2, (SCREEN_HEIGHT - textHeight) / 2, textWidth, textHeight };

    SDL_RenderCopy(renderer, pauseBackgroundTexture, nullptr, &backgroundQuad);
    SDL_RenderCopy(renderer, textTexture, nullptr, &renderQuad);
    SDL_DestroyTexture(textTexture);
}
void renderStartScreen(SDL_Renderer* renderer, TTF_Font* font) {
    SDL_Color textColor = { 0, 0, 0, 255 }; // Black color for text
    string startText = "Press 'Enter' to Start";

  
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, startText.c_str(), textColor);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    int textWidth = textSurface->w;
    int textHeight = textSurface->h;
    SDL_FreeSurface(textSurface);

    
    int boxWidth = textWidth + 60;
    int boxHeight = textHeight + 60;
    SDL_Rect backgroundQuad = { (SCREEN_WIDTH - boxWidth) / 2, (SCREEN_HEIGHT - boxHeight) / 2, boxWidth, boxHeight };


    SDL_Rect renderQuad = { (SCREEN_WIDTH - textWidth) / 2, (SCREEN_HEIGHT - textHeight) / 2, textWidth, textHeight };

 
    SDL_RenderCopy(renderer, startBackgroundTexture, nullptr, &backgroundQuad);

    
    SDL_RenderCopy(renderer, textTexture, nullptr, &renderQuad);

    SDL_DestroyTexture(textTexture);
}
void renderLevelUp(SDL_Renderer* renderer, TTF_Font* font, const string& message) {
    SDL_Color textColor = { 0, 0, 0, 255 };
    string levelUpText1 = "Congo!! You are on " + message;
    string levelUpText2 = message == "level 2" ? "Be aware of the RUSSELL's VIPER SNAKE." : "Be aware of the stone.";

 
    SDL_Surface* textSurface1 = TTF_RenderText_Blended(font, levelUpText1.c_str(), textColor);
    SDL_Texture* textTexture1 = SDL_CreateTextureFromSurface(renderer, textSurface1);
    int textWidth1 = textSurface1->w;
    int textHeight1 = textSurface1->h;

    SDL_Surface* textSurface2 = TTF_RenderText_Blended(font, levelUpText2.c_str(), textColor);
    SDL_Texture* textTexture2 = SDL_CreateTextureFromSurface(renderer, textSurface2);
    int textWidth2 = textSurface2->w;
    int textHeight2 = textSurface2->h;

   
    int boxWidth = customMax(textWidth1, textWidth2) + 80;
    int boxHeight = textHeight1 + textHeight2 + 80;
    SDL_Rect backgroundQuad = { (SCREEN_WIDTH - boxWidth) / 2, (SCREEN_HEIGHT - boxHeight) / 2, boxWidth, boxHeight };


    SDL_Rect renderQuad1 = { (SCREEN_WIDTH - textWidth1) / 2, (SCREEN_HEIGHT - boxHeight) / 2 + 20, textWidth1, textHeight1 };
    SDL_Rect renderQuad2 = { (SCREEN_WIDTH - textWidth2) / 2, (SCREEN_HEIGHT - boxHeight) / 2 + textHeight1 + 40, textWidth2, textHeight2 };

    
    SDL_RenderCopy(renderer, gameOverBackgroundTexture, nullptr, &backgroundQuad);

   
    SDL_RenderCopy(renderer, textTexture1, nullptr, &renderQuad1);
    SDL_RenderCopy(renderer, textTexture2, nullptr, &renderQuad2);

    
    SDL_DestroyTexture(textTexture1);
    SDL_FreeSurface(textSurface1);
    SDL_DestroyTexture(textTexture2);
    SDL_FreeSurface(textSurface2);
}
void renderCountdownTimer(SDL_Renderer* renderer, TTF_Font* font, Uint32 countdownStartTime, Uint32 countdownDuration) {
    Uint32 currentTime = SDL_GetTicks();
    Uint32 elapsedTime = currentTime - countdownStartTime;
    Uint32 remainingTime = countdownDuration - elapsedTime;

    if (remainingTime > 0) {
        SDL_Color textColor = { 0, 0, 0, 255 };
        string timerText = "Resuming in: " + to_string(remainingTime / 1000) + "s";
        SDL_Surface* textSurface = TTF_RenderText_Solid(font, timerText.c_str(), textColor);
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        int textWidth = textSurface->w;
        int textHeight = textSurface->h;
        SDL_FreeSurface(textSurface);
        SDL_Rect renderQuad = { (SCREEN_WIDTH - textWidth) / 2, (SCREEN_HEIGHT - textHeight) / 2, textWidth, textHeight };
        SDL_RenderCopy(renderer, textTexture, nullptr, &renderQuad);
        SDL_DestroyTexture(textTexture);
    }
}
void renderBananaTimer(SDL_Renderer* renderer, TTF_Font* font, Uint32 bananaSpawnTime, Uint32 bananaLifetime) {
    Uint32 currentTime = SDL_GetTicks();
    Uint32 elapsedTime = currentTime - bananaSpawnTime;
    Uint32 remainingTime = bananaLifetime - elapsedTime;

    if (remainingTime > 0) {
        SDL_Color textColor = { 0, 0, 0, 255 };
        string timerText = "Banana disappears in: " + to_string(remainingTime / 1000) + "s";
        SDL_Surface* textSurface = TTF_RenderText_Solid(font, timerText.c_str(), textColor);
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        int textWidth = textSurface->w;
        int textHeight = textSurface->h;
        SDL_FreeSurface(textSurface);
        SDL_Rect renderQuad = { SCREEN_WIDTH - textWidth - 10, 10, textWidth, textHeight };
        SDL_RenderCopy(renderer, textTexture, nullptr, &renderQuad);
        SDL_DestroyTexture(textTexture);
    }
}


void renderRandomSnake(SDL_Renderer* renderer, const RandomSnake& randomSnake) {
    int numSegments = randomSnake.segments.size();
    for (int i = 0; i < numSegments; ++i) {
        float t = static_cast<float>(i) / (numSegments - 1);
        Color currentColor = calculateGradientColor({255, 165, 0, 255}, {255, 140, 0, 255}, t);

        
        SDL_SetRenderDrawColor(renderer, currentColor.r, currentColor.g, currentColor.b, currentColor.a);
        SDL_Rect fillRect = { randomSnake.segments[i].x, randomSnake.segments[i].y, SNAKE_SIZE, SNAKE_SIZE };
        SDL_RenderFillRect(renderer, &fillRect);

        
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderDrawRect(renderer, &fillRect);

   
        if (i == 0) {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Red color for eye
            SDL_Rect eyeRect = { randomSnake.segments[i].x + SNAKE_SIZE / 4, randomSnake.segments[i].y + SNAKE_SIZE / 4, SNAKE_SIZE / 5, SNAKE_SIZE / 5 };
            SDL_RenderFillRect(renderer, &eyeRect);

            
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Red color for tongue
            SDL_Rect tongueRect = { randomSnake.segments[i].x + SNAKE_SIZE / 2, randomSnake.segments[i].y + SNAKE_SIZE, SNAKE_SIZE / 5, SNAKE_SIZE / 2 };
            SDL_RenderFillRect(renderer, &tongueRect);
        }
    }
}

void updateSnake(vector<SnakeSegment>& snake, Direction direction, bool& grow) {
    SnakeSegment newHead = snake.front();
    switch (direction) {
        case UP: newHead.y -= SNAKE_SIZE; break;
        case DOWN: newHead.y += SNAKE_SIZE; break;
        case LEFT: newHead.x -= SNAKE_SIZE; break;
        case RIGHT: newHead.x += SNAKE_SIZE; break;
    }
    snake.insert(snake.begin(), newHead);
    if (!grow) {
        snake.pop_back();
    } else {
        grow = false;
    }
}

void updateRandomSnake(RandomSnake& randomSnake, const vector<SDL_Rect>& obstacles, Uint32 currentTime) {
    if (currentTime - randomSnake.lastMoveTime > randomSnake.moveInterval) {
       
        if (rand() % 4 == 0) {
            randomSnake.direction = static_cast<Direction>(rand() % 4);
        }

       
        SnakeSegment newHead = randomSnake.segments.front();
        switch (randomSnake.direction) {
            case UP: newHead.y -= SNAKE_SIZE; break;
            case DOWN: newHead.y += SNAKE_SIZE; break;
            case LEFT: newHead.x -= SNAKE_SIZE; break;
            case RIGHT: newHead.x += SNAKE_SIZE; break;
        }

  
        if (newHead.x < 0) newHead.x = SCREEN_WIDTH - SNAKE_SIZE;
        else if (newHead.x >= SCREEN_WIDTH) newHead.x = 0;
        if (newHead.y < 0) newHead.y = SCREEN_HEIGHT - SNAKE_SIZE;
        else if (newHead.y >= SCREEN_HEIGHT) newHead.y = 0;

        bool collision = false;
        for (const auto& obstacle : obstacles) {
            if (newHead.x < obstacle.x + obstacle.w && newHead.x + SNAKE_SIZE > obstacle.x &&
                newHead.y < obstacle.y + obstacle.h && newHead.y + SNAKE_SIZE > obstacle.y) {
                collision = true;
                break;
            }
        }

        if (!collision) {
            randomSnake.segments.insert(randomSnake.segments.begin(), newHead);
            randomSnake.segments.pop_back();
            randomSnake.lastMoveTime = currentTime;
        }
    }
}

bool checkFoodCollision(int foodX, int foodY, const SnakeSegment& head) {
    return head.x == foodX && head.y == foodY;
}


bool checkBananaCollision(int bananaX, int bananaY, const SnakeSegment& head) {
    return head.x == bananaX && head.y == bananaY;
}

bool checkRandomSnakeCollision(const vector<SnakeSegment>& snake, const RandomSnake& randomSnake) {
    const SnakeSegment& head = snake.front();
    for (const auto& segment : randomSnake.segments) {
        if (head.x == segment.x && head.y == segment.y) {
            return true;
        }
    }
    return false;
}

bool checkCollision(const vector<SnakeSegment>& snake, const vector<SDL_Rect>& obstacles) {
    const SnakeSegment& head = snake.front();
    if (head.x < 0 || head.x >= SCREEN_WIDTH || head.y < 0 || head.y >= SCREEN_HEIGHT) {
        return true;
    }
    for (size_t i = 1; i < snake.size(); ++i) {
        if (head.x == snake[i].x && head.y == snake[i].y) {
            return true;
        }
    }
    for (const auto& obstacle : obstacles) {

        if (head.x < obstacle.x + obstacle.w && head.x + SNAKE_SIZE > obstacle.x &&
            head.y < obstacle.y + obstacle.h && head.y + SNAKE_SIZE > obstacle.y) {
            return true;
        }
    }
    return false;
}


void generateFood(int& foodX, int& foodY, const vector<SnakeSegment>& snake, const vector<SDL_Rect>& obstacles, const RandomSnake& randomSnake) {
    bool validPosition = false;
    while (!validPosition) {
        validPosition = true;
        foodX = (rand() % (SCREEN_WIDTH / SNAKE_SIZE)) * SNAKE_SIZE;
        foodY = (rand() % (SCREEN_HEIGHT / SNAKE_SIZE)) * SNAKE_SIZE;
        for (const auto& segment : snake) {
            if (segment.x == foodX && segment.y == foodY) {
                validPosition = false;
                break;
            }
        }
        for (const auto& obstacle : obstacles) {
            if (obstacle.x == foodX && obstacle.y == foodY) {
                validPosition = false;
                break;
            }
        }
        for (const auto& segment : randomSnake.segments) {
            if (segment.x == foodX && segment.y == foodY) {
                validPosition = false;
                break;
            }
        }
    }
}


void generateBanana(int& bananaX, int& bananaY, const vector<SnakeSegment>& snake, const vector<SDL_Rect>& obstacles, const RandomSnake& randomSnake) {
    bool validPosition = false;
    while (!validPosition) {
        validPosition = true;
        bananaX = (rand() % (SCREEN_WIDTH / SNAKE_SIZE)) * SNAKE_SIZE;
        bananaY = (rand() % (SCREEN_HEIGHT / SNAKE_SIZE)) * SNAKE_SIZE;
        for (const auto& segment : snake) {
            if (segment.x == bananaX && segment.y == bananaY) {
                validPosition = false;
                break;
            }
        }
        for (const auto& obstacle : obstacles) {
            if (obstacle.x == bananaX && obstacle.y == bananaY) {
                validPosition = false;
                break;
            }
        }
        for (const auto& segment : randomSnake.segments) {
            if (segment.x == bananaX && segment.y == bananaY) {
                validPosition = false;
                break;
            }
        }
    }
}


void generateObstacles(vector<SDL_Rect>& obstacles, const vector<SnakeSegment>& snake) {
    int numObstacles = 3;
    obstacles.clear();
    for (int i = 0; i < numObstacles; ++i) {
        SDL_Rect newObstacle;
        bool validPosition = false;
        while (!validPosition) {
            validPosition = true;
            newObstacle.x = (rand() % (SCREEN_WIDTH / OBSTACLE_SIZE)) * OBSTACLE_SIZE;
            newObstacle.y = (rand() % (SCREEN_HEIGHT / OBSTACLE_SIZE)) * OBSTACLE_SIZE;
            newObstacle.w = OBSTACLE_SIZE;
            newObstacle.h = OBSTACLE_SIZE;
            for (const auto& segment : snake) {
                if (segment.x == newObstacle.x && segment.y == newObstacle.y) {
                    validPosition = false;
                    break;
                }
            }
            for (const auto& obstacle : obstacles) {
                if (obstacle.x == newObstacle.x && obstacle.y == newObstacle.y) {
                    validPosition = false;
                    break;
                }
            }
        }
        obstacles.push_back(newObstacle);
    }
}
void handleEvents(SDL_Event& e, Direction& direction, bool& quit, GameState& state) {
    while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT) {
            quit = true;
        } else if (e.type == SDL_KEYDOWN) {
            switch (e.key.keysym.sym) {
                case SDLK_UP: if (direction != DOWN) direction = UP; break;
                case SDLK_DOWN: if (direction != UP) direction = DOWN; break;
                case SDLK_LEFT: if (direction != RIGHT) direction = LEFT; break;
                case SDLK_RIGHT: if (direction != LEFT) direction = RIGHT; break;
                case SDLK_p: if (state == PLAYING) state = PAUSED; else if (state == PAUSED) state = PLAYING; break;
                case SDLK_RETURN: if (state == MENU) state = PLAYING; break;
            }
        }
    }
}

int main(int argc, char* args[]) {
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    TTF_Font* font = nullptr;

    if (!init(window, renderer, font)) {
        cerr << "Failed to initialize!" << endl;
        return 1;
    }

    if (!loadMedia(renderer)) {
        cerr << "Failed to load media!" << endl;
        close(window, renderer, font);
        return 1;
    }

    srand(static_cast<unsigned int>(time(nullptr)));


    vector<SnakeSegment> snake = { {SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2} };
    Direction direction = RIGHT;
    bool grow = false;
    int foodX, foodY;
    int bananaX, bananaY;
    Uint32 bananaSpawnTime = 0;
    bool bananaActive = false;
    const Uint32 bananaLifetime = 5000;
    vector<SDL_Rect> obstacles;
    int score = 0;
    bool quit = false;
    GameState state = MENU;
    int initialSnakeSpeed = 130;
    int snakeSpeed = initialSnakeSpeed;
    int maxSnakeSpeed = 50;
    Uint32 levelUpStartTime = 0;
    SDL_Event e;
    int pointsSinceLastBanana = 0; 
    bool levelUpTriggered = false;
    string currentLevel = "level 1";

   
    RandomSnake randomSnake;
    randomSnake.segments.clear();
    int startX = (rand() % (SCREEN_WIDTH / SNAKE_SIZE)) * SNAKE_SIZE;
    int startY = (rand() % (SCREEN_HEIGHT / SNAKE_SIZE)) * SNAKE_SIZE;
    for (int i = 0; i < 3; ++i) {
        randomSnake.segments.push_back({ startX + i * SNAKE_SIZE, startY });
    }
    randomSnake.direction = static_cast<Direction>(rand() % 4);
    randomSnake.lastMoveTime = SDL_GetTicks();
    randomSnake.moveInterval = 500;
    bool randomSnakeActive = false;

    Uint32 countdownStartTime = 0;
    const Uint32 countdownDuration = 3000;
    bool countdownActive = false;


    generateFood(foodX, foodY, snake, obstacles, randomSnake);
    cout << "Initial Food Position: (" << foodX << ", " << foodY << ")\n";

   
    while (!quit) {
        handleEvents(e, direction, quit, state);

        if (state == PLAYING) {
            
            Uint32 currentTime = SDL_GetTicks();
            updateSnake(snake, direction, grow);

            if (checkFoodCollision(foodX, foodY, snake.front())) {
                grow = true;
                score++;
                pointsSinceLastBanana++;
                generateFood(foodX, foodY, snake, obstacles, randomSnake);
                cout << "New Food Position: (" << foodX << ", " << foodY << ")\n";

               
                if (!levelUpTriggered && score >= 8 && currentLevel == "level 1") {
                    state = LEVEL_UP;
                    levelUpStartTime = SDL_GetTicks();
                    levelUpTriggered = true;
                    currentLevel = "level 2";
                } else if (!levelUpTriggered && score >= 15 && currentLevel == "level 2") {
                    state = LEVEL_UP;
                    levelUpStartTime = SDL_GetTicks();
                    generateObstacles(obstacles, snake);
                    levelUpTriggered = true;
                    currentLevel = "level 3";
                }

                
                if (currentLevel == "level 2") {
                    randomSnakeActive = true;
                }
            }

            if (bananaActive && checkBananaCollision(bananaX, bananaY, snake.front())) {
                grow = true;
                score += 3;
                bananaActive = false;
                pointsSinceLastBanana = 0;
            }

            if (checkCollision(snake, obstacles) || (randomSnakeActive && checkRandomSnakeCollision(snake, randomSnake))) {
                state = GAME_OVER;
            }

      
            snakeSpeed = customMax(maxSnakeSpeed, initialSnakeSpeed - (snake.size() - 1) * 5);

            
            if (score >= 5 && pointsSinceLastBanana >= 3 && !bananaActive) {
                generateBanana(bananaX, bananaY, snake, obstacles, randomSnake);
                bananaSpawnTime = SDL_GetTicks();
                bananaActive = true;
            }

          
            if (bananaActive && SDL_GetTicks() - bananaSpawnTime >= bananaLifetime) {
                bananaActive = false;
            }

            if (randomSnakeActive) {
                updateRandomSnake(randomSnake, obstacles, currentTime);
            }

           
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, backgroundTexture, nullptr, nullptr);
            renderSnake(renderer, snake);
            renderFood(renderer, foodX, foodY);
            if (bananaActive) {
                renderBanana(renderer, bananaX, bananaY);
                renderBananaTimer(renderer, font, bananaSpawnTime, bananaLifetime);
            }
            renderObstacles(renderer, obstacles);
            renderScore(renderer, font, score);
            if (randomSnakeActive) {
                renderRandomSnake(renderer, randomSnake);
            }
            SDL_RenderPresent(renderer);

        } else if (state == LEVEL_UP) {
          
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, backgroundTexture, nullptr, nullptr);
            renderLevelUp(renderer, font, currentLevel);
            SDL_RenderPresent(renderer);

         
            if (!countdownActive) {
                countdownStartTime = SDL_GetTicks();
                countdownActive = true;
            }

           
            Uint32 currentTime = SDL_GetTicks();
            if (countdownActive && currentTime - countdownStartTime >= 3000) {
                countdownActive = false;
                state = COUNTDOWN;
                countdownStartTime = SDL_GetTicks();
            }

        } else if (state == COUNTDOWN) {
           
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, backgroundTexture, nullptr, nullptr);
            renderSnake(renderer, snake);
            renderFood(renderer, foodX, foodY);
            if (bananaActive) {
                renderBanana(renderer, bananaX, bananaY);
            }
            renderObstacles(renderer, obstacles);
            renderScore(renderer, font, score);
            if (randomSnakeActive) {
                renderRandomSnake(renderer, randomSnake);
            }


            renderCountdownTimer(renderer, font, countdownStartTime, countdownDuration);
            SDL_RenderPresent(renderer);

            Uint32 currentTime = SDL_GetTicks();
            if (currentTime - countdownStartTime >= countdownDuration) {
                state = PLAYING;
                levelUpTriggered = false;
            }

        } else if (state == GAME_OVER) {
          
            SDL_RenderCopy(renderer, backgroundTexture, nullptr, nullptr);
            renderSnake(renderer, snake);
            renderFood(renderer, foodX, foodY);
            renderScore(renderer, font, score);
            renderGameOver(renderer, font, score);
            SDL_RenderPresent(renderer);

        } else if (state == PAUSED) {
     
            SDL_RenderCopy(renderer, backgroundTexture, nullptr, nullptr);
            renderSnake(renderer, snake);
            renderFood(renderer, foodX, foodY);
            renderScore(renderer, font, score);
            renderPause(renderer, font);
            SDL_RenderPresent(renderer);

        } else if (state == MENU) {
            
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, backgroundTexture, nullptr, nullptr);
            renderStartScreen(renderer, font);
            SDL_RenderPresent(renderer);
        }

        SDL_Delay(snakeSpeed);
    }

    close(window, renderer, font);
    return 0;
}
