#ifndef _WIN32
#error only windows platform support
#else
#define NOMINMAX
#endif

#include <iostream>
#include <memory>
#include <cmath>

#include "SDL2/SDL.h"
#include "SDL2/SDL_syswm.h"

#include "shader.hpp"
#include "texture.hpp"
#include "math.hpp"

float vw{};
float vh{};

namespace
{
GLuint emptyVAO;
std::optional<Shader> fullscreenShader;
}

class Window
{
public:
    Window(const std::string& title, int width, int height, Vector2D position) :
        mPosition{position}, mWidth{width}, mHeight{height}, mTitle{title}
    {
        mpWindow = SDL_CreateWindow(mTitle.c_str(), static_cast<int>(mPosition.x), static_cast<int>(mPosition.y), mWidth, mHeight,
            SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_BORDERLESS);
        assert(mpWindow);

        SDL_SysWMinfo wmInfo{};
        SDL_GetWindowWMInfo(mpWindow, &wmInfo);

        SetWindowPos(wmInfo.info.win.window, HWND_TOPMOST, static_cast<int>(mPosition.x), static_cast<int>(mPosition.y),
            0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }

    virtual ~Window()
    {
        SDL_DestroyWindow(mpWindow);
        mpWindow = nullptr;
    }

    void update()
    {
        SDL_SetWindowPosition(mpWindow, static_cast<int>(mPosition.x), static_cast<int>(mPosition.y));
    }

protected:
    int mWidth{}, mHeight{};
    SDL_Window* mpWindow{};
    Vector2D mPosition;

private:
    std::string mTitle;
};

enum class EntityType
{
    DEFAULT,
    PLAYER,
    ENEMY,
    COUMT
};

class Entity final : public Window
{
public:
    inline static float maxSpeed{};

    Entity(EntityType type, Vector2D position, int width, int height, float speed = vw * 3, const std::string& title = "")
        : Window{title, width, height, position}, mSpeed{speed}
    {
        mpContext = SDL_GL_CreateContext(mpWindow);
        assert(mpContext);

        static bool isGladInit{};
        if (!isGladInit)
        {
            gladLoadGLLoader(SDL_GL_GetProcAddress);
        }
        isGladInit = true;

        mpTexture = std::make_unique<Texture>("face.png");
    }

    ~Entity()
    {
        SDL_GL_DeleteContext(mpContext);
        mpContext = nullptr;
    }

    void draw()
    {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glViewport(0, 0, mWidth, mHeight);

        mpTexture->bind();

        fullscreenShader->use();
        glBindVertexArray(emptyVAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        SDL_GL_SwapWindow(mpWindow);
    }

    void moveLeft() { mVelocity.x = -mSpeed; }
    void moveRight() { mVelocity.x = mSpeed; }
    void moveDown() { mVelocity.y = mSpeed; }
    void moveUp() { mVelocity.y = -mSpeed; }

    void update(const float deltaTime)
    {
        applyFriction(static_cast<float>(vw * 5));

        mVelocity.x = std::max(std::min(mVelocity.x, maxSpeed), -maxSpeed);
        mVelocity.y = std::max(std::min(mVelocity.y, maxSpeed), -maxSpeed);

        mPosition += mVelocity * deltaTime;

        Window::update();
    }

private:
    SDL_GLContext mpContext{};
    std::unique_ptr<Texture> mpTexture;
    Vector2D mVelocity;
    float mSpeed;

    void applyFriction(float friction)
    {
        mVelocity *= friction;
    }
};

class App
{
public:
    App()
    {
        assert(SDL_Init(SDL_INIT_VIDEO) == 0);
    }

    ~App()
    {
        glDeleteVertexArrays(1, &emptyVAO);

        SDL_Quit();
    }
};

int main(int argc, char** argv) try
{
    App app;

    SDL_DisplayMode mode;
    SDL_GetDisplayMode(0, 0, &mode);
    vw = (mode.w / 100.f);
    vh = (mode.h / 100.f);
    Entity::maxSpeed = vw * 20;

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    Entity player{EntityType::PLAYER, {mode.w / 2.f, mode.h / 2.f}, static_cast<int>(10 * vw), static_cast<int>(10 * vw)};

    glGenVertexArrays(1, &emptyVAO);

    fullscreenShader = std::make_optional<Shader>("shader.vert", "shader.frag");

    assert(Shader::isShaderCompilationsSuccess());

    fullscreenShader->use();

    unsigned lastTime{SDL_GetTicks()}, currentTime{};
    float deltaTime;
    bool gameIsRunning{true};
    while (gameIsRunning)
    {
        currentTime = SDL_GetTicks();
        deltaTime = (currentTime - lastTime) / 1000.f;

        lastTime = currentTime;

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                gameIsRunning = false;
            }

            const Uint8* state = SDL_GetKeyboardState(NULL);
            if (state[SDL_SCANCODE_RIGHT])
            {
                player.moveRight();
            }
            if (state[SDL_SCANCODE_LEFT])
            {
                player.moveLeft();
            }
            if (state[SDL_SCANCODE_UP])
            {
                player.moveUp();
            }
            if (state[SDL_SCANCODE_DOWN])
            {
                player.moveDown();
            }
        }

        player.update(deltaTime);
        player.draw();
    }

    return EXIT_SUCCESS;
}
catch (const std::exception& e)
{
    std::cerr << "STL_EXCEPTION: " << e.what() << "\n";
    return EXIT_FAILURE;
}
catch (...)
{
    std::cerr << "UNKNOWN ERROR\n";
    return EXIT_FAILURE;
}