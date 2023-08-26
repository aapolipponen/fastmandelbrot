#include <mpfr.h>
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_RGB_Image.H>
#include <vector>
#include <chrono>
#include <cmath>

// MPFR Utility functions
void init_mpfr_var(mpfr_t &var, long precision = 256) {
    mpfr_init2(var, precision);
}

void set_d(mpfr_t &var, double val) {
    mpfr_set_d(var, val, MPFR_RNDN);
}

double get_d(const mpfr_t &var) {
    return mpfr_get_d(var, MPFR_RNDN);
}

void add_d(mpfr_t &result, const mpfr_t &var, double val) {
    mpfr_t temp;
    init_mpfr_var(temp);
    set_d(temp, val);
    mpfr_add(result, var, temp, MPFR_RNDN);
    mpfr_clear(temp);
}

const int WIDTH = 800;
const int HEIGHT = 800;
const int REDRAW_INTERVAL_MS = 50;

int MAX_ITER = 1000;
int TILE_SIZE = 100;
bool useSmoothColoring = true;

// Convert zoom and offsets to MPFR
mpfr_t zoom, offsetX, offsetY;
std::vector<unsigned char> mandelbrotData(WIDTH * HEIGHT * 3);

// Initialization of MPFR variables
void initGlobals() {
    init_mpfr_var(zoom);
    init_mpfr_var(offsetX);
    init_mpfr_var(offsetY);
    set_d(zoom, 1.0);
    set_d(offsetX, 0.0);
    set_d(offsetY, 0.0);
}

// Renders a specific tile of the Mandelbrot set
void renderTile(int tileX, int tileY, int width, int height) {
    mpfr_t tmp, x2, y2, two, zx, zy, tmp2, iter_mpfr, nu;  // Assuming you have these variables
    mpfr_init(tmp);
    mpfr_init(x2);
    mpfr_init(y2);
    mpfr_init(two);
    mpfr_init(zx);
    mpfr_init(zy);
    mpfr_init(tmp2);
    mpfr_init(iter_mpfr);
    mpfr_init(nu);

    // Assuming you set the values for zx, zy, etc somewhere here...

    for (int y = tileY; y < tileY + height; y++) {
        for (int x = tileX; x < tileX + width; x++) {

            mpfr_set_d(tmp, (double)x, MPFR_RNDN);
            mpfr_sub_d(tmp, tmp, WIDTH * 0.5, MPFR_RNDN);

            mpfr_mul(tmp, x2, x2, MPFR_RNDN);
            mpfr_mul(y2, y2, y2, MPFR_RNDN);
            mpfr_add(tmp, tmp, y2, MPFR_RNDN);

            mpfr_sub(tmp, x2, y2, MPFR_RNDN);
            mpfr_add(tmp, tmp, zx, MPFR_RNDN);

            mpfr_mul(y2, x2, y2, MPFR_RNDN);
            mpfr_mul(y2, y2, two, MPFR_RNDN);
            mpfr_add(y2, y2, zy, MPFR_RNDN);

            mpfr_set_si(tmp2, x, MPFR_RNDN);
            mpfr_sub_d(tmp, tmp2, WIDTH * 0.5, MPFR_RNDN);
            mpfr_add_ui(iter_mpfr, iter_mpfr, 1, MPFR_RNDN);
        
            }
    }
            
    // Clean up after usage
    mpfr_clears(tmp, x2, y2, two, zx, zy, tmp2, iter_mpfr, nu, (mpfr_ptr) 0);
}

class MandelbrotWindow : public Fl_Double_Window {
public:
    MandelbrotWindow(int w, int h, const char* l = 0)
        : Fl_Double_Window(w, h, l), lastX(0), lastY(0) {
        mandelbrotImage = new Fl_RGB_Image(mandelbrotData.data(), WIDTH, HEIGHT, 3);

        init_mpfr_var(zoom);
        init_mpfr_var(offsetX);
        init_mpfr_var(offsetY);
        set_d(zoom, 1.0);
        set_d(offsetX, 0.0);
        set_d(offsetY, 0.0);
    }

    ~MandelbrotWindow() {
        delete mandelbrotImage;

        mpfr_clear(zoom);
        mpfr_clear(offsetX);
        mpfr_clear(offsetY);
    }
        
