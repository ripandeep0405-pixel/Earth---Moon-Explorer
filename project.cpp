
//
// author: Gordon Griesel
// OpenGL
// lab starting framework
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>
#include "fonts.h"

typedef float Flt;
#define rnd() (Flt)rand() / (Flt)RAND_MAX
#define PI 3.14159265358979323846264338327950

class Image {
public:
    int width, height;
    unsigned char *data;

    ~Image() { delete [] data; }

    Image(const char *fname) {
        if (fname[0] == '\0')
            return;

        char name[40];
        strcpy(name, fname);
        int slen = strlen(name);
        name[slen-4] = '\0';

        char ppmname[80];
        sprintf(ppmname,"%s.ppm", name);

        char ts[100];
        sprintf(ts, "convert %s %s", fname, ppmname);
        system(ts);

        FILE *fpi = fopen(ppmname, "r");
        if (fpi) {
            char line[200];
            fgets(line, 200, fpi);
            fgets(line, 200, fpi);

            //skip comments and blank lines
            while (line[0] == '#' || strlen(line) < 2)
                fgets(line, 200, fpi);

            sscanf(line, "%i %i", &width, &height);
            fgets(line, 200, fpi);

            //get pixel data
            int n = width * height * 3;
            data = new unsigned char[n];

            for (int i=0; i<n; i++)
                data[i] = fgetc(fpi);

            fclose(fpi);
        } else {
            printf("ERROR opening image: %s\n", ppmname);
            exit(0);
        }
        unlink(ppmname);
    }
};

Image img[3]= {"earth_daymap.jpg", "galaxy.jpg","moon.jpg"};
unsigned char *highlightData = NULL;


void highlightNorthAmerica()
{
    int w = img[0].width;
    int h = img[0].height;

    // Start with the original Earth colors
    for (int i = 0; i < w * h * 3; i++) {
        highlightData[i] = img[0].data[i];
    }

    for (int y = 80; y < 445 && y < h; y++) {
        for (int x = 20; x < 720 && x < w; x++) {
            int p = (y * w + x) * 3;

            int r = img[0].data[p];
            int g = img[0].data[p + 1];
            int b = img[0].data[p + 2];

            // Skip pixels that look like ocean
            if (b > r * 1.2f && b > g * 1.2f)
                continue;

            // Blend original land colors with yellow
            // Strong yellow highlight (85% yellow, 15% original color)
            highlightData[p] =
            (unsigned char)(r * 0.15f + 255.0f * 0.85f);

            highlightData[p + 1] =
            (unsigned char)(g * 0.15f + 255.0f * 0.85f);

            highlightData[p + 2] =
            (unsigned char)(b * 0.15f);
        }
    }
}

