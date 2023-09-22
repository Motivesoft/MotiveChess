#pragma once

#include <string>

class Registration
{
    bool registered;
    std::string name;
    std::string code;

public:
    enum class Status
    {
        CHECKING,
        OK,
        ERROR
    };

    Registration() :
        registered( false )
    {
    }

    bool registerLater()
    {
        // Indicating that we have allowed the request to register later, but this is not a permanent
        // registration
        registered = true;

        return registered;
    }

    bool registerNameCode( const std::string& name, const std::string& code )
    {
        this->name = name;
        this->code = code;

        // TODO validate
        registered = true;

        // Report success or failure
        return registered;
    }

    bool isRegistered()
    {
        return registered;
    }

    static const char* toString( const Registration::Status status )
    {
        switch ( status )
        {
            case Status::CHECKING:
                return "checking";

            case Status::OK:
                return "ok";

            default:
            case Status::ERROR:
                return "error";
        }
    }
};