#ifndef SNAPPY_COMPRESSOR_H_
#define SNAPPY_COMPRESSOR_H_

#include "i_compressor.h"

namespace axon { namespace serialization {

class AXON_SERIALIZE_API CSnappyCompressor
    : public ICompressor
{
public:
    virtual void Compress(const char *a_input, size_t a_inputSize, 
                          std::unique_ptr<char[]> &a_output, size_t &a_outputSize) const override;
    virtual std::unique_ptr<char[]> Decompress(const char *a_compressed, size_t a_compressedSize, 
                             size_t a_decompressedSize) const override;
};

} }

#endif