void highlightSouthAmerica()
{
    int w = img[0].width;
    int h = img[0].height;

    // Start with the original Earth texture
    for (int i = 0; i < w*h*3; i++)
        highlightData[i] = img[0].data[i];

    // Approximate South America region
    for (int y = 450; y < 850 && y < h; y++) {
        for (int x = 500; x < 900 && x < w; x++) {

            int p = (y*w + x)*3;

            int r = img[0].data[p];
            int g = img[0].data[p+1];
            int b = img[0].data[p+2];

            // Skip blue ocean pixels
            if (b > r*1.2f && b > g*1.2f)
                continue;

            // Bright pink highlight
            highlightData[p] =
                (unsigned char)(r*0.15f + 255.0f*0.85f);

            highlightData[p+1] =
                (unsigned char)(g*0.15f + 40.0f*0.85f);

            highlightData[p+2] =
                (unsigned char)(b*0.15f + 180.0f*0.85f);
        }
    }
}
void highlightEurope()
{
    int w = img[0].width;
    int h = img[0].height;

    // Restore the original Earth colors
    for (int i = 0; i < w*h*3; i++)
        highlightData[i] = img[0].data[i];

    // Approximate outline of Europe on a 2048x1024 map
    // Points are ordered around the region.
    const int px[] = {
         965,  970, 1000, 1040, 1080,
        1120, 1200, 1235, 1250, 1220,
        1190, 1150, 1100, 1060, 1020,
         980,  965
    };

    const int py[] = {
         310,  260,  225,  170,  120,
         110,  120,  180,  235,  270,
         285,  300,  310,  295,  305,
         310,  310
    };

    const int n = sizeof(px) / sizeof(px[0]);

    for (int y = 100; y < 330 && y < h; y++) {
        for (int x = 950; x < 1270 && x < w; x++) {

            // Check if the pixel is inside the polygon
            bool inside = false;

            for (int i = 0, j = n - 1; i < n; j = i++) {
                if ((py[i] > y) != (py[j] > y)) {
                    float crossX = px[i] +
                        (float)(y - py[i]) *
                        (px[j] - px[i]) / (py[j] - py[i]);

                    if (x < crossX)
                        inside = !inside;
                }
            }

            if (!inside)
                continue;

            int p = (y*w + x)*3;

            int r = img[0].data[p];
            int g = img[0].data[p+1];
            int b = img[0].data[p+2];

            // Skip strongly blue ocean pixels
            if (b > r*1.2f && b > g*1.2f)
                continue;

            // Bright cyan highlight
            highlightData[p] =
                (unsigned char)(r*0.15f);

            highlightData[p+1] =
                (unsigned char)(g*0.15f + 255.0f*0.85f);

            highlightData[p+2] =
                (unsigned char)(b*0.15f + 255.0f*0.85f);
        }
    }
}
void highlightAsia()
{
    int w = img[0].width;
    int h = img[0].height;

    // Restore original Earth texture
    for (int i = 0; i < w*h*3; i++)
        highlightData[i] = img[0].data[i];

    // Approximate Asia polygon on the original texture
    // Coordinates are normalized from 0 to 1
    const float px[] = {
        0.57f, 0.59f, 0.66f, 0.74f, 0.84f,
        0.93f, 0.98f, 0.98f, 0.94f, 0.91f,
        0.90f, 0.86f, 0.85f, 0.82f, 0.81f,
        0.78f, 0.76f, 0.74f, 0.72f, 0.71f,
        0.69f, 0.67f, 0.66f, 0.64f, 0.62f,
        0.59f, 0.57f, 0.57f
    };
    const float py[] = {
    0.26f, 0.08f, 0.015f, 0.005f, 0.005f,
    0.025f, 0.08f, 0.28f, 0.34f, 0.40f,
    0.47f, 0.44f, 0.52f, 0.57f, 0.58f,
    0.49f, 0.47f, 0.46f, 0.54f, 0.50f,
    0.43f, 0.45f, 0.48f, 0.43f, 0.39f,
    0.39f, 0.33f, 0.26f
};
    const int n = sizeof(px) / sizeof(px[0]);

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {

            float u = (float)x / w;
            float v = (float)y / h;

            bool asia = false;

            // Point-in-polygon test
            for (int i = 0, j = n-1; i < n; j = i++) {
                if ((py[i] > v) != (py[j] > v)) {
                    float crossX = px[i] +
                        (v - py[i]) * (px[j] - px[i]) /
                        (py[j] - py[i]);

                    if (u < crossX)
                        asia = !asia;
                }
            }

            if (!asia)
                continue;

            int k = (y*w + x)*3;

            int r = img[0].data[k];
            int g = img[0].data[k+1];
            int b = img[0].data[k+2];

            // Skip strongly blue ocean pixels
            if (b > r*1.2f && b > g*1.2f)
                continue;

            // Strong orange highlight
            highlightData[k] =
                (unsigned char)(r*0.15f + 255.0f*0.85f);

            highlightData[k+1] =
                (unsigned char)(g*0.15f + 140.0f*0.85f);

            highlightData[k+2] =
                (unsigned char)(b*0.15f);
        }
    }
}
void highlightAfrica()
{
    int w = img[0].width;
    int h = img[0].height;

    // Restore original Earth texture
    for (int i = 0; i < w*h*3; i++)
        highlightData[i] = img[0].data[i];

    // Refined Africa polygon
    const int px[] = {
         930,  980, 1040, 1100, 1165, 1220, 1270, 1310,
        1335, 1305, 1275, 1250, 1265, 1245, 1210, 1170,
        1120, 1070, 1020,  970,  930,  900,  890,  905
    };

    const int py[] = {
         320,  305,  300,  302,  312,  332,  360,  400,
         455,  490,  520,  555,  610,  665,  720,  755,
         770,  740,  690,  620,  545,  455,  380,  330
    };

    const int n = sizeof(px) / sizeof(px[0]);

    for (int y = 290; y < 780 && y < h; y++) {
        for (int x = 880; x < 1360 && x < w; x++) {

            bool inside = false;

            for (int i = 0, j = n - 1; i < n; j = i++) {
                if ((py[i] > y) != (py[j] > y)) {
                    float crossX = px[i] +
                        (float)(y - py[i]) * (px[j] - px[i]) / (py[j] - py[i]);
                    if (x < crossX)
                        inside = !inside;
                }
            }

            if (!inside)
                continue;

            int p = (y * w + x) * 3;

            int r = img[0].data[p];
            int g = img[0].data[p + 1];
            int b = img[0].data[p + 2];

            // Skip blue ocean
            if (b > r * 1.2f && b > g * 1.2f)
                continue;

            // Yellow highlight
            highlightData[p]   = (unsigned char)(r * 0.15f + 255.0f * 0.85f);
            highlightData[p+1] = (unsigned char)(g * 0.15f + 255.0f * 0.85f);
            highlightData[p+2] = (unsigned char)(b * 0.15f);
        }
    }

    // Madagascar as separate small ellipse
    for (int y = 540; y < 690 && y < h; y++) {
        for (int x = 1285; x < 1355 && x < w; x++) {
            float dx = (x - 1320) / 24.0f;
            float dy = (y - 615) / 52.0f;
            if (dx*dx + dy*dy > 1.0f)
                continue;

            int p = (y * w + x) * 3;

            int r = img[0].data[p];
            int g = img[0].data[p + 1];
            int b = img[0].data[p + 2];

            if (b > r * 1.2f && b > g * 1.2f)
                continue;

            highlightData[p]   = (unsigned char)(r * 0.15f + 255.0f * 0.85f);
            highlightData[p+1] = (unsigned char)(g * 0.15f + 255.0f * 0.85f);
            highlightData[p+2] = (unsigned char)(b * 0.15f);
        }
    }

}
void highlightAustralia()
{
    int w = img[0].width;
    int h = img[0].height;

    // Restore original Earth texture
    for (int i = 0; i < w*h*3; i++)
        highlightData[i] = img[0].data[i];

    // Improved Australia polygon (2048x1024 texture)
    const int px[] = {
        1650, 1690, 1745, 1805, 1860, 1910, 1930,
        1920, 1885, 1830, 1770, 1710, 1665, 1635
    };

    const int py[] = {
        705, 635, 595, 585, 595, 635, 705,
        765, 815, 840, 830, 800, 760, 725
    };

    const int n = sizeof(px) / sizeof(px[0]);

    for (int y = 580; y < 850 && y < h; y++) {
        for (int x = 1620; x < 1940 && x < w; x++) {

            bool inside = false;

            // Point-in-polygon test
            for (int i = 0, j = n-1; i < n; j = i++) {
                if ((py[i] > y) != (py[j] > y)) {
                    float crossX = px[i] +
                        (float)(y - py[i]) *
                        (px[j] - px[i]) / (py[j] - py[i]);

                    if (x < crossX)
                        inside = !inside;
                }
            }

            if (!inside)
                continue;

            int p = (y*w + x)*3;

            int r = img[0].data[p];
            int g = img[0].data[p+1];
            int b = img[0].data[p+2];

            // Skip strongly blue ocean
            if (b > r*1.2f && b > g*1.2f)
                continue;

            // Bright purple highlight
            highlightData[p] =
                (unsigned char)(r*0.15f + 180.0f*0.85f);
            highlightData[p+1] =
                (unsigned char)(g*0.15f + 40.0f*0.85f);
            highlightData[p+2] =
                (unsigned char)(b*0.15f + 255.0f*0.85f);
        }
    }

    // Extra small pass for Tasmania
    for (int y = 835; y < 875 && y < h; y++) {
        for (int x = 1835; x < 1888 && x < w; x++) {

            int p = (y*w + x)*3;

            int r = img[0].data[p];
            int g = img[0].data[p+1];
            int b = img[0].data[p+2];

            if (b > r*1.2f && b > g*1.2f)
                continue;

            highlightData[p] =
                (unsigned char)(r*0.15f + 180.0f*0.85f);
            highlightData[p+1] =
                (unsigned char)(g*0.15f + 40.0f*0.85f);
            highlightData[p+2] =
                (unsigned char)(b*0.15f + 255.0f*0.85f);
        }
    }
}
void highlightAntarctica()
{
    int w = img[0].width;
    int h = img[0].height;

    // Restore original Earth texture
    for (int i = 0; i < w*h*3; i++)
        highlightData[i] = img[0].data[i];

    // Approximate Antarctic region near
    // the bottom of the 2048x1024 world texture
    for (int y = 860; y < h; y++) {
        for (int x = 0; x < w; x++) {

            int p = (y*w + x)*3;

            int r = img[0].data[p];
            int g = img[0].data[p+1];
            int b = img[0].data[p+2];

            // Select light-colored snow and ice
            if (r > 150 && g > 150 && b > 150) {

                // Bright icy blue
                highlightData[p] =
                    (unsigned char)(r*0.15f + 60.0f*0.85f);

                highlightData[p+1] =
                    (unsigned char)(g*0.15f + 220.0f*0.85f);

                highlightData[p+2] =
                    (unsigned char)(b*0.15f + 255.0f*0.85f);
            }
        }
    }
}
class Global {
public:
    int xres, yres;
    GLuint tex,tex2,tex3,tex4;

