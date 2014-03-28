#include "serialization/format/i_compressor.h"
#include "serialization/format/snappy_compressor.h"

namespace axon { namespace serialization {

const ICompressor::Ptr ICompressor::DEFAULT_INSTANCE(new CSnappyCompressor());

} }
