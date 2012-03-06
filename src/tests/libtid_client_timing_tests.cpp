
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

#include <fstream>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>


#include "UnitTest++/UnitTest++.h"

#include "tobiid/IDMessage.hpp"
#include "tobiid/IDSerializerRapid.hpp"

#include "timed_tid_client.h"
#include "libtid/tid_server.h"
#include "libtid/tid_exceptions.h"

#include "tid_message_vector_builder.h"
#include "statistics.h"
#include "timing_test_helpers.h"

using std::fstream;

extern unsigned int NR_TID_MESSAGES;
extern boost::posix_time::milliseconds SLEEP_TIME_BETWEEN_MSGS;

//-------------------------------------------------------------------------------------------------

TEST(libTiDClientSendTiming)
{
  std::cout << "Running libTiD client-send timing test" << std::endl;

  TiD::TimedTiDClient client;
  TiDMessageVectorBuilder msg_builder;
  tobiss::Statistics  stat(true, 100);

  std::fstream file_stream;
  file_stream.precision(8);
  std::fstream summary_file_stream;
  summary_file_stream.precision(12);
  std::string filename;

  std::vector<unsigned int> description_str_lengths;
  description_str_lengths.push_back(5);
  description_str_lengths.push_back(10);
  description_str_lengths.push_back(20);
  description_str_lengths.push_back(50);
  description_str_lengths.push_back(100);

  filename = "libtid_send_client-" + boost::lexical_cast<std::string>(NR_TID_MESSAGES) +"-reps_summary.txt";
  summary_file_stream.open(filename.c_str(), fstream::in | fstream::out | fstream::trunc);
  summary_file_stream << "All values are in microseconds:" << std::endl << std::endl;


  for(unsigned int k= 0; k < description_str_lengths.size(); k++ )
  {
    std::cout << "  ... iteration " << k+1 << " from " << description_str_lengths.size() << std::endl;

    msg_builder.generateMsgsVector(NR_TID_MESSAGES, description_str_lengths[k]);
    std::vector<IDMessage>& msgs_vec = msg_builder.getMessagesVector();

    stat.reset();
    filename = "libtid_send_client_desc_len_" + boost::lexical_cast<std::string>(description_str_lengths[k])
        + "nr_reps_" + boost::lexical_cast<std::string>(msgs_vec.size()) +".csv";

    file_stream.open(filename.c_str(), fstream::in | fstream::out | fstream::trunc);

    for(unsigned int n = 0; n < msgs_vec.size(); n++ )
    {
      stat.update( client.sendTimedMessage( msgs_vec[n] ) );
      TiDHelpers::updateFileStream(file_stream, stat);
    }
    file_stream.unget();
    file_stream << " ";
    file_stream.close();

    summary_file_stream << "Desc-len: "<< boost::lexical_cast<std::string>(description_str_lengths[k]) << std::endl << std::endl;
    stat.printAll(summary_file_stream);
    summary_file_stream << std::endl << std::endl;
  }

  summary_file_stream.close();

  std::cout << std::endl << std::endl;
}

//-------------------------------------------------------------------------------------------------

TEST(libTiDClientReceiveTiming)
{
  std::cout << "Running libTiD client-receive timing test" << std::endl;

  TiDMessageVectorBuilder msg_builder;
  tobiss::Statistics  stat(true, 100);
  std::fstream file_stream;
  file_stream.precision(8);
  std::fstream summary_file_stream;
  summary_file_stream.precision(12);
  std::string filename;

  std::vector<unsigned int> description_str_lengths;
  description_str_lengths.push_back(5);
  description_str_lengths.push_back(10);
  description_str_lengths.push_back(20);
  description_str_lengths.push_back(50);
  description_str_lengths.push_back(100);

  try
  {
    TiD::TiDServer test_server;
    test_server.bind (9001);
    test_server.start();

    TiD::TiDClient send_client;
    send_client.connect("127.0.0.1",9001);

    TiD::TimedTiDClient recv_client;
    recv_client.connect("127.0.0.1",9001);
    recv_client.startReceiving(0);
    boost::this_thread::sleep(boost::posix_time::milliseconds(10));

    filename = "libtid_recv_client-" + boost::lexical_cast<std::string>(NR_TID_MESSAGES) +"-reps_summary.txt";
    summary_file_stream.open(filename.c_str(), fstream::in | fstream::out | fstream::trunc);
    summary_file_stream << "All values are in microseconds:" << std::endl << std::endl;

    for(unsigned int k= 0; k < description_str_lengths.size(); k++ )
    {
      std::cout << "  ... iteration " << k+1 << " from " << description_str_lengths.size() << std::endl;
      msg_builder.generateMsgsVector(NR_TID_MESSAGES, description_str_lengths[k]);
      std::vector<IDMessage>& msgs_vec = msg_builder.getMessagesVector();

      stat.reset();
      recv_client.clearRecvTimingValues();
      boost::this_thread::sleep(boost::posix_time::milliseconds(10));

      filename = "libtid_recv_client_desc_len_" + boost::lexical_cast<std::string>(description_str_lengths[k])
          + "nr_reps_" + boost::lexical_cast<std::string>(msgs_vec.size()) +".csv";

      file_stream.open(filename.c_str(), fstream::in | fstream::out | fstream::trunc);

      for(unsigned int n = 0; n < msgs_vec.size(); n++ )
      {
        send_client.sendMessage(msgs_vec[n]);
        boost::this_thread::sleep(SLEEP_TIME_BETWEEN_MSGS);
      }

      std::vector<double>& diffs = recv_client.getRecvDiffValues();

      for(unsigned int n = 0; n < diffs.size(); n++)
      {
        stat.update( diffs[n] );
        TiDHelpers::updateFileStream(file_stream, stat);
      }

      file_stream.unget();
      file_stream << " ";
      file_stream.close();

      summary_file_stream << "Desc-len: "<< boost::lexical_cast<std::string>(description_str_lengths[k]) << std::endl << std::endl;
      stat.printAll(summary_file_stream);
      summary_file_stream << std::endl << std::endl;

      boost::this_thread::sleep(boost::posix_time::milliseconds(10));
    }

    recv_client.stopReceiving();

    recv_client.disconnect();
    send_client.disconnect();
    boost::this_thread::sleep(boost::posix_time::milliseconds(10));
    test_server.stop();

    boost::this_thread::sleep(boost::posix_time::milliseconds(10));
  }
  catch(std::exception& e)
  {
    std::cerr << "Exception caught: " << e.what() << std::endl;
  }

  summary_file_stream.close();
  std::cout << std::endl << std::endl;
}

//-------------------------------------------------------------------------------------------------