    GLfloat lightAmbient[4];
    GLfloat lightDiffuse[4];
    GLfloat lightSpecular[4];
    GLfloat lightPosition[4];

    int lesson_num;
    Flt rtri;
    Flt moonAngle = 0.0f;
    float speed = 1.0f;
    bool paused = false;
    int menuPage=0;
    int selectedLocation= 0;
    float targetRotation = 0.0f;
    bool movingToLocation = false;

    Global() {
        srand(time(NULL));
        xres = 640;
        yres = 480;

        GLfloat la[] = {0.0f, 0.0f, 0.0f, 1.0f};
        GLfloat ld[] = {1.0f, 1.0f, 1.0f, 1.0f};
        GLfloat ls[] = {0.5f, 0.5f, 0.5f, 1.0f};
        GLfloat lp[] = { 1.0f, 0.3f, 0.5f, 0.0f };
        memcpy(lightAmbient, la, sizeof(GLfloat)*4);
        memcpy(lightDiffuse, ld, sizeof(GLfloat)*4);
        memcpy(lightSpecular, ls, sizeof(GLfloat)*4);
        memcpy(lightPosition, lp, sizeof(GLfloat)*4);

        lesson_num = 0;
        rtri = 0.0f;
    }
} g;


//X11 functions
class X11_wrapper {
private:
    Display *dpy;
    Window win;
    GLXContext glc;

public:
    X11_wrapper(void) {
        GLint att[] = {
            GLX_RGBA, GLX_DEPTH_SIZE, 24,
            GLX_DOUBLEBUFFER, None
        };
        //GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, None };

        XSetWindowAttributes swa;
        setup_screen_res(640, 480);

        dpy = XOpenDisplay(NULL);
        if (dpy == NULL) {
            printf("\n\tcannot connect to X server\n\n");
            exit(EXIT_FAILURE);
        }

        Window root = DefaultRootWindow(dpy);
        XVisualInfo *vi = glXChooseVisual(dpy, 0, att);

        if (vi == NULL) {
            printf("\n\tno appropriate visual found\n\n");
            exit(EXIT_FAILURE);
        }

        Colormap cmap = XCreateColormap(
            dpy, root, vi->visual, AllocNone);

        swa.colormap = cmap;
        swa.event_mask = ExposureMask | KeyPressMask |
            KeyReleaseMask | StructureNotifyMask |
            SubstructureNotifyMask;

        win = XCreateWindow(
            dpy, root, 0, 0, g.xres, g.yres, 0,
            vi->depth, InputOutput, vi->visual,
            CWColormap | CWEventMask, &swa);

        set_title();

        glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
        glXMakeCurrent(dpy, win, glc);
    }

