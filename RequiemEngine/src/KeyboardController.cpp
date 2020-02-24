#include <headers/KeyboardController.h>
#include <Windows.h>
#include <iostream>

KeyboardController::KeyboardController()
    : m_Phase(0),
    m_MouseMovement(0.0f)
{
    m_Set[0].fill(0);
    m_Set[1].fill(0);

    POINT p;
    GetCursorPos(&p);
    m_MousePos = glm::vec2(p.x, p.y);
}

void KeyboardController::UpdateController()
{
	//HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
	//DWORD cNumRead, fdwMode, i;
	//INPUT_RECORD irInBuf[128];
	m_Phase = (m_Phase + 1) % 2;
	//m_Set[m_Phase].reset();

 //   DWORD num = 0;
 //   GetNumberOfConsoleInputEvents(hStdin, &num);
	//if (num > 0 && ReadConsoleInput(
	//	hStdin,      // input buffer handle 
	//	irInBuf,     // buffer to read into 
 //       num,         // size of read buffer 
	//	&cNumRead))
	//{
 //       std::cout << cNumRead << std::endl;
 //       for (int i = 0; i < cNumRead; i++)
 //       {
 //           switch (irInBuf[i].EventType)
 //           {
 //           case KEY_EVENT: // keyboard input 
 //               //KeyEventProc(irInBuf[i].Event.KeyEvent);
 //           {
 //               if (irInBuf[i].Event.KeyEvent.bKeyDown)
 //               {
 //                   m_Set[m_Phase].set(irInBuf[i].Event.KeyEvent.wVirtualKeyCode, true);
 //               }
 //           }
 //               break;

 //           //case MOUSE_EVENT: // mouse input 
 //           //    MouseEventProc(irInBuf[i].Event.MouseEvent);
 //           //    break;

 //           //case WINDOW_BUFFER_SIZE_EVENT: // scrn buf. resizing 
 //           //    ResizeEventProc(irInBuf[i].Event.WindowBufferSizeEvent);
 //           //    break;

 //           //case FOCUS_EVENT:  // disregard focus events 

 //           //case MENU_EVENT:   // disregard menu events 
 //           //    break;

 //           //default:
 //           //    ErrorExit("Unknown event type");
 //           //    break;
 //           }
 //       }
	//}
 //   FlushConsoleInputBuffer(hStdin);

    memset(m_Set[m_Phase].data(), 0, sizeof(256));
    GetKeyState(0);
    if (GetKeyboardState(m_Set[m_Phase].data()))
    {
        //for (int i = 0; i < 256; i++)
        //{
        //    int temp = (int)m_Set[m_Phase][i];
        //    temp >>= 7;
        //    std::cout << temp;
        //}
        //std::cout << std::endl;
        //Sleep(5000);
    }

    POINT p;
    m_MouseMovement = glm::vec2(0.0f);
    if (GetCursorPos(&p))
    {
        glm::vec2 new_pos(p.x, p.y);
        m_MouseMovement = new_pos - m_MousePos;
        m_MouseMovement.y = -m_MouseMovement.y;
        m_MousePos = new_pos;
    }
}

bool KeyboardController::KeyState(uint16_t key_word)
{
    return m_Set[m_Phase][key_word] >> 7;
}

bool KeyboardController::KeyReleased(uint16_t key_word)
{
    uint8_t last_phase = (m_Phase + 1) % 2;
    return (!(m_Set[m_Phase][key_word] >> 7)) && (m_Set[last_phase][key_word] >> 7);
}

bool KeyboardController::KeyTriggered(uint16_t key_word)
{
    uint8_t last_phase = (m_Phase + 1) % 2;
    return (!(m_Set[last_phase][key_word] >> 7)) && (m_Set[m_Phase][key_word] >> 7);
}

uint16_t KeyboardController::GetCharKey(char _char)
{
    return toupper(_char) - 'A' + 0x41;
}

uint16_t KeyboardController::GetNumKey(uint8_t num)
{
    return num + 0x30;
}

glm::vec2 KeyboardController::GetMouseMovement()
{
    return m_MouseMovement;
}

bool KeyboardController::GetMouseState(int mouse_btn)
{
    return false;
}
