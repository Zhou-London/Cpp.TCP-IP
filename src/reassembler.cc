#include "reassembler.hh"
#include "debug.hh"

using namespace std;

void Reassembler::insert(uint64_t first_index, std::string data, bool is_last_substring) {
    if (data.empty() && !is_last_substring) {
        return;
    }

    // 1) Keep the original size of Data
    const size_t original_len = data.size();

    // 2) Cut the Data if necessary
    uint64_t available_capacity = output_.writer().available_capacity();
    uint64_t stream_end = next_byte_index + available_capacity;

    // Still Beyond Capacity after cutting, discard it
    if (first_index >= stream_end) {
        return;
    }
    // Can be cutted
    if (first_index + data.length() > stream_end) {
        data.resize(stream_end - first_index);
    }

    // 3) Update the index of the last byte
    if (is_last_substring) {
        has_last_substring = true;
        last_byte_index = first_index + original_len; 
    }

    // 4) Index is smaller or equal to expected
    if (first_index <= next_byte_index) {
        // Case1: Smaller
        // Overlap the Data
        if (first_index < next_byte_index) {
            uint64_t overlap = next_byte_index - first_index;
            if (overlap >= data.length()) {
                // 检查是否需要关闭流
                if (has_last_substring && data_buffer.empty() && next_byte_index >= last_byte_index) {
                    output_.writer().close();
                }
                return;
            }
            data = data.substr(overlap);
            first_index = next_byte_index;
        }

        output_.writer().push(data);
        next_byte_index += data.length();

        while (!data_buffer.empty()) {
            auto it = data_buffer.begin();
            if (it->first > next_byte_index) {
                break;
            }
            uint64_t buffer_index = it->first;
            std::string buffer_data = std::move(it->second);
            data_buffer.erase(it);
            if (buffer_index < next_byte_index) {
                uint64_t overlap = next_byte_index - buffer_index;
                if (overlap >= buffer_data.length()) {
                    continue;
                }
                buffer_data = buffer_data.substr(overlap);
            }
            output_.writer().push(buffer_data);
            next_byte_index += buffer_data.length();
        }
    } else {
        // first_index > next_byte_index
        auto it = data_buffer.lower_bound(first_index);
        if (it != data_buffer.begin()) {
            auto prev_it = it;
            --prev_it;
            if (prev_it->first + prev_it->second.length() > first_index) {
                uint64_t overlap = prev_it->first + prev_it->second.length() - first_index;
                if (overlap >= data.length()) {
                    // 全覆盖
                    // 检查是否需要关闭流
                    if (has_last_substring && data_buffer.empty() && next_byte_index >= last_byte_index) {
                        output_.writer().close();
                    }
                    return;
                }
                data = data.substr(overlap);
                first_index += overlap;
            }
        }
        while (it != data_buffer.end() && it->first < first_index + data.length()) {
            uint64_t next_end = it->first + it->second.length();
            if (next_end <= first_index + data.length()) {
                it = data_buffer.erase(it);
            } else {
                data.resize(it->first - first_index);
                break;
            }
        }
        if (!data.empty()) {
            data_buffer[first_index] = std::move(data);
        }
    }

    // 5) 再次检查是否可以关闭流
    if (has_last_substring && data_buffer.empty() && next_byte_index >= last_byte_index) {
        output_.writer().close();
    }
}

uint64_t Reassembler::count_bytes_pending() const {
    uint64_t s = 0;
    for (const auto &pair : data_buffer) {
        s += pair.second.size();
    }
    return s;
}
