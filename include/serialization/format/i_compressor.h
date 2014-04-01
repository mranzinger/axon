#ifndef I_COMPRESSOR_H_
#define I_COMPRESSOR_H_

#include <memory>

#include "../dll_export.h"

namespace axon { namespace serialization {

class AXON_SERIALIZE_API ICompressor
{
public:
    typedef std::shared_ptr<ICompressor> Ptr;

    virtual ~ICompressor() { }

    static const Ptr DEFAULT_INSTANCE;

    virtual void Compress(const char *a_input, size_t a_inputSIze, 
                          std::unique_ptr<char[]> &a_output, size_t &a_outputSize) const = 0;
    virtual std::unique_ptr<char[]> Decompress(const char *a_compressed, size_t a_compressedSize, 
                             size_t a_decompressedSize) const = 0;
};

} }

#endif
