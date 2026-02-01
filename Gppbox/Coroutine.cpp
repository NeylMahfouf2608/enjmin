#pragma once
//never used in the project ; but think the tech is cool
struct ScriptState {
	int line = 0;
	float timer = 0.0f;
	bool finished = false;
};

#define SCRIPT_BEGIN(state) \
    if (state.finished) return; \
    switch(state.line) { \
    case 0:;


#define SCRIPT_END(state) \
    } \
    state.finished = true; \
    state.line = 0;


#define WAIT(state, time, dt) \
    do { \
        state.line = __LINE__; \
        state.timer = (float)time; \
        case __LINE__: \
        state.timer -= (float)dt; \
        if (state.timer > 0) return; \
    } while(0)

#define WAIT_UNTIL(state, condition) \
    do { \
        state.line = __LINE__; \
        case __LINE__: \
        if (!(condition)) return; \
    } while(0)