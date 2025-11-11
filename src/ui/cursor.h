#ifndef __EDITOR_CURSOR_H
#define __EDITOR_CURSOR_H

typedef struct Window Window;

void editorMoveCursorLeft(Window *W);
void editorMoveCursorRight(Window *W);
void editorMoveCursorUp(Window *W);
void editorMoveCursorDown(Window *W);

void editorMoveCursorLineEnd(Window *W);
void editorMoveCursorLineStart(Window *W);

void editorMoveCursorPageUp(Window *W);
void editorMoveCursorPageDown(Window *W);

void editorCenterCursor(Window *W);

void editorScrollUp(Window *W);
void editorScrollDown(Window *W);

void editorMoveCursorNextWord(Window *W);
void editorMoveCursorPreviousWord(Window *W);

void editorCursorReset(Window *W);

#endif /* __EDITOR_CURSOR_H */
