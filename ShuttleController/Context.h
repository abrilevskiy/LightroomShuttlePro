#pragma once
#include <boost/asio.hpp>
#include "LrConnections.h"
#include <vector>
#include <string>

#ifndef CONTEXT_H
#define CONTEXT_H


class Context
{
	static std::vector<unsigned long> _pressed_keys;
	static LrConnections _lrConnection;
	static std::string _currentSettings;

public:
	Context();
	~Context();
	void static Init(std::string commandToKeyPath);
	void static AddAction(unsigned long);
	void static Clear();
	std::string static GetCommandKey(DWORD lastPressedKeyCode = -1);
	void static ProcessCommand(DWORD lastPressedKeyCode = -1);
	void static ShuttleSwitchProfile(std::string value);
	std::vector<unsigned long> static GetPressedKeys();

};

#endif