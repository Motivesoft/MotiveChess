// MotiveChess.cpp : Defines the entry point for the application.
//

#include "MotiveChess.h"

int main( int argc, char** argv )
{
	//printf( "\n" );
	//printf( "\x1B[31mTextin\033[0mg\t\t" );
	//printf( "\x1B[32mTextin\033[0mg\t\t" );
	//printf( "\x1B[33mTextin\033[0mg\t\t" );
	//printf( "\x1B[34mTextin\033[0mg\t\t" );
	//printf( "\x1B[35mTextin\033[0mg\n" );

	//printf( "\x1B[36mTexting\033[0m\t\t" );
	//printf( "\x1B[37mTexting\033[0m\t\t" );
	//printf( "\x1B[38mTexting\033[0m\t\t" );
	//printf( "\x1B[39mTexting\033[0m\t\t" );
	//printf( "\x1B[40mTexting\033[0m\n" );

	//printf( "\x1B[93mTexting\033[0m\n" );

	//printf( "\033[3;42;30mTexting\033[0m\t\t" );
	//printf( "\033[3;43;30mTexting\033[0m\t\t" );
	//printf( "\033[3;44;30mTexting\033[0m\t\t" );
	//printf( "\033[3;104;30mTexting\033[0m\t\t" );
	//printf( "\033[3;100;30mTexting\033[0m\n" );

	//printf( "\033[3;47;35mTexting\033[0m\t\t" );
	//printf( "\033[2;47;35mTexting\033[0m\t\t" );
	//printf( "\033[1;47;35mTexting\033[0m\t\t" );
	//printf( "\t\t" );
	//printf( "\n" );
	//return 0;
	std::cout << "MotiveChess 0.1" << std::endl;

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
		std::cout << "  " << switchPrefix << "input [filename]   : read input from [filename], rather than the console" << std::endl;
		std::cout << "  " << switchPrefix << "help               : this information" << std::endl;
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
			else if ( flag == "debug" )
			{
				engine.setDebug();
			}
			else if ( flag == "tee" )
			{
				engine.setTee();
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
