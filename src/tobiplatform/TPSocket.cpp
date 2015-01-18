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

  TPSocket.hpp/.cpp is adapted from libtransport
*/

#ifndef TPSOCKET_CPP
#define TPSOCKET_CPP

#include "TPSocket.hpp"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#ifdef _WIN32
#include <stdio.h>
#endif

TPHost::TPHost(void) {
  memset(this->address, 0, TPHost::AddressSize);
  this->port = 0;
}

#ifdef _WIN32
bool TPSocket::_wsaInitialized = false;
#endif

TPSocket::TPSocket(int type, size_t bsize) {
  this->_type = type;
  this->_bsize = bsize;
  this->_mc = 256;
  this->_buffer = NULL;
  this->_fd = -1;
}

TPSocket::~TPSocket(void) {
}

bool TPSocket::Async(bool block) {
#ifndef __MINGW32__
  int oldflags = fcntl(this->_fd, F_GETFL, 0);
  if(oldflags < 0)
    return false;

  if(block == true)
    oldflags |= O_NONBLOCK;
  else
    oldflags &= ~O_NONBLOCK;
  return(fcntl(this->_fd, F_SETFL, oldflags) > -1);
#else
  u_long arg = (u_long)block;
  ioctlsocket(this->_fd, FIONBIO, &arg);
  return true;
#endif
}

void TPSocket::Init(void) {
  this->_fd = 0;
  this->_mc = 0;
  this->_bsizemax = 0;

  this->Free();
  this->_buffer = malloc(this->_bsize);
  memset(this->_buffer, 0, this->_bsize);

  memset(&(this->_address), 0, sizeof(this->_address));
  memset(&(this->_results), 0, sizeof(this->_results));
  memset(&(this->_endpoint), 0, sizeof(this->_endpoint));

  this->_bsizemax = 0;
#ifdef _WIN32
  /* Stays here for the time being, later on I will handle
   * the (doggie)initialization better.
   */
  TPSocket::InitializeWSA();
#endif
}

void TPSocket::Free(void) {
  if(this->_buffer == NULL)
    return;
  free(this->_buffer);
  this->_buffer = NULL;
}

bool TPSocket::GetLocal(void) {
#ifndef _WIN32
  const char* status = NULL;
  struct sockaddr addr;
  int addrlen = sizeof(addr);

  getsockname(this->_fd,
      (struct sockaddr*)&addr, (socklen_t*)&addrlen);
  struct sockaddr_in *addr_ptr = (struct sockaddr_in*)&addr;

  this->local.port = ntohs(addr_ptr->sin_port);
  status = inet_ntop(AF_INET,
      &(addr_ptr->sin_addr.s_addr), this->local.address,
      (socklen_t)addrlen);
  return(status != NULL);
#else
  return false;
#endif
}

bool TPSocket::GetRemote(void) {
#ifndef _WIN32
  const char* status = NULL;
  struct sockaddr addr;
  int addrlen = sizeof(addr);

  getpeername(this->_fd,
      (struct sockaddr*)&addr, (socklen_t*)&addrlen);
  struct sockaddr_in *addr_ptr = (struct sockaddr_in*)&addr;

  this->remote.port = ntohs(addr_ptr->sin_port);
  status = inet_ntop(AF_INET, &(addr_ptr->sin_addr.s_addr),
      this->remote.address, (socklen_t)addrlen);
  return(status != NULL);
#else
  return false;
#endif
}

void TPSocket::GetMaxBSize(void) {
  socklen_t optlen = sizeof(this->_bsizemax);
#ifndef _WIN32
  if(getsockopt(this->_fd, SOL_SOCKET, SO_SNDBUF, &this->_bsizemax,
        &optlen) < 0)
    this->_bsizemax = 0;
#else
  this->_bsizemax = 0;
#endif
}

bool TPSocket::Open(bool asserver) {
  switch(this->_type) {
    case TPSocket::TCP:
      this->_results.ai_socktype = SOCK_STREAM;
      break;
    case TPSocket::UDP:
      this->_results.ai_socktype = SOCK_DGRAM;
      break;
    default:
      return false;
  }
  if(asserver == true)
    this->_results.ai_flags = AI_PASSIVE;
  this->_results.ai_family = AF_UNSPEC;

  this->Init();
  return true;
}

bool TPSocket::Close(void) {
  return(close(this->_fd) == 0);
}

