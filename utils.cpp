// #include<math.h>
#define internal static 
#define local_persist static 
#define global_variable static

#if PRODUCTION
#define assert(expression) ;
#define assert_msg(expression, message) ;
#else
#define assert(expression) if (!(expression)) {*(int *) 0 = 0;}
#define assert_msg(expression, message) if (!(expression)) {printf("\nASSERT: %s\n", message);*(int *) 0 = 0;}
#endif

#define LENGTH(array) sizeof(array) / sizeof(array[0])

void color_text(char *text, char* color) {
#if CONSOLE_COLORS
    printf("%s%s\033[0m", (char *)color, (char *)text);
#else
    printf("%s", (char *)text);
#endif
}

void color_text(char text, char* color) {
#if CONSOLE_COLORS
    printf("%s%c\033[0m", color, text);
#else
    printf("%c", text);
#endif
}

void color_text(char *text, u32 r, u32 g, u32 b) {
#if CONSOLE_COLORS
    printf("\033[38;2;%d;%d;%dm%s\033[0m", r, g, b, text);
#else
    printf("%s", text);
#endif
}

// https://en.wikipedia.org/wiki/ANSI_escape_code
// #define RED(text) color_text(text, (char *)"\033[38;5;196m")
#define RED(text) color_text(text, (char *)"\033[38;2;255;0;0m")
// #define GREEN(text) color_text(text, (char *)"\033[38;5;82m")
#define GREEN(text) color_text(text, (char *)"\033[38;2;0;255;0m")
// #define YELLOW_GREEN(text) color_text(text, (char *)"\033[38;5;190m")
#define YELLOW_GREEN(text) color_text(text, (char *)"\033[38;2;166;198;50m")
#define YELLOW(text) color_text(text, (char *)"\033[38;2;235;235;50m")

#define PI 3.14159265358979323846

u32 generate_angle(u32 n, u32 i) {
    f64 theta = PI;
    f64 PI2 = 2.0 * PI;
    f64 alpha;
    u32 angle;

    alpha = (theta + ((f64)i * PI2)) / ((u64) n);
    angle = ((u32) ((alpha / PI2) * 360.0)) % 360;

    return angle;
}

struct Color_rgb {
    u32 r;
    u32 g;
    u32 b;
};

void HSVtoRGB(f32 H, f32 S, f32 V, Color_rgb *result) {
    // https://www.codespeedy.com/hsv-to-rgb-in-cpp/
    // https://es.wikipedia.org/wiki/Modelo_de_color_HSV
    if(H > 360 || H < 0 || S > 100 || S < 0 || V > 100 || V < 0) {
        // cout<<"The givem HSV values are not in valid range"<<endl;
        return;
    }

    f32 s = S / 100;
    f32 v = V / 100;
    f32 C = s * v;
    f32 X = C * (1 - fabs(fmod(H / 60.0, 2.0) - 1));
    f32 m = v - C;
    f32 r, g, b;
    
    if (H >= 0 && H < 60) {
        r = C;
        g = X;
        b = 0;
    } else if (H >= 60 && H < 120) {
        r = X;
        g = C;
        b = 0;
    } else if (H >= 120 && H < 180) {
        r = 0;
        g = C;
        b = X;
    } else if (H >= 180 && H < 240) {
        r = 0;
        g = X;
        b = C;
    } else if (H >= 240 && H < 300) {
        r = X;
        g = 0;
        b = C;
    } else {
        r = C;
        g = 0;
        b = X;
    }

    result->r = (u32)((r + m) * 255);
    result->g = (u32)((g + m) * 255);
    result->b = (u32)((b + m) * 255);
}
