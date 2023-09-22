#pragma once

class CopyProtection
{
public:
    enum class Status
    {
        CHECKING,
        OK,
        ERROR
    };

    static const char* toString( const CopyProtection::Status status )
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
