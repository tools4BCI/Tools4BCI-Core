/*
    This file is part of TOBI Interface A (TiA).

    TOBI Interface A (TiA) is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    TOBI Interface A (TiA) is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with TOBI Interface A (TiA).  If not, see <http://www.gnu.org/licenses/>.

    Copyright 2010 Christian Breitwieser
    Contact: c.breitwieser@tugraz.at
*/

/**
* @file tcp_server.cpp
*
* @brief \TODO.
*
**/

// STL
#include <sstream>

// Boost
#include <boost/bind.hpp>

// local
#include "tcp_server.h"

namespace TiD
{
using boost::uint16_t;
using boost::uint32_t;

//-----------------------------------------------------------------------------

TCPServer::TCPServer(boost::asio::io_service& io_service)
  : io_service_(io_service), acceptor_(io_service)
{
  #ifdef DEBUG
    std::cout << "TCPConnection: Constructor" << std::endl;
  #endif
}

//-----------------------------------------------------------------------------

TCPServer::~TCPServer()
{
  #ifdef DEBUG
    std::cout << "TCPConnection: ~TCPServer" << std::endl;
  #endif
}

//-----------------------------------------------------------------------------

void TCPServer::bind(uint16_t port)
{  
  #ifdef DEBUG
    std::cout << "TCPServer: bind(uint16_t port)" << std::endl;
  #endif

  boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), port);
  acceptor_.open(endpoint.protocol());
  acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
  acceptor_.bind(endpoint);
}

//-----------------------------------------------------------------------------

void TCPServer::bind(const std::string& address, uint16_t port)
{
  #ifdef DEBUG
    std::cout << "TCPServer: bind(const std::string& address, uint16_t port)" << std::endl;
  #endif

  // Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
  boost::asio::ip::tcp::resolver resolver(io_service_);
  std::ostringstream ss;
  ss << port;
  boost::asio::ip::tcp::resolver::query query(address, ss.str());
  boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);
  acceptor_.open(endpoint.protocol());
  acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
  acceptor_.bind(endpoint);
}

//-----------------------------------------------------------------------------

void TCPServer::listen()
{
  #ifdef DEBUG
    std::cout << "TCPServer: listen" << std::endl;
  #endif

  acceptor_.listen();
  startAccept();
}

//-----------------------------------------------------------------------------

void TCPServer::startAccept()
{
  #ifdef DEBUG
    std::cout << "TCPServer: startAccept" << std::endl;
  #endif

  TCPConnection::pointer new_connection = TCPConnection::create(io_service_);

  acceptor_.async_accept(new_connection->socket(), boost::bind(
      &TCPServer::handleAccept, this, new_connection,
      boost::asio::placeholders::error));
}

//-----------------------------------------------------------------------------

} //Namespace tobiss

// End Of File
