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
    inline static float scaleFactor{};

    Window(const std::string& title, int width, int height, Vector2D position) :
        mPosition{position}, mWidth{width}, mHeight{height}, mTitle{title}
    {
        mpWindow = SDL_CreateWindow(mTitle.c_str(), static_cast<int>(mPosition.x), static_cast<int>(mPosition.y), mWidth, mHeight,
            SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_BORDERLESS);
        assert(mpWindow);
    }

    virtual ~Window()
    {
        SDL_DestroyWindow(mpWindow);
        mpWindow = nullptr;
    }

    void update()
    {
        SDL_SysWMinfo wmInfo{};
        SDL_GetWindowWMInfo(mpWindow, &wmInfo);

        SetWindowPos(wmInfo.info.win.window, HWND_TOPMOST, static_cast<int>(mPosition.x), static_cast<int>(mPosition.y),
            0, 0, SWP_NOSIZE);
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
    inline static float pushbackStrength{};

    Entity(EntityType type, Vector2D position, int width, int height, float speed = vw * 20, const std::string& title = "")
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
        applyFriction(static_cast<float>(-(vw * 0.05f)));

        mVelocity.x = std::max(std::min(mVelocity.x, maxSpeed), -maxSpeed);
        mVelocity.y = std::max(std::min(mVelocity.y, maxSpeed), -maxSpeed);


        SDL_DisplayMode mode{};
        SDL_GetDisplayMode(0, 0, &mode);

        const Vector2D newPos = mPosition + mVelocity * deltaTime;

        if (newPos.x < 0)
        {
            mVelocity.x = pushbackStrength;
        }
        else if (newPos.x + mWidth > mode.w)
        {
            mVelocity.x = -pushbackStrength;
        }

        if (newPos.y < 0)
        {
            mVelocity.y = pushbackStrength;
        }
        else if (newPos.y + mHeight > mode.h)
        {
            mVelocity.y = -pushbackStrength;
        }

        mPosition = mPosition + mVelocity * deltaTime;

        Window::update();
    }

private:
    SDL_GLContext mpContext{};
    std::unique_ptr<Texture> mpTexture;
    Vector2D mVelocity;
    float mSpeed;

    void applyFriction(float friction)
    {
        if (mVelocity.x > 0)
        {
            mVelocity.x = std::max(mVelocity.x + friction, 0.f);
        }
        else if (mVelocity.x < 0)
        {
            mVelocity.x = std::min(mVelocity.x - friction, 0.f);
        }

        if (mVelocity.y > 0)
        {
            mVelocity.y = std::max(mVelocity.y + friction, 0.f);
        }
        else if (mVelocity.y < 0)
        {
            mVelocity.y = std::min(mVelocity.y - friction, 0.f);
        }
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
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_SYSTEM_AWARE);

    App app;

    SDL_DisplayMode mode{};
    SDL_GetDisplayMode(0, 0, &mode);
    vw = (mode.w / 100.f);
    vh = (mode.h / 100.f);
    Entity::maxSpeed = vw * 20;
    Entity::pushbackStrength = vw * 100;

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    const int playerSize{static_cast<int>(10.f * vw)};
    Entity player{EntityType::PLAYER, {(mode.w / 2.f) - playerSize / 2.f, (mode.h / 2.f) - playerSize / 2.f}, playerSize, playerSize};

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