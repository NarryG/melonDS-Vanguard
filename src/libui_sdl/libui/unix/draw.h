// 5 may 2016

// draw.c
struct uiDrawContext {
	cairo_t *cr;
};

struct uiDrawBitmap {
    int Width;
    int Height;
    int Stride;

    cairo_surface_t* bmp;
};

// drawpath.c
extern void runPath(uiDrawPath *p, cairo_t *cr);
extern uiDrawFillMode pathFillMode(uiDrawPath *path);

// drawmatrix.c
extern void m2c(uiDrawMatrix *m, cairo_matrix_t *c);
