#include <stdlib.h>

#include <android/log.h>

#include "com_thedotbot_screens_ScreenReader.h"
#include "screenreader.h"

#define NUM_PARSERS 2

jobject (*parsers[NUM_PARSERS])(JNIEnv *env, screenshot_t *img) = {
    &try_dots_grid_screen,
    &new_unknown_screen
};


JNIEXPORT jobject JNICALL Java_com_thedotbot_screens_ScreenReader_readScreen(
            JNIEnv *env, jclass obj, jobject file) {

    /* Get the C string filename from the Java file. */
    jclass File = (*env)->GetObjectClass(env, file);
    jmethodID getPath = (*env)->GetMethodID(env, File, "getPath", "()Ljava/lang/String;");
    jobject javaPath = (*env)->CallObjectMethod(env, file, getPath);
    const char *path = (*env)->GetStringUTFChars(env, javaPath, NULL);

    screenshot_t img;
    open_screenshot(path, &img);

    jobject screen = NULL;
    int i;
    for (i = 0; i < NUM_PARSERS && screen == NULL; i++) {
        screen = (*parsers[i])(env, &img);
    }

    close_screenshot(&img);
    return screen;
}

jobject try_dots_grid_screen(JNIEnv *env, screenshot_t *img) {
    return NULL;
}

jobject new_dots_grid_screen(JNIEnv *env, color_t board[36],
            coord_t coords[36], coord_t *menu, coord_t *timeFreeze,
            coord_t *expander) {

    jobject DotColor = (*env)->FindClass(env, "com/thedotbot/screens/DotColor");
    jobject DotColorArray = (*env)->FindClass(env, "[com/thedotbot/screens/DotColor");
    jobject javaBoard = (*env)->NewObjectArray(env, (jsize)6, DotColorArray, NULL);

    jclass Point = (*env)->FindClass(env, "android/graphics/Point");
    jclass PointArray = (*env)->FindClass(env, "[android/graphics/Point");
    jobject javaCoords = (*env)->NewObjectArray(env, (jsize)6, PointArray, NULL);

    jmethodID initPoint = (*env)->GetMethodID(env, Point, "<init>", "(II)V");

    int r, c;
    for (r = 0; r < 6; r++) {
        jobjectArray colorRow = (*env)->NewObjectArray(env, (jsize)6, DotColor, NULL);
        jobjectArray coordRow = (*env)->NewObjectArray(env, (jsize)6, Point, NULL);
        for (c = 0; c < 6; c++) {

            jobject color = get_dot_color(env, board[6 * c + r]);
            (*env)->SetObjectArrayElement(env, colorRow, c, color);

            coord_t coord = coords[6 * c + r];
            jobject point = (*env)->NewObject(env, point, initPoint, coord.x, coord.y);
            (*env)->SetObjectArrayElement(env, coordRow, c, point);
        }
        (*env)->SetObjectArrayElement(env, javaBoard, r, colorRow);
        (*env)->SetObjectArrayElement(env, javaCoords, r, coordRow);
    }

    jobject javaMenu = (*env)->NewObject(env, Point, initPoint, menu->x, menu->y);
    jobject javaTimeFreeze  = (*env)->NewObject(env, Point, initPoint, timeFreeze->x, timeFreeze->y);
    jobject javaExpander = (*env)->NewObject(env, Point, initPoint, expander->x, expander->y);

    jclass DotsGridScreen = (*env)->FindClass(env, "com/thedotbot/screens/DotsGridScreen");
    jmethodID initDotsGridScreen = (*env)->GetMethodID(env, DotsGridScreen, "<init>",
            "(L[[com/thedotbot/screens/DotColor;L[[android/graphics/Point;Landroid/graphics/Point;Landroid/graphics/Point;Landroid/graphics/Point;)V");

    return (*env)->NewObject(env, DotsGridScreen, initDotsGridScreen,
                javaBoard, javaCoords, javaMenu, javaTimeFreeze,
                javaExpander);
}


jobject get_dot_color(JNIEnv *env, color_t color) {
    jobject DotColor = (*env)->FindClass(env, "com/thedotbot/screens/DotColor");
    const char *name;
    switch (color) {
        case RED:
            name = "RED";
            break;
        case YELLOW:
            name = "YELLOW";
            break;
        case GREEN:
            name = "GREEN";
            break;
        case BLUE:
            name = "BLUE";
            break;
        case PURPLE:
            name ="PURPLE";
            break;
    }
    jfieldID colorField = (*env)->GetStaticFieldID(DotColor, DotColor, name, "Lcom/thedotbot/screens/DotColor;");
    return (*env)->GetStaticObjectField(env, DotColor, colorField);
}

jobject new_unknown_screen(JNIEnv *env, screenshot_t *img) {
    jobject UnknownScreen = (*env)->FindClass(env, "com/thedotbot/screens/UnknownScreen");
    jmethodID initUnknownScreen = (*env)->GetMethodID(env, UnknownScreen, "<init>", "()V");
    return (*env)->NewObject(env, UnknownScreen, initUnknownScreen);
}