    ~X11_wrapper(void) {
        XDestroyWindow(dpy, win);
        XCloseDisplay(dpy);
    }

    void set_title(void) {
        //Set the window title bar.
        XMapWindow(dpy, win);
        XStoreName(dpy, win, "3480 OpenGL lab");
    }

    void setup_screen_res(const int w, const int h) {
        g.xres = w;
        g.yres = h;
    }

    void reshape_window(int width, int height) {
        //window has been resized.
        setup_screen_res(width, height);

        glViewport(0, 0, (GLint)width, (GLint)height);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glOrtho(0, g.xres, 0, g.yres, -1, 1);
        set_title();
    }

    void check_resize(XEvent *e) {
        //The ConfigureNotify is sent by the
        //server if the window is resized.
        if (e->type != ConfigureNotify)
            return;

        XConfigureEvent xce = e->xconfigure;
        if (xce.width != g.xres || xce.height != g.yres) {
            //Window size did change.
            reshape_window(xce.width, xce.height);
        }
    }

    bool getXPending() {
        return XPending(dpy);
    }

    XEvent getXNextEvent() {
        XEvent e;
        XNextEvent(dpy, &e);
        return e;
    }

    void swapBuffers() {
        glXSwapBuffers(dpy, win);
    }
} x11;


void init_opengl(void);
int check_keys(XEvent *e);
void physics(void);
void render(void);
void Moon(void);
void drawOrbit(void);

int main(void)
{
    init_opengl();

    //Do this to allow fonts
    glEnable(GL_TEXTURE_2D);
    initialize_fonts();

    int done = 0;

    while (!done) {
        while (x11.getXPending()) {
            XEvent e = x11.getXNextEvent();
            x11.check_resize(&e);
            done = check_keys(&e);
        }

        physics();
        render();
        x11.swapBuffers();
    }

    cleanup_fonts();
    return 0;
}