    int handle(int event) override {
        static auto lastRedraw = std::chrono::high_resolution_clock::now();

        mpfr_t prevZoom, prevOffsetX, prevOffsetY;
        init_mpfr_var(prevZoom);
        init_mpfr_var(prevOffsetX);
        init_mpfr_var(prevOffsetY);

        mpfr_set(prevZoom, zoom, MPFR_RNDN);
        mpfr_set(prevOffsetX, offsetX, MPFR_RNDN);
        mpfr_set(prevOffsetY, offsetY, MPFR_RNDN);

        switch (event) {
        case FL_PUSH:
            lastX = Fl::event_x();
            lastY = Fl::event_y();
            return 1;

        case FL_DRAG: {
            mpfr_t tmp1, tmp2;
            init_mpfr_var(tmp1);
            init_mpfr_var(tmp2);

            // offsetX += (double)(lastX - Fl::event_x()) / (WIDTH * zoom * 0.5);
            set_d(tmp1, (double)(lastX - Fl::event_x()));
            mpfr_set_d(tmp2, WIDTH, MPFR_RNDN);
            mpfr_div(tmp2, tmp2, zoom, MPFR_RNDN);
            mpfr_div_d(tmp2, tmp2, 2.0, MPFR_RNDN);
            mpfr_div(tmp1, tmp1, tmp2, MPFR_RNDN);
            mpfr_add(offsetX, offsetX, tmp1, MPFR_RNDN);

            // offsetY += (double)(lastY - Fl::event_y()) / (HEIGHT * zoom * 0.5);
            set_d(tmp1, (double)(lastY - Fl::event_y()));
            mpfr_set_d(tmp2, HEIGHT, MPFR_RNDN);
            mpfr_div(tmp2, tmp2, zoom, MPFR_RNDN);
            mpfr_div_d(tmp2, tmp2, 2.0, MPFR_RNDN);
            mpfr_div(tmp1, tmp1, tmp2, MPFR_RNDN);
            mpfr_add(offsetY, offsetY, tmp1, MPFR_RNDN);

            mpfr_clear(tmp1);
            mpfr_clear(tmp2);

            lastX = Fl::event_x();
            lastY = Fl::event_y();
            break;
        }

        case FL_MOUSEWHEEL:
            if (Fl::event_dy() != 0) {
                mpfr_t mouseX, mouseY, zoomFactor, tmp;
                init_mpfr_var(mouseX);
                init_mpfr_var(mouseY);
                init_mpfr_var(zoomFactor);
                init_mpfr_var(tmp);

                set_d(mouseX, (double)Fl::event_x() / WIDTH);
                set_d(mouseY, (double)Fl::event_y() / HEIGHT);
                set_d(zoomFactor, 4.0);

                // offsetX computations
                mpfr_set_d(tmp, 0.5, MPFR_RNDN);
                mpfr_sub(tmp, tmp, mouseX, MPFR_RNDN); 
                mpfr_div(tmp, tmp, zoom, MPFR_RNDN);
                mpfr_mul(tmp, tmp, zoomFactor, MPFR_RNDN);
                mpfr_sub(offsetX, offsetX, tmp, MPFR_RNDN);

                // offsetY computations
                mpfr_sub_d(tmp, 0.5, mouseY, MPFR_RNDN);
                mpfr_div(tmp, tmp, zoom, MPFR_RNDN);
                mpfr_mul(tmp, tmp, zoomFactor, MPFR_RNDN);
                mpfr_sub(offsetY, offsetY, tmp, MPFR_RNDN);

                // zoom *= (Fl::event_dy() > 0) ? 0.9 : 1.1;
                if (Fl::event_dy() > 0) {
                    mpfr_mul_d(zoom, zoom, 0.9, MPFR_RNDN);
                } else {
                    mpfr_mul_d(zoom, zoom, 1.1, MPFR_RNDN);
                }

                // offsetX computations after zoom update
                mpfr_set_d(tmp, 0.5, MPFR_RNDN);
                mpfr_sub(tmp, tmp, mouseX, MPFR_RNDN);                mpfr_div(tmp, tmp, zoom, MPFR_RNDN);
                mpfr_mul(tmp, tmp, zoomFactor, MPFR_RNDN);
                mpfr_add(offsetX, offsetX, tmp, MPFR_RNDN);

                // offsetY computations after zoom update
                mpfr_sub_d(tmp, 0.5, mouseY, MPFR_RNDN);
                mpfr_div(tmp, tmp, zoom, MPFR_RNDN);
                mpfr_mul(tmp, tmp, zoomFactor, MPFR_RNDN);
                mpfr_add(offsetY, offsetY, tmp, MPFR_RNDN);

                mpfr_clear(mouseX);
                mpfr_clear(mouseY);
                mpfr_clear(zoomFactor);
                mpfr_clear(tmp);
            }
            break;
        }

        auto now = std::chrono::high_resolution_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastRedraw).count() > REDRAW_INTERVAL_MS) {
            if (mpfr_cmp(zoom, prevZoom) != 0 || mpfr_cmp(offsetX, prevOffsetX) != 0 || mpfr_cmp(offsetY, prevOffsetY) != 0) {
                redraw();
                lastRedraw = now;
            }
        }

        mpfr_clear(prevZoom);
        mpfr_clear(prevOffsetX);
        mpfr_clear(prevOffsetY);

        return Fl_Double_Window::handle(event);
    }

    void draw() override {
        Fl_Double_Window::draw();
        renderMandelbrot(zoom, offsetX, offsetY);
        mandelbrotImage->uncache();
        mandelbrotImage->draw(0, 0, w(), h());
    }

private:
    int lastX, lastY;
    Fl_RGB_Image* mandelbrotImage;
    mpfr_t zoom, offsetX, offsetY;
};

int main(int argc, char** argv) {
    MandelbrotWindow window(WIDTH, HEIGHT, "Mandelbrot Set Renderer");
    window.show(argc, argv);
    return Fl::run();
}