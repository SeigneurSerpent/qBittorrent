/***************************************************************************
*   Copyright (C) 2005-09 by the Quassel Project                          *
*   devel@quassel-irc.org                                                 *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) version 3.                                           *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/

#ifndef STACKTRACE_WIN_H
#define STACKTRACE_WIN_H

#include <Windows.h>
#include <QString>

namespace straceWin
{
    const QString getBacktrace(HANDLE hProcess, HANDLE hThread);

    QString GetLastErrorString();
    QString GetErrorString(DWORD errNum);

    template<typename D>
    struct resource_guard
    {
        resource_guard(const resource_guard&) = delete;
        resource_guard& operator=(const resource_guard&) = delete;
        
        resource_guard(D&& d)
            : deleter{ std::move(d) }
        {}

        resource_guard(resource_guard&& g)
            : deleter{ std::move(g.deleter) },
            freeResource(false)
        {}

        ~resource_guard()
        {
            if (freeResource)
                this->deleter();
        }
    private:
        D deleter;
        bool freeResource = true;
    };

    template<typename D>
    resource_guard<std::remove_reference_t<D>> make_resource_guard(D&& d)
    {
        return resource_guard<std::remove_reference_t<D>>(std::forward<D>(d));
    }
}

#endif // STACKTRACE_WIN_H