void init_opengl(void)
{
    //OpenGL initialization
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0);
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(
        45.0f,
        (GLfloat)g.xres/(GLfloat)g.yres,
        0.1f, 100.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0,5,10, 0,0,0, 0,1,0);

    //Enable this so material colors are the same
    //as vertex colors.
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);

    glLightfv(GL_LIGHT0, GL_AMBIENT, g.lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, g.lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, g.lightSpecular);
    glLightfv(GL_LIGHT0, GL_POSITION, g.lightPosition);
    glEnable(GL_LIGHT0);

    // setup a texture
    glGenTextures(1, &g.tex2);

    int w = img[0].width;
    int h = img[0].height;

    glBindTexture(GL_TEXTURE_2D, g.tex2);

    glTexParameteri(
        GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(
        GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexImage2D(
        GL_TEXTURE_2D, 0, 3, w, h, 0,
        GL_RGB, GL_UNSIGNED_BYTE, img[0].data);
    
    // Make a copy of Earth's pixels for highlighting
    int totalPixels = w * h * 3;

    if (highlightData == NULL) {
        highlightData = new unsigned char[totalPixels];
    }

    for (int i = 0; i < totalPixels; i++) {
        highlightData[i] = img[0].data[i];
    }

    glBindTexture(GL_TEXTURE_2D, 0);

    glGenTextures(1, &g.tex3);

    w = img[1].width;
    h = img[1].height;

    glBindTexture(GL_TEXTURE_2D, g.tex3);

    glTexParameteri(
        GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(
        GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexImage2D(
        GL_TEXTURE_2D, 0, 3, w, h, 0,
        GL_RGB, GL_UNSIGNED_BYTE, img[1].data);

    glBindTexture(GL_TEXTURE_2D, 0);


    // Moon texture
    glGenTextures(1, &g.tex4);

    w = img[2].width;
    h = img[2].height;

    glBindTexture(GL_TEXTURE_2D, g.tex4);

    glTexParameteri(GL_TEXTURE_2D,
        GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D,
        GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, 3, w, h, 0,
        GL_RGB, GL_UNSIGNED_BYTE, img[2].data);

    glBindTexture(GL_TEXTURE_2D, 0);

}
int check_keys(XEvent *e)
{
    if (e->type != KeyPress)
        return 0;

    int key = XLookupKeysym(&e->xkey, 0);

    if (g.menuPage == 2) {
        if (key >= XK_1 && key <= XK_8) {
            g.selectedLocation = key - XK_0;
            g.menuPage = 1;
            g.lesson_num = 1;
            if (g.selectedLocation == 1) {
                highlightNorthAmerica();
            }
            else if (g.selectedLocation == 2) {
                highlightSouthAmerica();
            }
            else if (g.selectedLocation == 3) {
                highlightEurope();
            }
            else if (g.selectedLocation == 4) {
                highlightAsia();
            }
            else if (g.selectedLocation == 5) {
                highlightAfrica();
            }
            else if (g.selectedLocation == 6) {
                highlightAustralia();
            }
            else if (g.selectedLocation == 7) {
                highlightAntarctica();
            }
            else {
                for (int i = 0; i < img[0].width*img[0].height*3; i++)
                    highlightData[i] = img[0].data[i];
            }
    
            // Update the existing Earth texture
            glBindTexture(GL_TEXTURE_2D, g.tex2);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
                img[0].width, img[0].height,
                GL_RGB, GL_UNSIGNED_BYTE, highlightData);
            glBindTexture(GL_TEXTURE_2D, 0);

            if (g.selectedLocation == 1) {
                     // Focus on North America
                g.targetRotation = 180.0f;
                g.movingToLocation = true;
            } 
            else if (g.selectedLocation == 2) {
                // Focus on South America
                g.targetRotation = 160.0f;
                g.movingToLocation = true;
            } 
            else if (g.selectedLocation == 3) {
            // Focus on Europe
                g.targetRotation = 80.0f;
                g.movingToLocation = true;
            }
            else if (g.selectedLocation == 4) {
            // Focus on Asia
                g.targetRotation = 360.0f;
                g.movingToLocation = true;
            }
            else if (g.selectedLocation == 5) {
            // Focus on Africa
                g.targetRotation = 75.0f;
                g.movingToLocation = true;
            }
            else if (g.selectedLocation == 6) {
            // Focus on Australia
                g.targetRotation = 300.0f;
                g.movingToLocation = true;
            }
            else if (g.selectedLocation == 7) {
            // Focus on Antarctica
                g.targetRotation = 180.0f;
            }
            else if (g.selectedLocation == 8) {
            // Focus on California
                    g.targetRotation = 200.0f;
                    g.movingToLocation = true;
            }
                    return 0;
        }
    }

    switch(key) {
        case XK_1:
            g.lesson_num = 1;
            init_opengl();
            break;
                case XK_g:
            // Open Geographic Menu
            g.menuPage = 2;
            break;

        case XK_h:
            // Open Help Menu
            g.menuPage = 3;
            break;

        case XK_m:
            // Return to Main Menu
            g.menuPage = 0;
            g.lesson_num = 0;
            break;

        case XK_b:
            // Return to Simulation
            g.menuPage = 1;
            g.lesson_num = 1;
            break;
        case XK_l:
            g.lightPosition[0] = rnd() * 200.0 - 100.0;
            g.lightPosition[1] = rnd() * 100.0 + 20.0;
            g.lightPosition[2] = rnd() * 300.0 - 20.0;
            glLightfv(GL_LIGHT0, GL_POSITION, g.lightPosition);
            break;

        case XK_equal:
        case XK_plus:
            g.selectedLocation = 0;
            g.movingToLocation = false;
            g.paused = false;
            g.speed += 0.2f;
            if (g.speed > 5.0f)
                g.speed = 5.0f;
            break;

        case XK_minus:
            g.selectedLocation = 0;
            g.movingToLocation = false;
            g.paused = false;
            g.speed -= 0.2f;
            if (g.speed < 0.2f)
                g.speed = 0.2f;
            break;

        case XK_space:
            if (g.selectedLocation == 8) {
                g.selectedLocation = 0;
                g.movingToLocation = false;
                g.paused = false;
            } else {
                g.paused = !g.paused;
            }
            break;

        case XK_Escape:
            return 1;
    }

    return 0;
}

void physics(void) {

    if (!g.paused)
        g.moonAngle -= 0.1f * g.speed;

    if (g.moonAngle < 0.0f)
        g.moonAngle += 360.0f;

}

void GalaxyBackground()
{
    glDisable(GL_LIGHTING);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 1, 0, 1, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glBindTexture(GL_TEXTURE_2D, g.tex3);
    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);
        glTexCoord2f(0, 0);
        glVertex2f(0, 0);

        glTexCoord2f(1, 0);
        glVertex2f(1, 0);

        glTexCoord2f(1, 1);
        glVertex2f(1, 1);

        glTexCoord2f(0, 1);
        glVertex2f(0, 1);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, 0);
}


