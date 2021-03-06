/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2020 Thomas Pointhuber <thomas.pointhuber@gmx.at>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 * or you may search the http://www.gnu.org website for the version 2 license,
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#ifndef ALTIUM_PARSER_SCH_H
#define ALTIUM_PARSER_SCH_H

#include <cstdint>
#include <cstring>
#include <map>
#include <vector>

// this constant specifies a item which is not inside an component
const int ALTIUM_COMPONENT_NONE = -1;


enum class ALTIUM_SCH_RECORD
{
    HEADER              = 0,
    COMPONENT           = 1,
    PIN                 = 2,
    IEEE_SYMBOL         = 3,
    LABEL               = 4,
    BEZIER              = 5,
    POLYLINE            = 6,
    POLYGON             = 7,
    ELLIPSE             = 8,
    PIECHART            = 9,
    ROUND_RECTANGLE     = 10,
    ELLIPTICAL_ARC      = 11,
    ARC                 = 12,
    LINE                = 13,
    RECTANGLE           = 14,
    SHEET_SYMBOL        = 15,
    SHEET_ENTRY         = 16,
    POWER_PORT          = 17,
    PORT                = 18,
    NO_ERC              = 22,
    NET_LABEL           = 25,
    BUS                 = 26,
    WIRE                = 27,
    TEXT_FRAME          = 28,
    JUNCTION            = 29,
    IMAGE               = 30,
    SHEET               = 31,
    SHEET_NAME          = 32,
    FILE_NAME           = 33,
    DESIGNATOR          = 34,
    BUS_ENTRY           = 37,
    TEMPLATE            = 39,
    PARAMETER           = 41,
    WARNING_SIGN        = 43,
    IMPLEMENTATION_LIST = 44,
    IMPLEMENTATION      = 45,
    RECORD_46           = 46,
    RECORD_47           = 47,
    RECORD_48           = 48,
    RECORD_215          = 215,
    RECORD_216          = 216,
    RECORD_217          = 217,
    RECORD_218          = 218,
    RECORD_226          = 226,
};


struct ASCH_COMPONENT
{
    int      currentpartid;
    wxString libreference;

    int     orientation;
    wxPoint location;

    explicit ASCH_COMPONENT( const std::map<wxString, wxString>& aProperties );
};


struct ASCH_PIN
{
    int ownerindex;
    int ownerpartid;

    wxString name;
    wxString text;
    wxString designator;

    int     orientation;
    wxPoint location;

    explicit ASCH_PIN( const std::map<wxString, wxString>& aProperties );
};


struct ASCH_RECTANGLE
{
    int ownerindex;
    int ownerpartid;

    wxPoint bottomLeft;
    wxPoint topRight;

    int  lineWidth;
    bool isSolid;
    bool isTransparent;

    explicit ASCH_RECTANGLE( const std::map<wxString, wxString>& aProperties );
};


struct ASCH_NET_LABEL
{
    wxString text;
    int      orientation;
    wxPoint  location;

    explicit ASCH_NET_LABEL( const std::map<wxString, wxString>& aProperties );
};


struct ASCH_BUS
{
    int indexinsheet;
    int linewidth;

    std::vector<wxPoint> points;

    explicit ASCH_BUS( const std::map<wxString, wxString>& aProperties );
};


struct ASCH_WIRE
{
    int indexinsheet;
    int linewidth;

    std::vector<wxPoint> points;

    explicit ASCH_WIRE( const std::map<wxString, wxString>& aProperties );
};


struct ASCH_DESIGNATOR
{
    int ownerindex;
    int ownerpartid;

    wxString name;
    wxString text;

    int     orientation;
    wxPoint location;

    explicit ASCH_DESIGNATOR( const std::map<wxString, wxString>& aProperties );
};

#endif //ALTIUM_PARSER_SCH_H