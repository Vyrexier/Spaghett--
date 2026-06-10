#pragma once
#include <cstdint>

namespace Spaghett
{
    struct Position
    {
        unsigned int line;
        unsigned int column;

        Position(unsigned int line = 0, unsigned int column = 0)
            : line(line), column(column) {
        }

        bool operator==(const Position& rhs) const
        {
            return line == rhs.line && column == rhs.column;
        }

        bool operator!=(const Position& rhs) const
        {
            return !(*this == rhs);
        }
    };

    struct Location
    {
        Position begin;
        Position end;

        Location() : begin(0, 0), end(0, 0) {}

        Location(Position begin, Position end)
            : begin(begin), end(end) {
        }

        Location(Position begin, unsigned int length)
            : begin(begin), end(begin.line, begin.column + length) {
        }

        bool operator==(const Location& rhs) const
        {
            return begin == rhs.begin && end == rhs.end;
        }
    };
}