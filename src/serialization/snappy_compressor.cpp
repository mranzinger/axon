#include "serialization/format/snappy_compressor.h"

#include <snappy.h>
#include <memory>
#include <stdexcept>

using namespace std;

namespace axon { namespace serialization {

void CSnappyCompressor::Compress(const char *a_input, size_t a_inputSize, 
                                 unique_ptr<char[]> &a_output, size_t &a_outputSize) const 
{
    a_output.reset(new char[snappy::MaxCompressedLength(a_inputSize)]);                            

    snappy::RawCompress(a_input, a_inputSize, a_output.get(), &a_outputSize);
}

unique_ptr<char[]> CSnappyCompressor::Decompress(const char *a_compressed, size_t a_compressedSize, 
                                               size_t a_decompressedSize) const 
{
    unique_ptr<char[]> l_ret(new char[a_decompressedSize]);

    if (!snappy::RawUncompress(a_compressed, a_compressedSize, l_ret.get()))
        throw runtime_error("The specified compressed data is corrupted.");

    return move(l_ret);
}

} }
