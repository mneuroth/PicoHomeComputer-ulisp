
#include "libs/EdifixEditor.h"

class IOProcessorTerminalImpl : public EdifixEditorTerminalInterface
{
    const char CR = '\r';
    const char LF = '\n';
    
public:
    IOProcessorTerminalImpl(int cols, int rows);

    virtual int GetScreenWidth() const;
    virtual int GetScreenHeight() const;
    virtual void ClearScreen();
    virtual void SaveScreen();
    virtual void RestoreScreen();
    virtual void Write(int col, int row, const char * text, bool eraseToEndOfLine);
    virtual void SetCursor(int col, int row);
    virtual void SetInsertCursorModus(bool bIsInsert);

    virtual void SetTerminalModus(bool value);
    
    virtual EdifixEditorEvent GetNextEvent();
    
private:
    SimpleString ReadLineFromIOProcessorInTerminalMode();

    int m_iMaxCols;
    int m_iMaxRows;
    int m_iCurrentCursorX;
    int m_iCurrentCursorY;
    int m_iVirtualCursorXPos;
    bool m_bIsInTerminalModus;
    bool m_bIsVerboseTerminalModus;
};

IOProcessorTerminalImpl::IOProcessorTerminalImpl(int cols, int rows)
{
    m_iMaxCols = cols;
    m_iMaxRows = rows;
    m_iCurrentCursorX = 0;
    m_iCurrentCursorY = 0;
    m_iVirtualCursorXPos = 0;
    m_bIsInTerminalModus = false;
    m_bIsVerboseTerminalModus = false;
}

int IOProcessorTerminalImpl::GetScreenWidth() const
{
    return m_iMaxCols;
}

int IOProcessorTerminalImpl::GetScreenHeight() const
{
    return m_iMaxRows;    
}
  
void IOProcessorTerminalImpl::ClearScreen()
{
    pfstring(PSTR("\x01#CLR\x01"), pserial);
}

void IOProcessorTerminalImpl::SaveScreen()
{
    pfstring(PSTR("\x01#SAVESCREEN\x01"), pserial);
}

void IOProcessorTerminalImpl::RestoreScreen()
{
    pfstring(PSTR("\x01#RESTORESCREEN\x01"), pserial);
}

void IOProcessorTerminalImpl::Write(int col, int row, const char * text, bool eraseToEndOfLine)
{
    char buf[16+/*TODO MSC++ GetScreenWidth()*/255];
    if( m_bIsInTerminalModus )
    {
        sprintf(buf,"#V:%02d,%02d,%s%c",col,row,text,CR/*,LF*/);
    }
    else
    {
        sprintf(buf,"\x01#V:%02d,%02d,%s\x01",col,row,text);
    }
    pfstring(buf, pserial);
    if( eraseToEndOfLine )
    {
        int len = strlen(text);
        int fillLen = m_iMaxCols-col-len+1;
        char * emptyText = (char *)malloc(fillLen);
        memset(emptyText,' ',fillLen);
        emptyText[fillLen] = 0;
        if( m_bIsInTerminalModus )
        {
            sprintf(buf,"#V:%02d,%02d,%s%c",col+len,row,emptyText,CR/*,LF*/);
        }
        else
        {
            sprintf(buf,"\x01#V:%02d,%02d,%s\x01",col+len,row,emptyText);          
        }
        pfstring(buf, pserial);        
        free(emptyText);
    }
}

void IOProcessorTerminalImpl::SetCursor(int col, int row)
{
    m_iCurrentCursorX = col;
    m_iCurrentCursorY = row;
    m_iVirtualCursorXPos = col;
    
    char buf[16];
    if( m_bIsInTerminalModus )
    {
        sprintf(buf,"#V:%02d,%02d,%c",col,row,CR/*,LF*/);
    }
    else
    {
        sprintf(buf,"\x01#V:%02d,%02d,\x01",col,row);
    }
    pfstring(buf, pserial);  
// TODO --> hier debug ausgaben hinzufuegen
}

void IOProcessorTerminalImpl::SetInsertCursorModus(bool bIsInsert)
{
    // TODO
}

