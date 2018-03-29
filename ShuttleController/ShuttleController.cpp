#include <boost/asio.hpp>
#include "ShuttleSDK.h"
#include <vector>
#include "Context.h"
#include <iostream>
#include <boost\program_options.hpp>
#include <algorithm>
#include <iterator>

using namespace std;

void CALLBACK ShuttleCallback(DWORD event, UCHAR status, WORD type, WORD devicenumber);
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wp, LPARAM lp);
namespace po = boost::program_options;

// A helper function to simplify the main part.
template<class T> 
ostream& operator<<(ostream& os, const vector<T>& v)
{
	copy(v.begin(), v.end(), ostream_iterator<T>(os, " "));
	return os;
}

void main(int ac, char* av[])
{
	try {
		int opt;
		po::options_description desc("Allowed options");
		desc.add_options()
			("input-file", po::value< std::vector<std::string> >(), "input file")
			;

		po::positional_options_description p;
		p.add("input-file", -1);

		po::variables_map vm;
		po::store(po::command_line_parser(ac, av).
			options(desc).positional(p).run(), vm);
		po::notify(vm);

		if (vm.count("input-file"))
		{
			std::cout << "Input files are: ";
			std::cout << vm["input-file"].as< std::vector<std::string> >() << "\n";
		}



		MSG msg;

		// Register the callback. This operation initializes the library, create the receiver window
		// if needed and register the receiver window.
		// You can register the same callback for any device so your callback will be called if any event from
		// that device occurs.
		if (Shuttle_Register_Callback_Global(ShuttleCallback, SHUTTLEPRO2, SHUTTLEPRIMARY) != SHUTTLESDK_OK)
		{
			printf("Error: Shuttle_Register_Callback\n");
			return;
		}
		Context::Init(vm["input-file"].as<vector<string>>()[0]);

		//Context::SendCommand("test");

		// This cycle is needed in order to allow the hidden receiver window to receive shuttle
		// notifications.
		while (GetMessage(&msg, NULL, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// Always clean up.
		Shuttle_Unregister_Callback(SHUTTLEPRO2, SHUTTLEPRIMARY);
		//Context::ShutDown();
	}catch (const std::exception& ex)
	{
		printf(ex.what());
		printf("\r\n");
	}
}

void CALLBACK ShuttleCallback(DWORD event, UCHAR status, WORD type, WORD devicenumber)
{
	switch (type)
	{
	case SHUTTLEXPRESS:
		printf("ShuttleXpress ");
		break;
	case SHUTTLEPRO:
		printf("ShuttlePro    ");
		break;
	case SHUTTLEPRO2:
		printf("ShuttlePro2   ");
		break;
	default:
		printf("Shuttle?      ");
	}

	switch (devicenumber)
	{
	case SHUTTLEPRIMARY:
		printf("(1) ");
		break;
	case SHUTTLESECONDARY:
		printf("(2) ");
		break;
	default:
		printf("(?) ");
		break;
	}

	if ((event >= SHUTTLEEVENTSHUTTLEFIRST) && (event <= SHUTTLEEVENTSHUTTLELAST))
	{
		printf(" Shuttle ");
		if (status) printf("enters ");
		else printf("exits ");
		printf("zone %d", event - SHUTTLEEVENTSHUTTLE);
		printf("\n");
	}

	if ((event >= SHUTTLEEVENTBUTTONFIRST) && (event <= SHUTTLEEVENTBUTTONLAST))
	{
		printf(" Button %d", event - SHUTTLEEVENTBUTTON);
		if (status) {
			printf(" pressed");
			Context::AddAction(event);
			printf("\n");
		}
		else {
			printf(" released");
			printf("\n");

			/*
			auto currentPressed = Context::GetPressedKeys();
			for (int i = 0; i < currentPressed.size(); i++) {
				printf(" Already Button %d \n", currentPressed[i] - SHUTTLEEVENTBUTTON);
			}
			*/

			Context::ProcessCommand();

			Context::Clear();
		}
	}

	if ((event >= SHUTTLEEVENTJOGFIRST) && (event <= SHUTTLEEVENTJOGLAST))
	{
		if (status)
		{
			printf(" Jog ");
			switch (event)
			{
			case SHUTTLEEVENTJOGLEFT:
				printf("left\r\n");
				Context::ProcessCommand(event);
				break;
			case SHUTTLEEVENTJOGRIGHT:
				printf("right\r\n");
				Context::ProcessCommand(event);
				break;
			}
		}
	}


}