bool TPSocket::Bind(const std::string& ip, const std::string& port) {
  int retopt = 0;
  int bndret = 0;
  struct addrinfo *ai;
  int opt = 1;

  if(getaddrinfo(ip.c_str(), port.c_str(), &(this->_results), &ai) != 0)
    return -1;

  for(this->_info = ai; this->_info != NULL;
      this->_info = this->_info->ai_next) {
    this->_fd = socket(this->_info->ai_family,
        this->_info->ai_socktype,
        this->_info->ai_protocol);
    if(this->_fd == -1)
      continue;

#ifndef _WIN32
    if(this->_type == TPSocket::TCP) {
      retopt = setsockopt(this->_fd,
          SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));
      if(retopt == -1)
        return false;
    }
#endif
    bndret = bind(this->_fd, this->_info->ai_addr, this->_info->ai_addrlen);
    if(bndret == -1) {
      close(this->_fd);
      continue;
    }
    break;
  }
  freeaddrinfo(ai);

  this->GetLocal();

  return(bndret == 0);
}

bool TPSocket::Listen(void) {
  return(listen(this->_fd, this->_mc) == 0);
}

int TPSocket::Accept(TPSocket* endpoint) {
  if(endpoint->_type != this->_type)
    return -3;
  endpoint->Init();

  unsigned int addrlen = sizeof(endpoint->_address);

  /*
  endpoint->_fd = accept(this->_fd, (struct sockaddr*)&this->_endpoint,
      &addrlen);
  */
  endpoint->_fd = accept(this->_fd, (struct sockaddr*)&this->_endpoint,
      (socklen_t*)&addrlen);

  this->GetLocal();
  this->GetRemote();
  return endpoint->_fd;
}

bool TPSocket::Connect(const std::string& ip, const std::string& port) {
  struct addrinfo *ai;
  int conopt = 0;

  if(getaddrinfo(ip.c_str(), port.c_str(), &(this->_results), &ai) != 0)
    return false;

  for(this->_info = ai; this->_info != NULL; this->_info = this->_info->ai_next) {
    this->_fd = socket(this->_info->ai_family,
        this->_info->ai_socktype, this->_info->ai_protocol);
    if(this->_fd == -1)
      continue;

    if(this->_type == TPSocket::TCP) {
      conopt = connect(this->_fd, this->_info->ai_addr,
          this->_info->ai_addrlen);
      if(conopt == -1) {
        this->Close();
        continue;
      }
    }
    break;
  }

  this->GetMaxBSize();
  this->GetLocal();
  this->GetRemote();

  return(conopt == 0);
}

ssize_t TPSocket::Send(const std::string& message) {
  ssize_t bytes = -1;
  switch(this->_type) {
    case TPSocket::TCP:
#ifndef _WIN32
      bytes = send(this->_fd, message.c_str(), message.size(),
          MSG_NOSIGNAL);
#else
      bytes = send(this->_fd, message.c_str(), message.size(), 0);
#endif
      break;
    case TPSocket::UDP:
      return sendto(this->_fd, message.c_str(), message.size(), 0,
          this->_info->ai_addr, this->_info->ai_addrlen);
      break;
  }
  return bytes;
}

ssize_t TPSocket::Recv(std::string* message) {
  ssize_t bytes = -1;
  switch(this->_type) {
    case TPSocket::TCP:
#ifndef _WIN32
      bytes = recv(this->_fd, this->_buffer, this->_bsize, 0);
#else
      bytes = recv(this->_fd, (char*)this->_buffer, this->_bsize, 0);
#endif
      break;
    case TPSocket::UDP:
      int addr_len = sizeof(this->_endpoint);
#ifndef _WIN32
      bytes = recvfrom(this->_fd, this->_buffer, this->_bsize, 0,
          (struct sockaddr *)&this->_endpoint, (socklen_t*)&addr_len);
#else
      bytes = recvfrom(this->_fd, (char*)this->_buffer, this->_bsize, 0,
          (struct sockaddr *)&this->_endpoint, (socklen_t*)&addr_len);
#endif
      break;
  }

  if(bytes > 0)
    message->assign((const char*)this->_buffer, (size_t)bytes);

  return bytes;
}

#ifdef _WIN32
bool TPSocket::InitializeWSA(void) {
  if(TPSocket::_wsaInitialized == true)
    return true;

  WSADATA wsaData;
  TPSocket::_wsaInitialized = (WSAStartup(MAKEWORD(2,2), &wsaData) != 0);
  return TPSocket::_wsaInitialized;
}
#endif

bool TPSocket::IsConnected(void) {
  std::string empty;
  empty.push_back('\0');
  return(this->Send(empty) > (ssize_t)0);
}

#endif
