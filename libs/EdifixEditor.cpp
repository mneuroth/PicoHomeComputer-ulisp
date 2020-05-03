/*
    Edifix Editor, is a very simple editor intended to run on microcontrollers.
    
    Written by Michael Neuroth for the PicoHomeComputer
    
    MIT License    
*/

#include "EdifixEditor.h"

#include <stdlib.h>
#include <string.h>

//#include <iostream>

#if !defined(ARDUINO_ARCH_PIC32)  // or __PIC32 or PIC32 or __PIC32MX__
#include <algorithm>
#define _min std::min
#else
#define _min min
#endif

// for debugging only
typedef void (*pfun_t)(char);
void pserial (char c);
void pfstring (const char *s, pfun_t pfun);
void pfl (pfun_t pfun);

void test()
{
    EdifixEditorEvent b('a');
    b.eventType == EdifixEditorEvent::EventType::KEY;
    b.data.pos.x;
    b.data.key;
}

// ************************************************************************

EdifixEditorEvent::EdifixEditorEvent()
{
    eventType = UNDEFINED;
    memset(&data, 0, sizeof(data));
}

EdifixEditorEvent::EdifixEditorEvent(char ch)
{
    eventType = KEY;
    data.key = ch;
}

EdifixEditorEvent::EdifixEditorEvent(int metaCh)
{
    eventType = META_KEY;
    data.metaKey = metaCh;
}

EdifixEditorEvent::EdifixEditorEvent(int x, int y)
{
    eventType = MOUSE;
    data.pos.x = x;
    data.pos.y = y;
}

EdifixEditorEvent::EdifixEditorEvent(EventType evType)
{
    eventType = evType;
    memset(&data, 0, sizeof(data));
}

EdifixEditorEvent::EdifixEditorEvent(const EdifixEditorEvent & other)
{
    Copy(other);
}

EdifixEditorEvent & EdifixEditorEvent::operator=(const EdifixEditorEvent & other)
{
    if( this != &other)
    {
        Copy(other);
    }
    return *this;
}

void EdifixEditorEvent::Copy(const EdifixEditorEvent & other)
{
    eventType = other.eventType;
    // ggf. memcpy
    memcpy(&data, &(other.data), sizeof(data));
}


// ************************************************************************

SimpleString::SimpleString()
{
    Init(INITIAL_BUFFER_SIZE);
}

SimpleString::SimpleString(const char * text)
{
    int len = strlen(text);
    Init(len);
    Copy(text, len);
}

SimpleString::~SimpleString()
{
    DeInit();
}

SimpleString::SimpleString(const SimpleString & other)
{
    int len = other.size();
    Init(len);
    Copy(other.m_pBuffer, len);
}

SimpleString & SimpleString::operator=(const SimpleString & other)
{
    if( this != &other )
    {
        DeInit();
        int len = other.size();
        Init(len);
        Copy(other.m_pBuffer, len);
    }
    return *this;
}

bool SimpleString::operator==(const SimpleString & other) const
{
    return strcmp(m_pBuffer, other.m_pBuffer) == 0;
}

void SimpleString::Copy(const char * text, int len)
{
    memcpy(m_pBuffer, text, len + 1);
    m_iNextIndexToAddToBuffer = len + 1;    
}

void SimpleString::clear()
{
    m_iNextIndexToAddToBuffer = 0;
    m_pBuffer[m_iNextIndexToAddToBuffer] = 0;
}

void SimpleString::push_back(char ch)
{
    if( m_iNextIndexToAddToBuffer+1 >= m_iBufferSize )
    {
        m_iBufferSize = m_iBufferSize + DELTA;
        m_pBuffer = (char *)realloc(m_pBuffer, m_iBufferSize * sizeof(char));
    }

    if( m_iNextIndexToAddToBuffer+1 < m_iBufferSize )
    {
        m_pBuffer[m_iNextIndexToAddToBuffer] = ch;
        ++m_iNextIndexToAddToBuffer;
        m_pBuffer[m_iNextIndexToAddToBuffer] = 0;
    }
}

void SimpleString::push_back(char * line)
{
    if( line )
    {
        int i = 0;
        while( line[i] != 0 )
        {
            push_back(line[i]);
            ++i;
        }
    }
}

const char * SimpleString::c_str() const
{
    return m_pBuffer;
}
    
int SimpleString::size() const
{
    return m_iNextIndexToAddToBuffer; // == strlen(m_pBuffer);
}

