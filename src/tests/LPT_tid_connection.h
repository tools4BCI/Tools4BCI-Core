/*
    This file is part of the libTiD test routine.

    The libTiD test routine is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    The libTiD test routine is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with the libTiD test routine. If not, see <http://www.gnu.org/licenses/>.

    Copyright 2012 Christian Breitwieser
    Contact: c.breitwieser@tugraz.at
*/

#ifndef LPT_TID_CONNECTION_H
#define LPT_TID_CONNECTION_H


#include "libtid/tcp_connection.h"
#include "libtid/tid_connection.h"
#include "timed_input_stream_socket.h"
#include "libtid/messages/tid_message_parser_1_0.h"

extern unsigned int NR_TID_MESSAGES;

namespace TiD
{

class LPTTiDConnection : public TiDConnection
{
  public:

    typedef boost::shared_ptr<TiDConnection> pointer;

    virtual ~LPTTiDConnection()
    {
      #ifdef DEBUG
        std::cout << BOOST_CURRENT_FUNCTION <<  std::endl;
      #endif
    }

  static pointer create(const TCPConnection::pointer& tcp_conn_handle,
                        deleteConnectionCallback del_con_cb, dispatchTiDMessageCallback disp_msg_cb)
  {
    return pointer(new LPTTiDConnection(tcp_conn_handle, del_con_cb, disp_msg_cb));
  }


  private:
    LPTTiDConnection(const TCPConnection::pointer& tcp_conn_handle,
                       deleteConnectionCallback del_con_cb, dispatchTiDMessageCallback disp_msg_cb)
      : TiDConnection(tcp_conn_handle, del_con_cb, disp_msg_cb)
    {
      #ifdef DEBUG
        std::cout << BOOST_CURRENT_FUNCTION <<  std::endl;
      #endif
    }


    virtual void receive()
    {
      #ifdef DEBUG
        std::cout << BOOST_CURRENT_FUNCTION <<  std::endl;
      #endif

      while(state_ == State_Running)
      {
        try
        {
          msg_parser_->parseMessage(&msg_, input_stream_ );
          disp_tid_msg_callback_ref_( msg_ , connection_id_);
        }
        catch(TiDLostConnection&)
        {
          close();
          break;
        }
        catch(TiDException& e)
        {
          if(state_ == State_Running)
            std::cerr << e.what() << std::endl << ">> ";
          close();
          break;
        }
        catch(...)
        {
          std::cerr << "Caught unknown exception during message parsing!" << std::endl;
          throw;
        }

      }
  }

  private:


};

}

#endif // TIMED_TID_CONNECTION_H