void Earth(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);

    GalaxyBackground();

    glClear(GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(
        45.0f,
        (GLfloat)g.xres / g.yres,
        0.1f, 100.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glEnable(GL_LIGHTING);

    glTranslatef(0.0f, 0.0f, -10.0f);

    static int firsttime = 1;
    int i, j, i2, j2;

    static float verts[33][64][3];
    static float norms[33][64][3];
    static float tx[33][65][2];

    if (firsttime) {
        firsttime = 0;

        float circle[64][2];
        float angle = 0.0f;
        float inc = (PI * 2.0f) / 64.0f;

        for (i = 0; i < 64; i++) {
            circle[i][0] = cos(angle);
            circle[i][1] = sin(angle);
            angle += inc;
        }

        for (i = 0; i <= 32; i++) {
            float lat = PI * i / 32.0f;
            for (j = 0; j < 64; j++) {
                verts[i][j][0] =
                    circle[j][0] * sin(lat);

                verts[i][j][2] =
                    circle[j][1] * sin(lat);

                verts[i][j][1] = cos(lat);

                norms[i][j][0] = verts[i][j][0];
                norms[i][j][1] = verts[i][j][1];
                norms[i][j][2] = verts[i][j][2];

                tx[i][j][0] = 1.0- (float)j / 64.0f;
                tx[i][j][1] = (float)i / 32.0f;
            }
        }

        /* Duplicate texture coordinates for
           the longitude seam. */
        for (i = 0; i <= 32; i++) {
            tx[i][64][0] = 1.0f;
            tx[i][64][1] = (float)i / 32.0f;
        }
    }
        
glPushMatrix();

glScalef(1.8f, 1.8f, 1.8f);

// Tilt Earth to center California
if (g.selectedLocation == 1 || g.selectedLocation == 8) {
    glRotatef(29.0f, 1.0f, 0.0f, 0.0f);
}
else if (g.selectedLocation == 3) {
    glRotatef(35.0f, 1.0f, 0.0f, 0.0f);
}
else if (g.selectedLocation == 4) {
    // Tilt Earth toward Asia
    glRotatef(35.0f, 1.0f, 0.0f, 0.0f);
}
else if (g.selectedLocation == 6) {
    // Tilt Earth to bring Australia upward
    glRotatef(-25.0f, 1.0f, 0.0f, 0.0f);
}
else if (g.selectedLocation == 7) {
    // Bring Antarctica into view
    glRotatef(-85.0f, 1.0f, 0.0f, 0.0f);
}

// Rotate Earth horizontally
glRotatef(g.rtri, 0.0f, 1.0f, 0.0f);


    // glEnable(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, g.tex2);
    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);

    for (i = 0; i < 32; i++) {
        for (j = 0; j < 64; j++) {
            i2 = i + 1;
            j2 = (j + 1) % 64;

            glNormal3fv(norms[i][j]);
            glTexCoord2fv(tx[i][j]);
            glVertex3fv(verts[i][j]);

            glNormal3fv(norms[i2][j]);
            glTexCoord2fv(tx[i2][j]);
            glVertex3fv(verts[i2][j]);

            glNormal3fv(norms[i2][j2]);
            glTexCoord2f(1.0 - (float)(j + 1) / 64.0f, tx[i2][j2][1]);
            glVertex3fv(verts[i2][j2]);

            glNormal3fv(norms[i][j2]);
            glTexCoord2f(1.0 - (float)(j + 1) / 64.0f, tx[i][j2][1]);
            glVertex3fv(verts[i][j2]);
        }
    }

    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);


    // California location marker
    if (g.selectedLocation == 8) {
        float latitude = 36.0f * PI / 180.0f;
        float longitude =  -61.0f * PI / 180.0f;

        float x = 1.02f * cos(latitude) * cos(longitude);
        float y = 1.02f * sin(latitude);
        float z = 1.02f * cos(latitude) * sin(longitude);

        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_LIGHTING);

        glPointSize(9.0f);
        glColor3f(1.0f, 1.0f, 0.0f);

        glBegin(GL_POINTS);
            glVertex3f(x, y, z);
        glEnd();

        glPopAttrib();
    }

    // Atmospheric glow around Earth
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    glDepthMask(GL_FALSE);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    GLUquadric *atmosphere = gluNewQuadric();
    
    // Draw outer atmosphere first, then work inward
    for (int layer = 12; layer >= 1; layer--) {

        float t = (float)(layer - 1) / 11.0f;

        // Radius: 1.025 near Earth to 1.175 outside
        float radius = 1.025f + t * 0.15f;

        // White near Earth, dark blue toward space
        float red   = 0.98f - t * 0.68f;
        float green = 0.99f - t * 0.64f;
        float blue  = 1.0f;

        // Gradually reduce transparency outward
        float alpha = 0.09f * (1.0f - t) * (1.0f - t);

        glColor4f(red, green, blue, alpha);
        gluSphere(atmosphere, radius, 64, 32);
    }
    
    gluDeleteQuadric(atmosphere);

    glPopAttrib();
    glPopMatrix();
    