void SimpleString::Init(int size)
{
    m_iBufferSize = size+1;
    m_iNextIndexToAddToBuffer = 0;
    m_pBuffer = (char *)malloc(m_iBufferSize * sizeof(char));
    m_pBuffer[0] = 0;
}

void SimpleString::DeInit()
{
    free(m_pBuffer);
    m_pBuffer = 0;
    m_iBufferSize = 0;
    m_iNextIndexToAddToBuffer = 0;
}

// ************************************************************************

EdifixEditor::EdifixEditor(const char * text, EdifixEditorTerminalInterface * pTerminalInterface)
  : m_pTerminalInterface( pTerminalInterface ),
    m_bInsertModus( true )
{
    Init();
    
    FillBuffer(text);    
}

EdifixEditor::~EdifixEditor()
{
    DeInit();
}

SimpleString EdifixEditor::GetText() const
{
    SimpleString s;
    for(int i=0; i<m_iNextIndexToAddToBuffer; ++i)
    {
        s.push_back(m_pBuffer[i]);
        s.push_back('\n');
    }
    return s;
}

int EdifixEditor::GetLineCount() const
{
    return m_iNextIndexToAddToBuffer;
}

bool EdifixEditor::Run()
{    
    if( m_pTerminalInterface )
    {       
        m_pTerminalInterface->SaveScreen();        
        m_pTerminalInterface->ClearScreen();        
        m_pTerminalInterface->SetTerminalModus(true);
        m_pTerminalInterface->SetInsertCursorModus(m_bInsertModus);
        
        DrawTextBlock(0, m_pTerminalInterface->GetScreenHeight(), 0);
        m_pTerminalInterface->SetCursor(0, 0);
        
        // key and mouse event loop ...        
        EdifixEditorEvent event = m_pTerminalInterface->GetNextEvent();
        while( !event.IsExit() && !event.IsQuit() )
        {
            ProcessEvent(event);
            
            event = m_pTerminalInterface->GetNextEvent();
        }
        
        m_pTerminalInterface->SetTerminalModus(false);
        m_pTerminalInterface->RestoreScreen();
        
        return event.IsExit();
    }
    
    return true;
}

void EdifixEditor::ProcessEvent(const EdifixEditorEvent & event)
{
    if( event.IsMetaKey() )
    {
//std::cerr << "PROC meta " << event.GetMetaKey() << std::endl;    

// TODO --> text laenger als screen  behandeln

        switch( event.GetMetaKey() )
        {
            case ARROW_LEFT:
                CursorLeft();
                break;
            case ARROW_RIGHT:
                CursorRight();
                break;
            case ARROW_UP:
                CursorUp();
                break;
            case ARROW_DOWN:
                CursorDown();
                break;
// TODO Scroll up/down             
// TODO Page Up/Down            
// TODO Ctrl + Page Up/Down
// TODO Scroll left/right
// TODO Ctrl + HOME/END
// TODO Ctrl + Left/Right
// TODO Copy/Paste/Cut
// TODO Search/Replace
// TODO Goto Line            
// TODO Undo/Redo
// TODO Save/SaveAs
// TODO Load            
            case HOME:
                m_iCurrentCursorX = 0;
                UpdateCursor();
                break;
            case END:
                m_iCurrentCursorX = GetLengthOfLine(m_iCurrentCursorY);
                UpdateCursor();
                break;
            case DELETE:
                Delete();
                break;
            case BACKSPACE:
                if( m_iCurrentCursorX>=0 && m_iCurrentCursorY>=0 )
                {
                    CursorLeft();
                    Delete();
                }
                break;
            case INSERT:
                m_bInsertModus = !m_bInsertModus;
                if( m_pTerminalInterface )
                {
                    m_pTerminalInterface->SetInsertCursorModus(m_bInsertModus);
                }
                break;
            case ENTER:
                {
                    // copy rest of line into buffer
                    char buf[strlen(m_pBuffer[m_iCurrentCursorY]+m_iCurrentCursorX)+1];
                    strcpy(buf,m_pBuffer[m_iCurrentCursorY]+m_iCurrentCursorX);
                    // remove rest of line from current line (because it will be 'moved' into the next line
                    m_pBuffer[m_iCurrentCursorY][m_iCurrentCursorX] = 0;
                    // insert 'new' line 
                    ++m_iCurrentCursorY;
                    InsertLineAtCursorAndUpdateScreen(buf);
                    // we need the side effect of update screen in InsertLineAtCursorAndUpdateScreen()
                }
                break;
            default:
              break;
        }
    }    
    else if( event.IsKey() )
    {
//std::cerr << "PROC key " << event.GetKey() << std::endl;    
        if( m_bInsertModus )
        {
            InsertCharacterAtCursor( event.GetKey() );
        }
        else
        {
            if( m_iCurrentCursorX < GetLengthOfLine(m_iCurrentCursorY) )
            {
                m_pBuffer[m_iCurrentCursorY][m_iCurrentCursorX] = event.GetKey();
                m_iCurrentCursorX++;
                UpdateCurrentLine();
            }
            else
            {
                InsertCharacterAtCursor( event.GetKey() );
            }
        }
    }
    else if( event.IsExit() || event.IsQuit() )
    {
    }
}

