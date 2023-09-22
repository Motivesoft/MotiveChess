#include "MotiveChess.h"

#include "Version.h"

int main( int argc, char** argv )
{
	printf( "MotiveChess %d.%d.%d.%d\n", MotiveChess_VERSION_MAJOR, MotiveChess_VERSION_MINOR, MotiveChess_VERSION_PATCH, MotiveChess_VERSION_TWEAK );

	std::vector<std::string> args;
	for ( unsigned short loop = 1; loop < argc; loop++ )
	{
		args.push_back( argv[ loop ] );
	}

#ifdef _WIN32
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
		std::cout << std::endl << "Arguments:" << std::endl;
		std::cout << "  " << switchPrefix << "debug              : turn on debug mode" << std::endl;
		std::cout << "  " << switchPrefix << "logfile [filename] : log to [filename], rather than to stderr (console)" << std::endl;
		std::cout << "  " << switchPrefix << "tee                : when used with '" << switchPrefix << "logfile', log to file and stderr (console)" << std::endl;
		std::cout << "  " << switchPrefix << "colorized          : use colors for stderr (console) logging" << std::endl;
		std::cout << "  " << switchPrefix << "silent             : disable all logging" << std::endl;
		std::cout << "  " << switchPrefix << "input [filename]   : read input from [filename], rather than the console" << std::endl;
		std::cout << "  " << switchPrefix << "help               : this information" << std::endl;
	}

	// Exit
	return 0;
}

bool processCommandLine( Engine& engine, const std::string& switchPrefix, const std::vector<std::string>& args )
{
	bool success = true;

	for ( std::vector<std::string>::const_iterator it = args.cbegin(); success && it != args.cend(); )
	{
		if ( (*it).starts_with( switchPrefix ) )
		{
			std::string flag = ( *it ).substr( switchPrefix.length() );
			if ( flag == "help" )
			{
				success = false;
			}
			else if ( flag == "debug" )
			{
				engine.setDebug();
			}
			else if ( flag == "tee" )
			{
				engine.setTee();
			}
			else if ( flag == "colorized" )
			{
				engine.setColorizedLogging();
			}
			else if ( flag == "silent" )
			{
				engine.setSilent();
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
