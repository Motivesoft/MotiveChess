// MotiveChess.cpp : Defines the entry point for the application.
//

#include "MotiveChess.h"

#include <iostream>
#include <string>
#include <vector>

bool processCommandLine( const std::vector<std::string>& args );

int main( int argc, char** argv )
{
	std::cout << "MotiveChess 0.1" << std::endl;

	std::vector<std::string> args;
	for ( unsigned short loop = 1; loop < argc; loop++ )
	{
		args.push_back( argv[ loop ] );
	}

	if ( processCommandLine( args ) )
	{
		// Continue
	}

	// Exit
	return 0;
}

bool processCommandLine( const std::vector<std::string>& args )
{
#if _WIN32
	std::string switchPrefix = "-";
#elif __linux__
	std::string switchPrefix = "--";
#endif

	std::cout << switchPrefix << std::endl;

	for ( std::vector<std::string>::const_iterator it = args.cbegin(); it != args.cend(); )
	{
		if ( (*it).starts_with( switchPrefix ) )
		{
			std::string flag = ( *it ).substr( switchPrefix.length() );
			if ( flag == "debug" || *it == "-d" )
			{
				// TODO Set debug mode
			}
			else if ( flag == "input" || *it == "-i" )
			{
				if ( it + 1 != args.cend() )
				{
					// TODO Set input from file
					std::string inputFile = *( ++it );
				}
				else
				{
					std::cerr << "Missing input file: " << std::endl;
				}
			}
			else
			{
				std::cerr << "Unrecognised argument: " << *it << std::endl;
			}
		}
		else
		{
			std::cerr << "Unexpected argument: " << *it << std::endl;
		}

		it++;
	}

	return true;
}
