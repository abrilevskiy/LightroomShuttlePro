#include <boost/asio.hpp>
#include <unordered_map>
#include "Context.h"
#include <boost/algorithm/string/join.hpp>
#include <cereal/archives/xml.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/unordered_map.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/filesystem.hpp>
#include <fstream>
#include <iostream>
#include <regex>
#include "LrConnections.h"

std::vector<unsigned long> Context::_pressed_keys = std::vector<unsigned long>();

LrConnections Context::_lrConnection = LrConnections();
std::unordered_map<DWORD, std::string> keyCodeToName = std::unordered_map<DWORD, std::string>();
std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::string>>> commandKeyToAction = std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::string>>>();
std::string Context::_currentSettings = std::string();

Context::Context()
{
}

Context::~Context()
{
}

void Context::Init(std::string commandToKeyPath) {
	printf("Init is being executed...\r\n");
	_lrConnection.Connect();
	_currentSettings = "Library";

	for (DWORD i = 61; i <= 75; i++) {
		keyCodeToName.insert(std::pair<DWORD, std::string>(i, "Button_" + std::to_string(i - 60)));
	}

	keyCodeToName.insert(std::pair<DWORD, std::string>(0, "Jog_Left"));
	keyCodeToName.insert(std::pair<DWORD, std::string>(1, "Jog_Right"));


	commandKeyToAction = {
		{
			"Library",
			{
				{ "Jog_Left",	{ "ActiveDevelopParam = -" }},
				{ "Jog_Right",	{ "ActiveDevelopParam = +" }},
				{ "Button_4",	{ "rating = 1" } },
				{ "Button_5",	{ "rating = 2" } },
				{ "Button_6",	{ "rating = 3" } },
				{ "Button_7",	{ "rating = 4" } },
				{ "Button_8",	{ "rating = 5" } },
				{ "Button_9+Jog_Left",	{ "SwitchBasicAdjustParam = -" }},
				{ "Button_9+Jog_Right",	{ "SwitchBasicAdjustParam = +" }},
				{ "Button_9+Button_7",{ "ShuttleSwitchProfile = Library", "SwitchToModule = library" } },
				{ "Button_9+Button_8",{ "ShuttleSwitchProfile = Develop", "SwitchToModule = develop" } }
			}
		},
		{
			"Develop",
			{
				{ "Jog_Left",	{ "ActiveDevelopParam = -" }},
				{ "Jog_Right",	{ "ActiveDevelopParam = +" }},
				{ "Button_9+Jog_Left",	{ "SwitchBasicAdjustParam = -" }},
				{ "Button_9+Jog_Right",	{ "SwitchBasicAdjustParam = +" }},
				{ "Button_10+Jog_Left",	{ "SwitchDetailsAdjustParam = -" } },
				{ "Button_10+Jog_Right",	{ "SwitchDetailsAdjustParam = +" } },
				{ "Button_9+Button_7",	{ "ShuttleSwitchProfile = Library", "SwitchToModule = library" }},
				{ "Button_9+Button_8",	{ "ShuttleSwitchProfile = Develop", "SwitchToModule = develop" }}
			}
		}

	};
	{
		/*
		//for write
		std::ofstream ofs("commandKeyToAction.xml");
		/*
		boost::archive::xml_oarchive xmlOut(ofs,boost::archive::no_header| boost::archive::no_tracking);
		xmlOut << boost::serialization::make_nvp("options", commandKeyToAction);
		ofs.close();
		
		

		cereal::XMLOutputArchive archive(ofs);
		archive(commandKeyToAction);
		*/
	};


	commandKeyToAction.clear();
	
	printf("Reading key mapping...\r\n");



	try
	{
		{
			std::ifstream ifs(commandToKeyPath + "\\commandKeyToAction.xml");
			cereal::XMLInputArchive archive(ifs);
			archive(commandKeyToAction);
		};
	}
	catch (const std::exception& ex)
	{
		printf(ex.what());
		printf("\r\n");
	}

	printf("Key mapping is loaded.\r\n");
	/*

	boost::archive::xml_iarchive xmlIn(ifs, boost::archive::no_header | boost::archive::no_tracking);
	xmlIn >> boost::serialization::make_nvp("options", commandKeyToAction);
	*/

}

void Context::AddAction(unsigned long action) {
	_pressed_keys.push_back(action);
}

void Context::Clear() {
	_pressed_keys.clear();
}

std::string Context::GetCommandKey(DWORD lastPressedKeyCode) {
	std::vector<std::string> returnKeys = std::vector<std::string>();
	for (int i = 0; i < _pressed_keys.size(); i++) {
		returnKeys.push_back(keyCodeToName[_pressed_keys[i]]);
	}
	if (lastPressedKeyCode != -1) {
		returnKeys.push_back(keyCodeToName[lastPressedKeyCode]);
	}
	return boost::algorithm::join(returnKeys, "+");
}


void Context::ProcessCommand(DWORD lastPressedKeyCode)
{

	std::string prefix("Shuttle");

	std::unordered_map<std::string, std::function<void(std::string)>> ShuttleCommand_map;
	ShuttleCommand_map["ShuttleSwitchProfile"] = [](std::string profileName) {
		Context::ShuttleSwitchProfile(profileName);
	};

	//script_map m;
//m.emplace("blah", &some_function);




	auto commandKeyPrefix = Context::GetCommandKey(lastPressedKeyCode);
	std::cout << commandKeyPrefix + "\r\n";
	for (std::size_t i = 0; i < commandKeyToAction[_currentSettings][commandKeyPrefix].size(); i++) {
		auto commandToProcess = commandKeyToAction[_currentSettings][commandKeyPrefix][i];
		std::cout << commandToProcess + "\r\n";
		if (commandToProcess.compare(0, prefix.size(), prefix)) {
			_lrConnection.SendCommand(commandToProcess);
		}
		else {
			std::regex rgx("(\\w+)\\s*=\\s*(\\w+)");
			std::smatch match;
			std::regex_search(commandToProcess, match, rgx);
			auto shuttleCommand = match[1].str();
			auto shuttleCommandValue = match[2].str();
			std::cout << "Before regexp: " + commandToProcess + "\r\n";
			std::cout << "Shuttle command: " + shuttleCommand + "; Value: " + shuttleCommandValue + "\r\n";
			ShuttleCommand_map[shuttleCommand](shuttleCommandValue);

			/*if (shuttleCommand == "ShuttleSwitchProfile") {
				_currentSettings = shuttleCommandValue;
			}
			*/

		}
	}
}


void Context::ShuttleSwitchProfile(std::string profileName) {
	_currentSettings = profileName;
	std::cout << "Shuttle profile is switched to: " + _currentSettings + "\r\n";
}

std::vector<unsigned long>  Context::GetPressedKeys() {
	return _pressed_keys;
}