// MotiveChess.cpp : Defines the entry point for the application.
//

#include "MotiveChess.h"

int main( int argc, char** argv )
{
	std::cout << "MotiveChess 0.1" << std::endl;

	std::vector<std::string> args;
	for ( unsigned short loop = 1; loop < argc; loop++ )
	{
		args.push_back( argv[ loop ] );
	}

#if _WIN32
	std::string switchPrefix = "-";
#elif __linux__
	std::string switchPrefix = "--";
#endif

	Engine engine;
	if ( processCommandLine( engine, switchPrefix, args ) )
	{
		// Continue
		engine.initialize();
		engine.run();
	}
	else
	{
		std::cout << std::endl << "Arugments:" << std::endl;
		std::cout << "  " << switchPrefix << "debug             : turn on debug mode" << std::endl;
		std::cout << "  " << switchPrefix << "input [filename]  : read input from [filename], rather than the console" << std::endl;
		std::cout << "  " << switchPrefix << "help              : this information" << std::endl;
	}

	// Exit
	return 0;
}

bool processCommandLine( Engine& engine, const std::string& switchPrefix, const std::vector<std::string>& args )
{
	bool success = true;

	for ( std::vector<std::string>::const_iterator it = args.cbegin(); success == true, it != args.cend(); )
	{
		if ( (*it).starts_with( switchPrefix ) )
		{
			std::string flag = ( *it ).substr( switchPrefix.length() );
			if ( flag == "help" )
			{
				success = false;
			}
			if ( flag == "debug" )
			{
				engine.setDebug();
			}
			else if ( flag == "input" )
			{
				if ( it + 1 != args.cend() )
				{
					it++;
					engine.setInputFile( *it );
				}
				else
				{
					std::cerr << "Missing input file: " << std::endl;
					success = false;
				}
			}
			else if ( flag == "logfile" )
			{
				if ( it + 1 != args.cend() )
				{
					it++;
					engine.setLogFile( *it );
				}
				else
				{
					std::cerr << "Missing log file: " << std::endl;
					success = false;
				}
			}
			else
			{
				std::cerr << "Unrecognised argument: " << *it << std::endl;
				success = false;
			}
		}
		else
		{
			std::cerr << "Unexpected argument: " << *it << std::endl;
			success = false;
		}

		it++;
	}

	return success;
}
