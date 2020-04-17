
#include "EdifixEditor.h"

#include <stdlib.h>
#include <string.h>

// for debugging only
typedef void (*pfun_t)(char);
void pserial (char c);
void pfstring (const char *s, pfun_t pfun);
void pfl (pfun_t pfun);

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
    m_iBufferSize = INITIAL_BUFFER_SIZE;
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
{
    Init();
    
    FillBuffer(text);
    
    m_pTerminalInterface = pTerminalInterface;
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
    bool ok = false;
    
    if( m_pTerminalInterface )
    {       
        m_pTerminalInterface->SaveScreen();        
        m_pTerminalInterface->ClearScreen();        
        m_pTerminalInterface->SetTerminalModus(true);
        
        DrawTextBlock(0, m_pTerminalInterface->GetScreenHeight(), 0);
        m_pTerminalInterface->SetCursor(0, 0);
        
        // key and mouse event loop ...        
        int event = m_pTerminalInterface->GetNextEvent();
ok = true;        
        while( event != 0 )
        {
            ProcessEvent(event, &ok);
            
            event = m_pTerminalInterface->GetNextEvent();
        }
        
        m_pTerminalInterface->SetTerminalModus(false);
        m_pTerminalInterface->RestoreScreen();
    }
    
    return ok;
}

void EdifixEditor::ProcessEvent(int code, bool * pOk)
{
    if( pOk )
    {
        *pOk = true;
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
//pserial(ch);        
        if( ch == '\r' || ch == '\n' )
        {
//pfstring(" NL ", pserial);
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
//pfstring(" PUSH ", pserial);
            s.push_back(ch);
        }
        ++i;
    }
    if( s.size() > 0 )
    {
        AddLine(s.c_str());
    }
}

void EdifixEditor::AddLine(const char * line)
{   
//     pfstring(line, pserial);
//     pfl(pserial);
 
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

void EdifixEditor::DrawTextBlock(int iScreenStartRow, int iNumberOfRows, int iBufferStartPos)
{
    if( m_pTerminalInterface )
    {
        int rowCount = min(iScreenStartRow+iNumberOfRows,m_pTerminalInterface->GetScreenHeight());
        int i = 0;
        for(int row=iScreenStartRow; row<rowCount; ++row)
        {
            int index = iBufferStartPos + i;
            if( index < m_iNextIndexToAddToBuffer && m_pBuffer[index] )
            {
                m_pTerminalInterface->Write(0, row, m_pBuffer[index], /*eraseToEndOfLine=*/false);
            }
            ++i;
        }
    }
}