void EdifixEditor::Delete()
{
    if( m_iCurrentCursorX == GetLengthOfLine(m_iCurrentCursorY) )
    {
        if( m_iCurrentCursorY+1 < GetLineCount() )
        {
            // merge current line and following line into current line
            AppendString(m_pBuffer[m_iCurrentCursorY+1]);
            // remove following line
            RemoveLine(m_iCurrentCursorY+1);                        
            // update screen
            UpdateScreen();
            UpdateCursor();
        }
    }
    else
    {
        DeleteCharacterAtCursor();
    }
}

void EdifixEditor::Init()
{
    m_iNextIndexToAddToBuffer = 0;
    m_iCurrentBufferSize = DELTA;
    m_pBuffer = (char * *)calloc(m_iCurrentBufferSize, sizeof(char *));
    /*
    // initialization not needed, because calloc initializes memory with 0
    for(int i=0; i<m_iCurrentBufferSize; ++i)
    {
        m_pBuffer[i] = 0;
    }
    */

    //m_iMaxCols = m_pTerminalInterface != 0 ? m_pTerminalInterface->GetScreenWidth() : 0;
    //m_iMaxRows = m_pTerminalInterface != 0 ? m_pTerminalInterface->GetScreenHeight() : 0;
    m_iCurrentCursorX = 0;
    m_iCurrentCursorY = 0;
    m_iVirtualCursorXPos = 0;
}

void EdifixEditor::DeInit()
{
    for(int i=0; i<m_iCurrentBufferSize; ++i)
    {
        free(m_pBuffer[i]);
    }
    free(m_pBuffer);
    m_pBuffer = 0;
    m_iCurrentBufferSize = 0;
}

void EdifixEditor::FillBuffer(const char * text)
{
    SimpleString s;
    char ch;
    int i = 0;
    
    while( (ch = text[i]) != 0 )
    {
        if( ch == '\r' || ch == '\n' )
        {
            AddLine(s.c_str());
            s.clear();
            
            // skip next character of sequences like: \r \n or \n \r
            if( text[i+1] != 0 && (text[i+1] == '\r' || text[i+1] == '\n') )
            {
                ++i;
            }
        }
        else
        {
            s.push_back(ch);
        }
        ++i;
    }
    if( s.size() > 0 )
    {
        AddLine(s.c_str());
    }
}

void EdifixEditor::IncreaseTextBufferIfNeeded()
{
    // is current buffer too small ?
    if( m_iNextIndexToAddToBuffer >= m_iCurrentBufferSize )
    {
        // yes --> resize buffer
        m_pBuffer = (char * *)realloc(m_pBuffer, (m_iCurrentBufferSize + DELTA) * sizeof(char *));
        // initialize new area of memory
        for(int i=m_iCurrentBufferSize; i<m_iCurrentBufferSize+DELTA; ++i)
        {
            m_pBuffer[i] = 0;
        }
        m_iCurrentBufferSize = m_iCurrentBufferSize + DELTA;
    }
    
}

void EdifixEditor::AddLine(const char * line)
{   
    IncreaseTextBufferIfNeeded();
    
    if( m_iNextIndexToAddToBuffer < m_iCurrentBufferSize )
    {
        // free current content of line (if existing)
        if( m_pBuffer[m_iNextIndexToAddToBuffer] != 0 )
        {
            free(m_pBuffer[m_iNextIndexToAddToBuffer]);
            m_pBuffer[m_iNextIndexToAddToBuffer] = 0;
        }
        // create string to add into array of strings
        if( m_pBuffer[m_iNextIndexToAddToBuffer] == 0 )
        {
            m_pBuffer[m_iNextIndexToAddToBuffer] = (char *)malloc(strlen(line)+1);
            strcpy(m_pBuffer[m_iNextIndexToAddToBuffer], line);
        }
        ++m_iNextIndexToAddToBuffer;
    }        
}

