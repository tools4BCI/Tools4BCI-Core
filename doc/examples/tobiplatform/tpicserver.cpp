/*
    Copyright (C) 2009-2011  EPFL (Ecole Polytechnique Fédérale de Lausanne)
    Michele Tavella <michele.tavella@epfl.ch>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <string>
#include <iostream>
#include <unistd.h>
#include <tobiplatform/TPiC.hpp>
#include <tobiic/ICMessage.hpp>
#include <tobiic/ICSerializerRapid.hpp>
#include <tobicore/TCTime.hpp>

#define ENDLESS

int main(void) {
	ICMessage message;
	ICSerializerRapid serializer(&message);

	TPiC server;
	std::string buffer;

#ifdef ENDLESS
	while(true) {
#endif
		std::cout << "Initializing iC server and waiting for client to plug-in" << std::endl;

		if(server.Plug("127.0.0.1", "8000", TPiC::AsServer) != TPiC::Successful) {
#ifdef ENDLESS
			std::cout << "Cannot plug iC server: trying in 5 seconds" << std::endl;
			TCSleep(5000);
			continue;
#else
			std::cout << "Cannot plug iC server" << std::endl;
			return false;
#endif
		}

		int frame = 1;
		bool first = true;
		while(true) {
			message.SetBlockIdx(++frame);
			int status = server.Get(&serializer);
			if(status == TPiC::Successful) {
				frame = message.GetBlockIdx();
				std::cout << "iC message received: " << frame << std::endl;
				if(first == true) {
					message.Dump();
					first = false;
				}
			}
			else if(status == TPiC::Unsuccessful)
				continue;
			else 
				break;

		}
		std::cout << "iC client is down" << std::endl;
		server.Unplug();
#ifdef ENDLESS
	}
#endif
	return 0;
}
