/*
    Edifix Editor, is a very simple Editor intented to run on microcontrollers.
    
    Written by Michael Neuroth for the PicoHomeComputer
    
    MIT License    
*/

#ifndef _EDIFIXEDITOR_H
#define _EDIFIXEDITOR_H

const int ARROW_LEFT        = 140;
const int ARROW_RIGHT       = 141;
const int ARROW_UP          = 142;
const int ARROW_DOWN        = 143;
const int END               = 147;
    
class EdifixEditorEvent
{
public:           
    enum EventType { KEY, META_KEY, MOUSE, QUIT, EXIT, UNDEFINED } eventType;

    EdifixEditorEvent();
    EdifixEditorEvent(EventType evType);
    EdifixEditorEvent(char ch);
    EdifixEditorEvent(int metaCh);
    EdifixEditorEvent(int x, int y);
    
    EdifixEditorEvent(const EdifixEditorEvent & other);
    EdifixEditorEvent & operator=(const EdifixEditorEvent & other);

    bool IsUndefined() const { return eventType == UNDEFINED; }
    bool IsExit() const { return eventType == EXIT; } 
    bool IsQuit() const { return eventType == QUIT; } 
    bool IsMetaKey() const { return eventType == META_KEY; }
    bool IsKey() const { return eventType == KEY; }
    bool IsMouse() const { return eventType == MOUSE; }

    int GetMetaKey() const { return data.metaKey; }
    
    union {
        struct {
            int x;
            int y;
        } pos;
        char key;
        int metaKey;
    } data;

private:    
    void Copy(const EdifixEditorEvent & other);
};

// ************************************************************************

// A simple, stl conform string class (used by the EdifixEditor)
class SimpleString
{
    const int INITIAL_BUFFER_SIZE = 256;
    const int DELTA = 32;

public:
    SimpleString();
    SimpleString(const char * text);
    ~SimpleString();
    SimpleString(const SimpleString & other);

    SimpleString & operator=(const SimpleString & other);
    bool operator==(const SimpleString & other) const;

    void clear();
    void push_back(char ch);
    void push_back(char * line);

    const char * c_str() const;
    int size() const;

private:
    void Init(int size);
    void DeInit();
    void Copy(const char * text, int len);

    char * m_pBuffer;
    int    m_iBufferSize;    
    int    m_iNextIndexToAddToBuffer;
};
    
// ************************************************************************

// Terminal interface for the edifix editor,
// the edifix editor needs an implementation of this interface to run
class EdifixEditorTerminalInterface
{    
public:
    virtual int GetScreenWidth() const = 0;
    virtual int GetScreenHeight() const = 0;

    virtual void ClearScreen() = 0;
    virtual void SaveScreen() = 0;
    virtual void RestoreScreen() = 0;

    virtual void Write(int col, int row, const char * text, bool eraseToEndOfLine = false) = 0;
    virtual void SetCursor(int col, int row) = 0;

    virtual void SetTerminalModus(bool value) = 0;

    // poll for the events
    virtual EdifixEditorEvent GetNextEvent() = 0;
};

// ************************************************************************

// A very simple editor which can run on microcontrollers.
// Edifix <--> Idefix
class EdifixEditor
{
    const int DELTA = 10;
    
public:    
    EdifixEditor(const char * text, EdifixEditorTerminalInterface * pTerminalInterface);
    //maybe: EdifixEditor(File * file, EdifixEditorTerminalInterface * pTerminalInterface);
    ~EdifixEditor();

    // return value: false for quit (do not use the modified text), true for ok (use modified text)
    bool Run();

    SimpleString GetText() const;

    int GetLineCount() const;
    int GetTotalLength() const;

private:
    void ProcessEvent(const EdifixEditorEvent & event);

    void Init();
    void DeInit();
    void FillBuffer(const char * text);
    void AddLine(const char * line);

    void DrawTextBlock(int iScreenStartRow, int iNumberOfRows, int iBufferStartPos);

    bool CursorLeft();
    bool CursorRight();
    bool CursorUp();
    bool CursorDown();

    void CursorToEndOfText();

    
    EdifixEditorTerminalInterface * m_pTerminalInterface;     // not an owner !

    char * *  m_pBuffer;                  // array of strings
    int       m_iCurrentBufferSize;       // capacity of array of strings
    int       m_iNextIndexToAddToBuffer;  // current index in array of strings to add line into the buffer

    int       m_iMaxCols;
    int       m_iMaxRows;
    int       m_iCurrentCursorX;
    int       m_iCurrentCursorY;
    int       m_iVirtualCursorXPos;
};

#endif
