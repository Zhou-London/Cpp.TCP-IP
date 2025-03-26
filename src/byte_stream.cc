#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity )
    : buffer(""), 
      data_pushed_total(0),
      data_popped_total(0),
      capacity_( capacity ),
      error_(false),
      writer_closed_(false)
      {} 

void Writer::push( string data )
{
  uint64_t data_bytes = data.size();
  uint64_t available = available_capacity();
  if(data_bytes > available){
    data = data.substr(0, available);
    data_bytes = available;
  }

  buffer += data;
  data_pushed_total += data_bytes; 
}

void Writer::close()
{
  writer_closed_ = true;
}

bool Writer::is_closed() const
{
  return writer_closed_; // Your code here.
}

uint64_t Writer::available_capacity() const
{
  return capacity_ - buffer.size(); // Your code here.
}

uint64_t Writer::bytes_pushed() const
{
  return data_pushed_total; // Your code here.
}

string_view Reader::peek() const
{
  return buffer; // Your code here.
}

void Reader::pop( uint64_t len )
{
  uint64_t bsize = buffer.size();
  if (len > bsize) {
    len = bsize;
  }
  buffer = buffer.substr(len);
  data_popped_total += len;
}

bool Reader::is_finished() const
{
  
  return writer_closed_ && buffer.empty(); // Your code here.
}

uint64_t Reader::bytes_buffered() const
{
  return buffer.size(); // Your code here.
}

uint64_t Reader::bytes_popped() const
{
  return data_popped_total; // Your code here.
}

