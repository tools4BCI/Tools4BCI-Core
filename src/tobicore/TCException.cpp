/*
    Copyright (C) 2009-2011  EPFL (Ecole Polytechnique Fédérale de Lausanne)
    Michele Tavella <michele.tavella@epfl.ch>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as
    published by the Free Software Foundation, either version 3 of
    the License, or (at your option) any later version.

    It is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this file.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef TCEXCEPTION_CPP
#define TCEXCEPTION_CPP

#include "TCException.hpp"

TCException::TCException(std::string info, std::string caller) {
  this->_info = info;
  this->_caller = caller;
}

TCException::~TCException(void) {
}

std::string TCException::GetCaller(void) const {
  return this->_caller;
}

std::string TCException::GetInfo(void) const {
  return this->_info;
}

std::ostream &operator<<(std::ostream& output, const TCException& obj) {
  output << std::string("[") << obj.GetCaller() << std::string("] ")
    << obj.GetInfo();
  return output;
}

std::ostream &operator<<(std::ostream& output, const TCException* obj) {
  output << std::string("[") << obj->GetCaller() << std::string("] ")
    << obj->GetInfo();
  return output;
}

bool TCException::operator==(const TCException& right) {
  return(this->_info.compare(right.GetInfo()) == 0);
}

bool TCException::operator!=(const TCException& right) {
  return(this->_info.compare(right.GetInfo()) != 0);
}

#endif
