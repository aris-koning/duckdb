//===----------------------------------------------------------------------===//
//                         DuckDB
//
// storage/data_block.hpp
//
//
//===----------------------------------------------------------------------===//

#include "common/common.hpp"
#include "common/constants.hpp"
#include "common/file_system.hpp"
#include "common/fstream_util.hpp"
#include "common/types/data_chunk.hpp"
#include "common/types/null_value.hpp"
#include "common/types/vector.hpp"
#include "common/vector_operations/vector_operations.hpp"

#include <stddef.h>
#include <string>

namespace duckdb {

class DataTable;
//! Block Header Size = 48 bytes
//! Max Block Size = 2^16 Bytes =  64kb
constexpr const size_t max_block_size = 16384; // 16KB
//! Representation of a Page (content stored within a file). Each page holds data from multiuple columns in a PAX way
//! (https://dl.acm.org/citation.cfm?id=641271).  The Block stored in a data block Stores the header of each data block
struct DataBlockHeader {
	//! Id used to identify the block
	size_t block_id;
	//! Data size in bytes
	size_t data_size;
	//! Amount of tuples within the block
	size_t amount_of_tuples;
	//! The offset for each column data
	vector<size_t> data_offset;
};

//! The DataBlock is the physical unit to store data. It has a physical block which is stored in a file with multiple
//! blocks
class DataBlock {
public:
	//! This class constructs the Data Block
	class Builder;

private:
	DataBlockHeader header;
	//! The data of the block-> multiple columns (each column has the same type).
	unique_ptr<char[]> data_buffer;

	//! Only one simple constructor - rest is handled by Builder
	DataBlock(const DataBlockHeader _header) {
		data_buffer = unique_ptr<char[]>(new char[max_block_size]);
		header = _header;
		offset = sizeof(DataBlockHeader);
	};

public:
	void Append(DataChunk &chunk);
	void FlushOnDisk(string &path_to_file, size_t block_id);
	bool is_full = false;
	size_t offset;
};

//! The intent of the Builder design pattern is to separate the construction of a complex object from its representation
class DataBlock::Builder {
private:
	DataBlockHeader header;
	size_t block_count = 1;

public:
	//! Produces a new DataBlock
	DataBlock Build(const size_t tuple_size);
	size_t GetBlockCount() const {
		return block_count;
	}
};

} // namespace duckdb
