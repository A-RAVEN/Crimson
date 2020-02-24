#pragma once
#include <bitset>
#include <array>
#include <glm/glm.hpp>

class KeyboardController
{
public:
	KeyboardController();
	void UpdateController();
	bool KeyState(uint16_t key_word);
	bool KeyReleased(uint16_t key_word);
	bool KeyTriggered(uint16_t key_word);
	uint16_t GetCharKey(char _char);
	uint16_t GetNumKey(uint8_t num);
	glm::vec2 GetMouseMovement();
	bool GetMouseState(int mouse_btn);
private:
	glm::vec2 m_MousePos;
	glm::vec2 m_MouseMovement;
	uint8_t m_Phase;
	//std::bitset<256> m_Set[2];
	std::array<uint8_t, 256> m_Set[2];
};