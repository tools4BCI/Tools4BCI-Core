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

/**
* @file statistics.cpp
**/

#include "statistics.h"

#include <boost/current_function.hpp>
#include <functional>
#include <algorithm>
#include <numeric>
#include <math.h>
#include <vector>
#include <limits>

namespace tobiss
{

  using std::for_each;

//-----------------------------------------------------------------------------

Statistics::Statistics(bool buffer_data, size_t window_size)
  : update_coefficient_(0.01), buffer_data_(buffer_data), window_size_(window_size),
    win_buf_is_sorted_(0), sample_buf_is_sorted_(0)
{
  if(buffer_data && !window_size)
  {
    throw(std::invalid_argument("Statistics -- Window size has to be at least of size one!"));
  }

  window_buffer_.set_capacity( window_size );
  window_buffer_sorted_.set_capacity( window_size );

//  sample_buffer_.resize(window_size_ * 1000);
  reset( );
}

//-----------------------------------------------------------------------------

Statistics::~Statistics( )
{
}

//-----------------------------------------------------------------------------

void Statistics::update( double x )
{
  if(buffer_data_)
  {
    window_buffer_.push_back( x );
    sample_buffer_.push_back( x );
    win_buf_is_sorted_ = false;
    sample_buf_is_sorted_ = false;
  }

  N++;

  min_ = std::min(min_, x);
  max_ = std::max(max_, x);

  mean_ = ( (N-1)*mean_ + x ) / N;

  adaptive_mean_ = adaptive_mean_ * (1-update_coefficient_) + x * update_coefficient_;
  adaptive_var_ = adaptive_var_ * (1-update_coefficient_)
                  + update_coefficient_ * ( x - adaptive_mean_ ) * ( x - adaptive_mean_ );
}

//-----------------------------------------------------------------------------

void Statistics::printAll( std::ostream &out )
{
  out << "   nr of values: " << N << std::endl;
  out << "           mean: " << get_mean( ) << std::endl;
  out << "            min: " << get_min( ) << std::endl;
  out << "            max: " << get_max( ) << std::endl;

  if(buffer_data_)
  {
    out << "         median: " << get_median() << std::endl;
    out << "            std: " << sqrt(get_var()) << std::endl;
  }

  out << "  adaptive mean: " << get_adaptive_mean() << std::endl;
  out << "  adaptive  std: " << sqrt(get_adaptive_var()) << std::endl;

  if(buffer_data_)
  {
    out << "    window size: " << window_size_ << std::endl;
    out << "    window mean: " << get_window_mean( ) << std::endl;
    out << "  window median: " << get_window_median( ) << std::endl;
    out << "     window min: " << get_window_min( ) << std::endl;
    out << "     window max: " << get_window_max( ) << std::endl;
    out << "     window std: " << sqrt(get_window_mean() ) << std::endl;
  }
}

//-----------------------------------------------------------------------------

void Statistics::printWindowStatistics( std::ostream &out )
{
  if(buffer_data_)
  {
    out << "    window size: " << window_size_ << std::endl;
    out << "    window mean: " << get_window_mean( ) << std::endl;
    out << "  window median: " << get_window_median( ) << std::endl;
    out << "     window min: " << get_window_min( ) << std::endl;
    out << "     window max: " << get_window_max( ) << std::endl;
    out << "     window std: " << sqrt(get_window_mean() ) << std::endl;
  }
}

//-----------------------------------------------------------------------------

void Statistics::printSampleValues( std::ostream &out , char separator)
{
  if(buffer_data_)
  {
    std::list<double>::const_iterator it(sample_buffer_.begin());;

    for( ; it != sample_buffer_.end(); std::advance(it, 1) )
      out << *it << separator;
  }
}

//-----------------------------------------------------------------------------

void Statistics::printFastStatistics( std::ostream &out )
{
  out << "           mean: " << get_mean( ) << std::endl;
  out << "           min: "  << get_min( ) << std::endl;
  out << "           max: "  << get_max( ) << std::endl;
  out << "  adaptive mean: " << get_adaptive_mean() << std::endl;
  out << "  adaptive  std: " << sqrt(get_adaptive_var()) << std::endl;
}

//-----------------------------------------------------------------------------

void Statistics::setUpdateCoefficient(double coeff)
{
  reset();
  update_coefficient_ = coeff;
}

//-----------------------------------------------------------------------------

void Statistics::reset( )
{
  window_buffer_.clear( );
  window_buffer_sorted_.clear();
  sample_buffer_.clear();

  N = 0;
  mean_ = 0;
  var_  = 0;
  min_  = std::numeric_limits<double>::infinity();
  max_  = 0;
  adaptive_mean_ = 0;
  adaptive_var_ = 0;
}

//-----------------------------------------------------------------------------

void Statistics::getSamples( size_t buffersize, std::list<double>::iterator begin, std::list<double>::iterator end  )
{
  end   = sample_buffer_.end();
  begin = sample_buffer_.end();
  for(size_t n = 0; n < buffersize; n++)
    --begin;
}


//-----------------------------------------------------------------------------

double Statistics::get_window_median()
{
  if(!buffer_data_)
    return(std::numeric_limits<double>::quiet_NaN());

  if( window_buffer_.size() == 0 )
  {
    std::cerr << "Error -- " << BOOST_CURRENT_FUNCTION << " -- No samples in the window buffer!"  << std::endl;
    return(0);
  }

  sort_win_buffer();
  return(window_buffer_sorted_[ window_buffer_sorted_.size()/2 ]);
}

//-----------------------------------------------------------------------------

double Statistics::get_window_mean()
{
  if(!buffer_data_)
    return(std::numeric_limits<double>::quiet_NaN());

  if( window_buffer_.size() == 0 )
  {
    std::cerr << "Error -- " << BOOST_CURRENT_FUNCTION << " -- No samples in the window buffer!"  << std::endl;
    return(0);
  }

  return(std::accumulate(window_buffer_.begin(),window_buffer_.end(),0.0)/double(window_buffer_.size()));
}

//-----------------------------------------------------------------------------

double Statistics::get_window_min()
{
  if(!buffer_data_)
    return(std::numeric_limits<double>::quiet_NaN());

  if( window_buffer_.size() == 0 )
  {
    std::cerr << "Error -- " << BOOST_CURRENT_FUNCTION << " -- No samples in the window buffer!"  << std::endl;
    return(0);
  }

  sort_win_buffer();
  return( window_buffer_sorted_.front() );
}

//-----------------------------------------------------------------------------

double Statistics::get_window_max()
{
  if(!buffer_data_)
    return(std::numeric_limits<double>::quiet_NaN());

  if( window_buffer_.size() == 0 )
  {
    std::cerr << "Error -- " << BOOST_CURRENT_FUNCTION << " -- No samples in the window buffer!"  << std::endl;
    return(0);
  }

  sort_win_buffer();
  return( window_buffer_sorted_.back() );
}

//-----------------------------------------------------------------------------

double Statistics::get_window_var()
{
  if(!buffer_data_)
    return(std::numeric_limits<double>::quiet_NaN());

  if( window_buffer_.size() == 0 )
  {
    std::cerr << "Error -- " << BOOST_CURRENT_FUNCTION << " -- No samples in the window buffer!"  << std::endl;
    return(0);
  }

  std::vector<double> zero_mean( window_buffer_.begin(), window_buffer_.end() );
  transform( zero_mean.begin(), zero_mean.end(), zero_mean.begin(),bind2nd( std::minus<double>(), get_window_mean() ) );

  return(inner_product( zero_mean.begin(),zero_mean.end(), zero_mean.begin(), 0.0 )/ double( window_buffer_.size() - 1.0 ));
}

//-----------------------------------------------------------------------------

void Statistics::sort_win_buffer()
{
  if(!win_buf_is_sorted_)
  {
    window_buffer_sorted_ = window_buffer_;
    sort(window_buffer_sorted_.begin(), window_buffer_sorted_.end());
    win_buf_is_sorted_ = true;
  }
}

//-----------------------------------------------------------------------------

void Statistics::sort_sample_buffer()
{
  if(!sample_buf_is_sorted_)
  {
    sample_buffer_.sort();
    sample_buf_is_sorted_ = true;
  }
}

//-----------------------------------------------------------------------------

double Statistics::get_median()
{
  if(!buffer_data_)
    return(std::numeric_limits<double>::quiet_NaN());

  if( sample_buffer_.size() == 0 )
  {
    std::cerr << "Error -- " << BOOST_CURRENT_FUNCTION << " -- No samples in the sample buffer!"  << std::endl;
    return(0);
  }

  sort_sample_buffer();

  std::list<double>::iterator it(sample_buffer_.begin());
  size_t n = sample_buffer_.size()/2;
  std::advance(it, n);
  return(*it);

  //  for(size_t n = 0; n < sample_buffer_.size()/2; n++)
  //    it++;

}

//-----------------------------------------------------------------------------

double Statistics::get_var()
{
  if(!buffer_data_)
    return(std::numeric_limits<double>::quiet_NaN());

  if( sample_buffer_.size() == 0 )
  {
    std::cerr << "Error -- " << BOOST_CURRENT_FUNCTION << " -- No samples in the sample buffer!"  << std::endl;
    return(0);
  }

  std::vector<double> zero_mean( sample_buffer_.begin(), sample_buffer_.end() );
  transform( zero_mean.begin(), zero_mean.end(), zero_mean.begin(),bind2nd( std::minus<double>(), mean_ ) );

  return(inner_product( zero_mean.begin(),zero_mean.end(), zero_mean.begin(), 0.0 )/ ( sample_buffer_.size() - 1 ));
}

//-----------------------------------------------------------------------------

}
