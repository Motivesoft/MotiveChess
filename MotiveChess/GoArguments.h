#pragma once

#include <string>

class GoArguments
{
    GoArguments();

public:
    class Builder
    {
    public:
        GoArguments build();
    };
};