void EdifixEditor::InsertLineAtCursorAndUpdateScreen(const char * line)
{
    IncreaseTextBufferIfNeeded();
        
    for(int i=m_iNextIndexToAddToBuffer; i>m_iCurrentCursorY; i--)
    {
        m_pBuffer[i] = m_pBuffer[i-1];
    }
    m_iCurrentCursorX = 0;
    m_pBuffer[m_iCurrentCursorY] = (char *)malloc(strlen(line)+1);
    strcpy(m_pBuffer[m_iCurrentCursorY], line);
    
    ++m_iNextIndexToAddToBuffer;
    UpdateScreen();
    UpdateCursor();
}

void EdifixEditor::UpdateScreen()
{
    if( m_pTerminalInterface )
    {
        m_pTerminalInterface->ClearScreen();
        DrawTextBlock(0, m_pTerminalInterface->GetScreenHeight(), 0);
    }
}

void EdifixEditor::UpdateCurrentLine(bool bEraseToOfLine)
{
    // update current line / or rest of line
    if( m_pTerminalInterface )
    {
        m_pTerminalInterface->Write(0,m_iCurrentCursorY,m_pBuffer[m_iCurrentCursorY],bEraseToOfLine);
        UpdateCursor();
    }    
}

void EdifixEditor::UpdateCursor()
{
    if( m_pTerminalInterface )
    {
        m_pTerminalInterface->SetCursor(m_iCurrentCursorX,m_iCurrentCursorY);
    }
}

void EdifixEditor::DrawTextBlock(int iScreenStartRow, int iNumberOfRows, int iBufferStartPos)
{
    if( m_pTerminalInterface )
    {
        int rowCount = _min(iScreenStartRow+iNumberOfRows,m_pTerminalInterface->GetScreenHeight());
        int i = 0;
        for(int row=iScreenStartRow; row<rowCount; ++row)
        {
            int index = iBufferStartPos + i;
//std::cerr << "index=" << index << " " << m_iNextIndexToAddToBuffer << std::endl;            
//if( index < m_iNextIndexToAddToBuffer )
//  std::cerr << "Draw " << i << " " << (m_pBuffer[i] != 0 ? m_pBuffer[i] : "null") << std::endl;    
            if( index < m_iNextIndexToAddToBuffer && m_pBuffer[index] )
            {
                m_pTerminalInterface->Write(0, row, m_pBuffer[index], /*eraseToEndOfLine=*/false);
            }
            ++i;
        }
    }
}

bool EdifixEditor::CursorLeft()
{
    bool ok = true;
    
    if( m_pTerminalInterface )
    {
        if( m_iCurrentCursorX-1 >= 0 )
        {
            m_iCurrentCursorX--;
            UpdateCursor();
        }
        else
        {
            if( CursorUp() )
            {
                CursorToEndOfText();
            }
            else
            {
                // Cursor can not move !
                ok = false;        
            }           
        }
    }
    
    return ok;
}

bool EdifixEditor::CursorRight()
{
    bool ok = true;
    
    if( m_pTerminalInterface )
    {
        if( m_iCurrentCursorX+1 < m_pTerminalInterface->GetScreenWidth() && m_iCurrentCursorX+1 <= GetLengthOfLine(m_iCurrentCursorY) )
        {
            m_iCurrentCursorX++;
            UpdateCursor();
        }
        else
        {
            if( CursorDown() )
            {
                m_iCurrentCursorX = 0;
                UpdateCursor();
            }
            else
            {
                // Cursor can not move !
                ok = false;        
            }           
        }
    }
    
    return ok;
}

bool EdifixEditor::CursorUp()
{
    bool ok = true;
    
    if( m_pTerminalInterface )
    {
        if( m_iCurrentCursorY-1 >= 0 )
        {
            m_iCurrentCursorY--;
            FixCursorForLine(m_iCurrentCursorY);
            UpdateCursor();
        }
        else
        {
    // TODO: not finished
    /*          
            if( ScrollUp() )
            {
                if( m_iCurrentCursorX > LengthOfTextInLine() )
                {
                    CursorToEndOfText();
                }
            }
            else
    */            
            {
                // Cursor can not move !
//                ok = false;        
            }  
        }         
    }
    
    return ok;
}

