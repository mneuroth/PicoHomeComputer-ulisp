/*
    Edifix Editor, is a very simple Editor intented to run on microcontrollers.
    
    Written by Michael Neuroth for the PicoHomeComputer
    
    MIT License    
*/

#ifndef _EDIFIXEDITOR_H
#define _EDIFIXEDITOR_H

struct EdifixEditorEvent
{
    enum EventType { KEY, MOUSE, QUIT, EXIT } eventType;
    
    union {
        struct {
            int x;
            int y;
        } pos;
        char key;
    } data;
};

void test()
{
    EdifixEditorEvent b;
    b.eventType == EdifixEditorEvent::EventType::KEY;
    b.data.pos.x;
    b.data.key;
}

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
    virtual int GetNextEvent() = 0;
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
    void ProcessEvent(int code, bool * pOk);

    void Init();
    void DeInit();
    void FillBuffer(const char * text);
    void AddLine(const char * line);

    void DrawTextBlock(int iScreenStartRow, int iNumberOfRows, int iBufferStartPos);
    
    EdifixEditorTerminalInterface * m_pTerminalInterface;     // not an owner !

    char * *  m_pBuffer;                  // array of strings
    int       m_iCurrentBufferSize;       // capacity of array of strings
    int       m_iNextIndexToAddToBuffer;  // current index in array of strings to add line into the buffer
};

#endif