if (!g.paused) {
    if (g.movingToLocation) {
        float difference = g.targetRotation - g.rtri;

        // Find the shortest direction to rotate
        if (difference > 180.0f)
            difference -= 360.0f;
        if (difference < -180.0f)
            difference += 360.0f;

        float step = 0.5f * g.speed;

        if (fabs(difference) <= step) {
            g.rtri = g.targetRotation;
            g.movingToLocation = false;
        } else if (difference > 0.0f) {
            g.rtri += step;
        } else {
            g.rtri -= step;
        }
     }
    else if (g.selectedLocation != 8 &&
            g.selectedLocation != 1 &&
            g.selectedLocation != 2 &&
            g.selectedLocation != 3 &&
            g.selectedLocation != 4 &&
            g.selectedLocation != 5 &&
            g.selectedLocation != 6 &&
            g.selectedLocation != 7 &&
            g.selectedLocation != 8) {
    g.rtri += 0.2f * g.speed;
        }
    }

// Keep the rotation angle between 0 and 360
if (g.rtri >= 360.0f)
    g.rtri -= 360.0f;
if (g.rtri < 0.0f)
    g.rtri += 360.0f;

}

void Moon(void)
{
    const int slices = 64;
    const int stacks = 32;
    const float radius = 0.35f;
    
    glLoadIdentity();
    glPushMatrix();

    // Place Moon to the right of Earth
    // Earth is at (0, 0, -6)
    float orbitRadius = 2.5f;
    float angle = g.moonAngle * PI / 180.0f;

    float moonX = orbitRadius * cos(angle);
    float moonY = 0.0f;
    float moonZ = orbitRadius * sin(angle);
    glTranslatef(moonX, moonY, moonZ - 10.0f);
    

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, g.tex4);
    glColor3f(1.0f, 1.0f, 1.0f);

    for (int i = 0; i < stacks; i++) {
        float lat1 = PI * i / stacks;
        float lat2 = PI * (i + 1) / stacks;

        glBegin(GL_QUADS);

        for (int j = 0; j < slices; j++) {
            float lon1 = 2.0f * PI * j / slices;
            float lon2 = 2.0f * PI * (j + 1) / slices;

            float x, y, z;

            x = sin(lat1) * cos(lon1);
            y = cos(lat1);
            z = sin(lat1) * sin(lon1);
            glNormal3f(x, y, z);
            glTexCoord2f((float)j / slices,
                         (float)i / stacks);
            glVertex3f(radius*x, radius*y, radius*z);

            x = sin(lat2) * cos(lon1);
            y = cos(lat2);
            z = sin(lat2) * sin(lon1);
            glNormal3f(x, y, z);
            glTexCoord2f((float)j / slices,
                         (float)(i+1) / stacks);
            glVertex3f(radius*x, radius*y, radius*z);

            x = sin(lat2) * cos(lon2);
            y = cos(lat2);
            z = sin(lat2) * sin(lon2);
            glNormal3f(x, y, z);
            glTexCoord2f((float)(j+1) / slices,
                         (float)(i+1) / stacks);
            glVertex3f(radius*x, radius*y, radius*z);

            x = sin(lat1) * cos(lon2);
            y = cos(lat1);
            z = sin(lat1) * sin(lon2);
            glNormal3f(x, y, z);
            glTexCoord2f((float)(j+1) / slices,
                         (float)i / stacks);
            glVertex3f(radius*x, radius*y, radius*z);
        }

        glEnd();
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glPopMatrix();
}


