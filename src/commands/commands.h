#ifndef __EDITOR_COMMANDS_H
#define __EDITOR_COMMANDS_H

typedef struct TextBuffer TextBuffer;
typedef struct Window Window;

int editorOpen(Window *W, char *filename);
int editorSave(TextBuffer *buf);
int editorSaveAs(TextBuffer *buf);
void editorQuit(TextBuffer *buf, int fd);

void editorFind(Window *W, int fd);

void editorToggleLinenum(void);
void editorShell(void);

#endif /* __EDITOR_COMMANDS_H */
