#ifndef __EDITOR_EVENT_H
#define __EDITOR_EVENT_H

void editorProcessKeypress(int fd);

void updateWindowSize(void);
void handleSigWinCh(int);

void editorSetStatusMessage(const char *fmt, ...);
void editorFatalError(const char *fmt, ...);
void editorPrintFatalError(void);

#endif /* __EDITOR_EVENT_H */
