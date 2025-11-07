#ifndef __EDITOR_MODES_H
#define __EDITOR_MODES_H

#define NORMAL_MODE 0
#define INSERT_MODE 1

void editorSetInsertMode(void);
void editorIMProcessKeypress(int fd);

void editorSetNormalMode(void);
void editorNMProcessKeypress(int fd);

#endif /* __EDITOR_MODES_H */
