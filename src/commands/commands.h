#ifndef __EDITOR_COMMANDS_H
#define __EDITOR_COMMANDS_H

typedef struct TextBuffer TextBuffer;
typedef struct Window Window;

void editorShell(int fd);

int editorOpen(Window *W, const char *filename);
int editorSave(TextBuffer *buf);
int editorSaveAs(TextBuffer *buf, int fd);
void editorQuit(TextBuffer *buf, int fd);
void editorFind(Window *W, int fd);
void editorToggleLinenum(void);

void command_handler_open(int fd, int argc, char **argv);
void command_handler_save(int fd, int argc, char **argv);
void command_handler_quit(int fd, int argc, char **argv);
void command_handler_find(int fd, int argc, char **argv);
void command_handler_line(int fd, int argc, char **argv);

#endif /* __EDITOR_COMMANDS_H */