// reads commands/events for key and mouse from IO Processor in terminal mode
SimpleString IOProcessorTerminalImpl::ReadLineFromIOProcessorInTerminalMode()
{
    SimpleString s;
    
    bool bFinish = false;
    do {
        while (!pLispSerial->available());
        char data = pLispSerial->read();
        pLispSerialMonitor->write(data);
        
        if( data == 10 || data == 13 )
        {
            bFinish = true;           
        }
        else
        {
            s.push_back(data);
        }
    } while( !bFinish );

    return s;
}

void IOProcessorTerminalImpl::SetTerminalModus(bool value)
{
// TODO --> hier debug ausgaben hinzufuegen
    if( !m_bIsInTerminalModus && value )
    {
        pfstring(PSTR("\x01#TERMINAL\x01"), pserial);      
        m_bIsInTerminalModus = true;
    }
    else if( !m_bIsInTerminalModus && !value )
    {
        pfstring(PSTR("\x01#SHELL\x01"), pserial);      
        m_bIsInTerminalModus = false;
    }
    else if( m_bIsInTerminalModus && value )
    {
        pfstring(PSTR("#TERMINAL"), pserial);      
        if( m_bIsVerboseTerminalModus )
        {
            // wait for resoponse: "#OK #TERMINAL"
            SimpleString s = ReadLineFromIOProcessorInTerminalMode();
            bool ok = strcmp(s.c_str(), "#OK #TERMINAL") == 0;
        }
        m_bIsInTerminalModus = true;
    }
    else if( m_bIsInTerminalModus && !value )
    {
        pfstring(PSTR("#SHELL"), pserial);      
        pfl(pserial);
        if( m_bIsVerboseTerminalModus )
        {
            // wait for resoponse: "#OK #SHELL"
            SimpleString s = ReadLineFromIOProcessorInTerminalMode();
            bool ok = strcmp(s.c_str(), "#OK #SHELL") == 0;
        }
        m_bIsInTerminalModus = false;
    }
}

EdifixEditorEvent IOProcessorTerminalImpl::GetNextEvent()
{
    SimpleString s = ReadLineFromIOProcessorInTerminalMode();
    
    const char * cmd = s.c_str();

    if( strcmp(cmd, "@K:#140") == 0 )  // Left Arrow
    {
        return EdifixEditorEvent(ARROW_LEFT);
    }
    if( strcmp(cmd, "@K:#141") == 0 )  // Right Arrow
    {
        return EdifixEditorEvent(ARROW_RIGHT);
    }
    if( strcmp(cmd, "@K:#142") == 0 )  // Up Arrow
    {
        return EdifixEditorEvent(ARROW_UP);
    }
    if( strcmp(cmd, "@K:#143") == 0 )  // Down Arrow
    {
        return EdifixEditorEvent(ARROW_DOWN);
    }
        
    if( strcmp(s.c_str(), "@K:#146") == 0 )  // Home key
    {
        return EdifixEditorEvent(HOME);
    }
    if( strcmp(s.c_str(), "@K:#147") == 0 )  // End key
    {
        return EdifixEditorEvent(END);
    }
    if( strcmp(s.c_str(), "@K:#148") == 0 )  // Insert key
    {
        return EdifixEditorEvent(INSERT);
    }
    if( strcmp(s.c_str(), "@K:#149") == 0 )  // Delete key
    {
        return EdifixEditorEvent(DELETE);
    }
    if( strcmp(s.c_str(), "@K:#154") == 0 )  // F1 key
    {
        return EdifixEditorEvent(EdifixEditorEvent::EXIT);
    }
    
    if( strncmp(s.c_str(), "@K:", 3) == 0 )  // normal character
    {
        char ch = s.c_str()[3];
        if( ch == 10 || ch == 13 )
        {
            return EdifixEditorEvent(ENTER);
        }
        if( ch == 8 )
        {
            return EdifixEditorEvent(BACKSPACE);
        }
        return EdifixEditorEvent(ch);
    }

    return EdifixEditorEvent();
}

