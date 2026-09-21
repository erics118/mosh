/*
    Mosh: the mobile shell
    Copyright 2012 Keith Winstein

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

    In addition, as a special exception, the copyright holders give
    permission to link the code of portions of this program with the
    OpenSSL library under certain conditions as described in each
    individual source file, and distribute linked combinations including
    the two.

    You must obey the GNU General Public License in all respects for all
    of the code used other than OpenSSL. If you modify file(s) with this
    exception, you may extend this exception to your version of the
    file(s), but you are not obligated to do so. If you do not wish to do
    so, delete this exception statement from your version. If you delete
    this exception statement from all source files in the program, then
    also delete it here.
*/

#include <zlib.h>

#include "compressor.h"
#include "src/util/dos_assert.h"

using namespace Network;

Compressor::Compressor() : buffer(), deflate_stream(), inflate_stream()
{
  deflate_stream.zalloc = Z_NULL;
  deflate_stream.zfree = Z_NULL;
  deflate_stream.opaque = Z_NULL;
  /* default level/windowBits match zlib's compress(), so output stays wire-compatible */
  dos_assert( deflateInit( &deflate_stream, Z_DEFAULT_COMPRESSION ) == Z_OK );

  inflate_stream.zalloc = Z_NULL;
  inflate_stream.zfree = Z_NULL;
  inflate_stream.opaque = Z_NULL;
  dos_assert( inflateInit( &inflate_stream ) == Z_OK );
}

Compressor::~Compressor()
{
  deflateEnd( &deflate_stream );
  inflateEnd( &inflate_stream );
}

std::string Compressor::compress_str( const std::string& input )
{
  dos_assert( deflateReset( &deflate_stream ) == Z_OK );
  deflate_stream.next_in = reinterpret_cast<Bytef*>( const_cast<char*>( input.data() ) );
  deflate_stream.avail_in = input.size();
  deflate_stream.next_out = buffer;
  deflate_stream.avail_out = BUFFER_SIZE;
  dos_assert( deflate( &deflate_stream, Z_FINISH ) == Z_STREAM_END );
  return std::string( reinterpret_cast<char*>( buffer ), BUFFER_SIZE - deflate_stream.avail_out );
}

std::string Compressor::uncompress_str( const std::string& input )
{
  dos_assert( inflateReset( &inflate_stream ) == Z_OK );
  inflate_stream.next_in = reinterpret_cast<Bytef*>( const_cast<char*>( input.data() ) );
  inflate_stream.avail_in = input.size();
  inflate_stream.next_out = buffer;
  inflate_stream.avail_out = BUFFER_SIZE;
  dos_assert( inflate( &inflate_stream, Z_FINISH ) == Z_STREAM_END );
  return std::string( reinterpret_cast<char*>( buffer ), BUFFER_SIZE - inflate_stream.avail_out );
}

/* construct on first use */
Compressor& Network::get_compressor( void )
{
  static Compressor the_compressor;
  return the_compressor;
}
