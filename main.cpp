#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_RGB_Image.H>
#include <vector>
#include <complex>
#include <omp.h>

const int WIDTH = 800;
const int HEIGHT = 600;
const int MAX_ITER = 1000;

double zoom = 1.0;
double offsetX = 0.0;
double offsetY = 0.0;

std::vector<unsigned char> image(WIDTH * HEIGHT * 3);
Fl_RGB_Image* mandelbrotImage = nullptr;

std::vector<unsigned char>& renderMandelbrot(double zoom, double offsetX, double offsetY) {
    #pragma omp parallel for schedule(dynamic)
    for(int y = 0; y < HEIGHT; y++) {
        for(int x = 0; x < WIDTH; x++) {
            double zx = (1.5 * (x - WIDTH / 2) / (0.5 * zoom * WIDTH)) + offsetX;
            double zy = ((y - HEIGHT / 2) / (0.5 * zoom * HEIGHT)) + offsetY;
            std::complex<double> c(zx, zy);
            std::complex<double> z = c;
            int iter = 0;
            while(abs(z) < 2.0 && iter < MAX_ITER) {
                z = z * z + c;
                iter++;
            }

            unsigned char color = iter % 256;
            image[(y * WIDTH + x) * 3] = color;
            image[(y * WIDTH + x) * 3 + 1] = color;
            image[(y * WIDTH + x) * 3 + 2] = color;
        }
    }
    return image;
}

class MandelbrotWindow : public Fl_Window {
public:
    MandelbrotWindow(int w, int h, const char* l = 0)
        : Fl_Window(w, h, l), lastX(0), lastY(0) {}

    int handle(int event) override {
        double prevZoom = zoom;
        double prevOffsetX = offsetX;
        double prevOffsetY = offsetY;
        
        switch(event) {
            case FL_PUSH:
                lastX = Fl::event_x();
                lastY = Fl::event_y();
                return 1;
            case FL_DRAG:
                offsetX += (double)(lastX - Fl::event_x()) / (WIDTH * zoom);
                offsetY += (double)(lastY - Fl::event_y()) / (HEIGHT * zoom);
                lastX = Fl::event_x();
                lastY = Fl::event_y();
                break;
            case FL_MOUSEWHEEL:
                if(Fl::event_dy() > 0) {
                    zoom /= 1.1;
                } else if(Fl::event_dy() < 0) {
                    double mouseX = (double)Fl::event_x() / WIDTH;
                    double mouseY = (double)Fl::event_y() / HEIGHT;
                    offsetX += (0.5 - mouseX) / zoom;
                    offsetY += (0.5 - mouseY) / zoom;
                    zoom *= 1.1;
                    offsetX -= (0.5 - mouseX) / zoom;
                    offsetY -= (0.5 - mouseY) / zoom;
                }
                break;
        }

        if (zoom != prevZoom || offsetX != prevOffsetX || offsetY != prevOffsetY) {
            redraw();
        }

        return Fl_Window::handle(event);
    }

    void draw() override {
        Fl_Window::draw();
        std::vector<unsigned char>& imageRef = renderMandelbrot(zoom, offsetX, offsetY);
        if (mandelbrotImage) {
            delete mandelbrotImage;
        }
        mandelbrotImage = new Fl_RGB_Image(&imageRef[0], WIDTH, HEIGHT, 3);
        mandelbrotImage->draw(0, 0, w(), h());
    }

private:
    int lastX, lastY;
};

int main(int argc, char **argv) {
    MandelbrotWindow window(WIDTH, HEIGHT, "Mandelbrot Set Renderer");
    window.show(argc, argv);
    return Fl::run();
}
