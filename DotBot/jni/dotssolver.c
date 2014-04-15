#include <stdlib.h>
#include "com_thedotbot_ai_DotsSolver.h"

JNIEXPORT jobject JNICALL Java_com_thedotbot_ai_DotsSolver_chooseMove(
            JNIEnv *env, jclass obj, jobjectArray board,
            jint turnsRemaining, jboolean allowTimeFreeze,
            jboolean allowShrinkers, jboolean allowExpander) {
    return NULL;
}

JNIEXPORT jdouble JNICALL Java_com_thedotbot_ai_DotsSolver_scoreAbove(
            JNIEnv *env, jclass obj, jint turn, jint score,
            jint targetScore) {
    return 0;
}