bool EdifixEditor::CursorDown()
{
    bool ok = true;
    
    if( m_pTerminalInterface )
    {
        if( m_iCurrentCursorY+1 < m_pTerminalInterface->GetScreenHeight() && m_iCurrentCursorY+1 < GetLineCount() )
        {
            m_iCurrentCursorY++;
            FixCursorForLine(m_iCurrentCursorY);
            UpdateCursor();
        }
        else
        {
    // TODO: not finished
    /*     
            AdjustViewPort();
                   
            if( ScrollUp() )
            {
                if( m_iCurrentCursorX > LengthOfTextInLine() )
                {
                    CursorToEndOfText();
                }
            }
            else
    */            
            {
                // Cursor can not move !
                ok = false;        
            }
        }           
    }
    
    return ok;
}

void EdifixEditor::FixCursorForLine(int iLineNo)
{
    if( m_iCurrentCursorX > GetLengthOfLine(iLineNo) )
    {
        m_iVirtualCursorXPos = m_iCurrentCursorX;
        m_iCurrentCursorX = GetLengthOfLine(iLineNo);
    }
    else if( m_iVirtualCursorXPos > m_iCurrentCursorX )
    {
        m_iCurrentCursorX = _min(m_iVirtualCursorXPos,GetLengthOfLine(iLineNo)); 
    }
}

void EdifixEditor::CursorToEndOfText()
{
    m_iCurrentCursorX = GetLengthOfLine(m_iCurrentCursorY);
    UpdateCursor();
}

void EdifixEditor::AppendString(const char * text)
{
    int currentLen = GetLengthOfLine(m_iCurrentCursorY);
    char * newLine = (char *)malloc(currentLen+strlen(text)+1);
    char * currentLine = m_pBuffer[m_iCurrentCursorY];
    strcpy(newLine,currentLine);
    strcat(newLine+currentLen-1,text);
    free(m_pBuffer[m_iCurrentCursorY]);
    m_pBuffer[m_iCurrentCursorY] = newLine;
}

void EdifixEditor::InsertCharacterAtCursor(char ch)
{
    // insert into buffer
    int currentLen = GetLengthOfLine(m_iCurrentCursorY);
    char * newLine = (char *)malloc(currentLen+2);
    char * currentLine = m_pBuffer[m_iCurrentCursorY];
    strncpy(newLine,currentLine,m_iCurrentCursorX);
    newLine[m_iCurrentCursorX] = ch;
    strncpy(newLine+m_iCurrentCursorX+1,currentLine+m_iCurrentCursorX,currentLen-m_iCurrentCursorX);
    newLine[currentLen+1] = 0;
    free(m_pBuffer[m_iCurrentCursorY]);
    m_pBuffer[m_iCurrentCursorY] = newLine;
    
    m_iCurrentCursorX++;
    UpdateCurrentLine();
}

void EdifixEditor::DeleteCharacterAtCursor()
{
    int currentLen = GetLengthOfLine(m_iCurrentCursorY);
    char * currentLine = m_pBuffer[m_iCurrentCursorY];
    memmove(currentLine+m_iCurrentCursorX,currentLine+m_iCurrentCursorX+1,currentLen-m_iCurrentCursorX);     // include terminating 0
    
    UpdateCurrentLine(true);
}

void EdifixEditor::RemoveLine(int iLineNo)
{
    if( iLineNo >= 0 && iLineNo < m_iNextIndexToAddToBuffer )
    {
        // delete the given line
        free(m_pBuffer[iLineNo]);
        
        // move all the lines one line down
        for( int i=iLineNo; i<m_iNextIndexToAddToBuffer-1; ++i)
        {
            m_pBuffer[i] = m_pBuffer[i+1];
        }
        --m_iNextIndexToAddToBuffer;
        m_pBuffer[m_iNextIndexToAddToBuffer] = 0;
    }
}

int EdifixEditor::GetLengthOfLine(int iLineNo) const 
{
    if( iLineNo>=0 && iLineNo<GetLineCount() )
    {
        return strlen(m_pBuffer[iLineNo]);
    }
    
    return -1;
}