void drawOrbit()
{
    float radius = 2.5f;

    glPushMatrix();

    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);

    glColor3f(1.0f, 0.85f, 0.2f);

    glBegin(GL_LINE_LOOP);

    for (int i = 0; i < 360; i++) {
        float angle = i * PI / 180.0f;

        float x = radius * cos(angle);
        float z = radius * sin(angle);

        glVertex3f(x, 0.0f, z - 10.0f);
    }

    glEnd();

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);

    glPopMatrix();
}


void render(void)
{
    Rect r;

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(
        45.0f,
        (GLfloat)g.xres/(GLfloat)g.yres,
        0.1f, 100.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    gluLookAt(0,5,10, 0,0,0, 0,1,0);

    //Enable this so material colors are the same
    //as vertex colors.
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);

    switch (g.lesson_num) {
        case 0:
            break;
        case 1:
            Earth();
            Moon();
            break;
    }


    //Set 2D mode (no perspective)
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, g.xres, 0, g.yres, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glDisable(GL_LIGHTING);
    
    
r.bot = g.yres - 15;
r.left = 10;
r.center = 0;

int fsz = 10;
int y = 16;

if (g.menuPage == 0) {
    ggprint(&r, fsz, y, 0x0070caff, "EARTH AND MOON EXPLORER");
    ggprint(&r, fsz, y, 0x00ffdd55, "1 - Start Simulation");
    ggprint(&r, fsz, y, 0x00ffdd55, "G - Geographic Locations");
    ggprint(&r, fsz, y, 0x00ffdd55, "H - Help / Controls");
    ggprint(&r, fsz, y, 0x00ffdd55, "ESC - Exit");
}
else if (g.menuPage == 1) {
    ggprint(&r, fsz, y, 0x0070caff, "EARTH AND MOON");
    ggprint(&r, fsz, y, 0x00ffdd55, "G - Geographic Menu");
    ggprint(&r, fsz, y, 0x00ffdd55, "H - Help");
    ggprint(&r, fsz, y, 0x00ffdd55, "M - Main Menu");
}
else if (g.menuPage == 2) {
    ggprint(&r, fsz, y, 0x0070caff, "GEOGRAPHIC LOCATIONS");
    ggprint(&r, fsz, y, 0x00ffdd55, "1 - North America");
    ggprint(&r, fsz, y, 0x00ffdd55, "2 - South America");
    ggprint(&r, fsz, y, 0x00ffdd55, "3 - Europe");
    ggprint(&r, fsz, y, 0x00ffdd55, "4 - Asia");
    ggprint(&r, fsz, y, 0x00ffdd55, "5 - Africa");
    ggprint(&r, fsz, y, 0x00ffdd55, "6 - Australia");
    ggprint(&r, fsz, y, 0x00ffdd55, "7 - Antarctica");
    ggprint(&r, fsz, y, 0x00ffdd55, "8 - California");
    ggprint(&r, fsz, y, 0x00ffffff, "B - Back to Simulation");
}
else if (g.menuPage == 3) {
    ggprint(&r, fsz, y, 0x0070caff, "HELP / CONTROLS");
    ggprint(&r, fsz, y, 0x00ffdd55, "1 - Start Simulation");
    ggprint(&r, fsz, y, 0x00ffdd55, "L - Change Light");
    ggprint(&r, fsz, y, 0x00ffdd55, "+ / - - Change Speed");
    ggprint(&r, fsz, y, 0x00ffdd55, "SPACE - Pause / Resume");
    ggprint(&r, fsz, y, 0x00ffdd55, "B - Back to Simulation");
    ggprint(&r, fsz, y, 0x00ffdd55, "M - Main Menu");
}

       }

