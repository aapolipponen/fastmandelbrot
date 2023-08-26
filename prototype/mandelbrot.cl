__kernel void mandelbrot(__global uchar *img, double offsetX, double offsetY, double zoom, int maxIter, int width, int height) {
    int x = get_global_id(0);
    int y = get_global_id(1);

    double zx = (2.0 * (x - width * 0.5) / (zoom * width)) + offsetX;
    double zy = (2.0 * (y - height * 0.5) / (zoom * height)) + offsetY;

    double x2 = 0.0;
    double y2 = 0.0;
    int iter = 0;

    while (x2 + y2 < 4.0 && iter < maxIter) {
        double tmp = x2 * x2 - y2 * y2 + zx;
        y2 = 2.0 * x2 * y2 + zy;
        x2 = tmp;
        iter++;
    }

    uchar color = (uchar)(iter % 256);
    int index = (y * width + x) * 3;
    img[index] = img[index + 1] = img[index + 2] = color;
}
