#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_RGB_Image.H>
#include <vector>
#include <chrono>

// Constants
const int WIDTH = 800;
const int HEIGHT = 800;
const int MAX_ITER = 1000;
const int TILE_SIZE = 100;
const int REDRAW_INTERVAL_MS = 50;

// Global zoom and offset
double zoom = 1.0;
double offsetX = 0.0;
double offsetY = 0.0;

std::vector<unsigned char> mandelbrotData(WIDTH * HEIGHT * 3);

// Renders a specific tile of the Mandelbrot set
void renderTile(int startX, int endX, int startY, int endY) {
    for (int y = startY; y < endY; y++) {
        for (int x = startX; x < endX; x++) {
            double zx = (2.0 * (x - WIDTH * 0.5) / (zoom * WIDTH)) + offsetX;
            double zy = (2.0 * (y - HEIGHT * 0.5) / (zoom * HEIGHT)) + offsetY;

            double x2 = 0.0;
            double y2 = 0.0;
            int iter = 0;

            while (x2 + y2 < 4.0 && iter < MAX_ITER) {
                double tmp = x2 * x2 - y2 * y2 + zx;
                y2 = 2.0 * x2 * y2 + zy;
                x2 = tmp;
                iter++;
            }

            unsigned char color = iter % 256;
            size_t index = (y * WIDTH + x) * 3;
            mandelbrotData[index] = mandelbrotData[index + 1] = mandelbrotData[index + 2] = color;
        }
    }
}

// Render the entire Mandelbrot set
std::vector<unsigned char>& renderMandelbrot(double zoom, double offsetX, double offsetY) {
    #pragma omp parallel for collapse(2)
    for (int ty = 0; ty < HEIGHT; ty += TILE_SIZE) {
        for (int tx = 0; tx < WIDTH; tx += TILE_SIZE) {
            renderTile(tx, std::min(tx + TILE_SIZE, WIDTH), ty, std::min(ty + TILE_SIZE, HEIGHT));
        }
    }
    return mandelbrotData;
}

// Custom window to display the Mandelbrot set
class MandelbrotWindow : public Fl_Double_Window {
public:
    MandelbrotWindow(int w, int h, const char* l = 0)
        : Fl_Double_Window(w, h, l), lastX(0), lastY(0) {
        mandelbrotImage = new Fl_RGB_Image(mandelbrotData.data(), WIDTH, HEIGHT, 3);
    }

    ~MandelbrotWindow() {
        delete mandelbrotImage;
    }

    // Handling mouse events for interaction
    int handle(int event) override {
        static auto lastRedraw = std::chrono::high_resolution_clock::now();

        double prevZoom = zoom;
        double prevOffsetX = offsetX;
        double prevOffsetY = offsetY;

        switch (event) {
        case FL_PUSH:
            lastX = Fl::event_x();
            lastY = Fl::event_y();
            return 1;

        case FL_DRAG:
            offsetX += (double)(lastX - Fl::event_x()) / (WIDTH * zoom * 0.5);
            offsetY += (double)(lastY - Fl::event_y()) / (HEIGHT * zoom * 0.5);
            lastX = Fl::event_x();
            lastY = Fl::event_y();
            break;

        case FL_MOUSEWHEEL:
            if (Fl::event_dy() != 0) {
                double mouseX = (double)Fl::event_x() / WIDTH;
                double mouseY = (double)Fl::event_y() / HEIGHT;
                offsetX -= (0.5 - mouseX) / zoom;
                offsetY -= (0.5 - mouseY) / zoom;
                zoom *= (Fl::event_dy() > 0) ? 0.9 : 1.1;
                offsetX -= (0.5 - mouseX) / zoom;
                offsetY -= (0.5 - mouseY) / zoom;
            }
            break;
        }

        auto now = std::chrono::high_resolution_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastRedraw).count() > REDRAW_INTERVAL_MS) {
            if (zoom != prevZoom || offsetX != prevOffsetX || offsetY != prevOffsetY) {
                redraw();
                lastRedraw = now;
            }
        }

        return Fl_Double_Window::handle(event);
    }

    // Draw the Mandelbrot set on the window
    void draw() override {
        Fl_Double_Window::draw();
        renderMandelbrot(zoom, offsetX, offsetY);
        mandelbrotImage->uncache();
        mandelbrotImage->draw(0, 0, w(), h());
    }

private:
    int lastX, lastY;
    Fl_RGB_Image* mandelbrotImage;
};

int main(int argc, char** argv) {
//    mpfr::mpreal::set_default_prec(256); // Set precision to 256 bits, adjust as needed
    MandelbrotWindow window(WIDTH, HEIGHT, "Mandelbrot Set Renderer");
    window.show(argc, argv);
    return Fl::run();
}
