#include <iostream>
#include <memory>

using namespace std;

class Window {
public:
    virtual ~Window() = default;
    virtual void draw() const = 0;
};

class Button {
public:
    virtual ~Button() = default;
    virtual void draw() const = 0;
};

class Shader {
public:
    virtual ~Shader() = default;
    virtual void compile() const = 0;
};

class OpenGLWindow : public Window {
public:
    void draw() const override { cout << "[OpenGL] Dibujando Window\n"; }
};

class OpenGLButton : public Button {
public:
    void draw() const override { cout << "[OpenGL] Dibujando Button\n"; }
};

class OpenGLShader : public Shader {
public:
    void compile() const override { cout << "[OpenGL] Compilando Shader\n"; }
};

class VulkanWindow : public Window {
public:
    void draw() const override { cout << "[Vulkan] Dibujando Window\n"; }
};

class VulkanButton : public Button {
public:
    void draw() const override { cout << "[Vulkan] Dibujando Button\n"; }
};

class VulkanShader : public Shader {
public:
    void compile() const override { cout << "[Vulkan] Compilando Shader\n"; }
};

class RenderFactory {
public:
    virtual ~RenderFactory() = default;
    virtual unique_ptr<Window> createWindow() const = 0;
    virtual unique_ptr<Button> createButton() const = 0;
    virtual unique_ptr<Shader> createShader() const = 0;
};

class OpenGLFactory : public RenderFactory {
public:
    unique_ptr<Window> createWindow() const override { return make_unique<OpenGLWindow>(); }
    unique_ptr<Button> createButton() const override { return make_unique<OpenGLButton>(); }
    unique_ptr<Shader> createShader() const override { return make_unique<OpenGLShader>(); }
};

class VulkanFactory : public RenderFactory {
public:
    unique_ptr<Window> createWindow() const override { return make_unique<VulkanWindow>(); }
    unique_ptr<Button> createButton() const override { return make_unique<VulkanButton>(); }
    unique_ptr<Shader> createShader() const override { return make_unique<VulkanShader>(); }
};

// El cliente solo conoce RenderFactory: no sabe que plataforma hay detras
void renderScene(const RenderFactory& factory) {
    auto window = factory.createWindow();
    auto button = factory.createButton();
    auto shader = factory.createShader();
    shader->compile();
    window->draw();
    button->draw();
}

int main() {
    unique_ptr<RenderFactory> factory = make_unique<OpenGLFactory>();
    auto window = factory->createWindow();
    auto button = factory->createButton();
    window->draw();
    button->draw();

    factory = make_unique<VulkanFactory>();
    auto shader = factory->createShader();
    shader->compile();

    cout << "\n--- Escena completa con OpenGL ---\n";
    renderScene(OpenGLFactory());
    cout << "\n--- Escena completa con Vulkan ---\n";
    renderScene(VulkanFactory());

    return 0;
}
