
#include "FlightDataRecorder.h"

#include <cstdint>
#include <cstring>
#include <fstream>
#include <iterator>
#include <limits>
#include <vector>

namespace {

constexpr std::uint32_t VERSION = 1;
constexpr std::size_t HEADER_SIZE = 12;
constexpr std::size_t SAMPLE_SIZE = 65;
constexpr std::size_t CRC_SIZE = 4;
constexpr std::uint32_t MAX_SAMPLES = 1000000;

void writeU32(std::vector<std::uint8_t>& out,
              std::uint32_t value) {
    for (int i = 0; i < 4; ++i) {
        out.push_back(
            static_cast<std::uint8_t>(value >> (8 * i))
        );
    }
}

void writeU64(std::vector<std::uint8_t>& out,
              std::uint64_t value) {
    for (int i = 0; i < 8; ++i) {
        out.push_back(
            static_cast<std::uint8_t>(value >> (8 * i))
        );
    }
}

void writeDouble(std::vector<std::uint8_t>& out,
                 double value) {
    std::uint64_t bits;
    std::memcpy(&bits, &value, sizeof(bits));
    writeU64(out, bits);
}

std::uint32_t readU32(
    const std::vector<std::uint8_t>& data,
    std::size_t& pos
) {
    std::uint32_t value = 0;

    for (int i = 0; i < 4; ++i) {
        value |= static_cast<std::uint32_t>(
            data[pos++]
        ) << (8 * i);
    }

    return value;
}

std::uint64_t readU64(
    const std::vector<std::uint8_t>& data,
    std::size_t& pos
) {
    std::uint64_t value = 0;

    for (int i = 0; i < 8; ++i) {
        value |= static_cast<std::uint64_t>(
            data[pos++]
        ) << (8 * i);
    }

    return value;
}

double readDouble(
    const std::vector<std::uint8_t>& data,
    std::size_t& pos
) {
    std::uint64_t bits = readU64(data, pos);
    double value;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}

std::uint32_t crc32(
    const std::vector<std::uint8_t>& data,
    std::size_t length
) {
    std::uint32_t crc = 0xFFFFFFFFu;

    for (std::size_t i = 0; i < length; ++i) {
        crc ^= data[i];

        for (int bit = 0; bit < 8; ++bit) {
            crc = (crc >> 1) ^
                ((crc & 1u) ? 0xEDB88320u : 0u);
        }
    }

    return crc ^ 0xFFFFFFFFu;
}

void writeSample(
    std::vector<std::uint8_t>& out,
    const FlightSample& sample
) {
    writeDouble(out, sample.timestamp_sec);

    const auto& s = sample.state;

    writeDouble(out, s.altitude_ft);
    writeDouble(out, s.airspeed_kts);
    writeDouble(out, s.heading_deg);
    writeDouble(out, s.pitch_deg);
    writeDouble(out, s.roll_deg);
    writeDouble(out, s.vertical_speed_fpm);

    writeDouble(out, sample.measured_altitude_ft);

    out.push_back(
        sample.altimeter_fault_active ? 1 : 0
    );
}

FlightSample readSample(
    const std::vector<std::uint8_t>& data,
    std::size_t& pos
) {
    FlightSample sample{};

    sample.timestamp_sec = readDouble(data, pos);

    auto& s = sample.state;

    s.altitude_ft = readDouble(data, pos);
    s.airspeed_kts = readDouble(data, pos);
    s.heading_deg = readDouble(data, pos);
    s.pitch_deg = readDouble(data, pos);
    s.roll_deg = readDouble(data, pos);
    s.vertical_speed_fpm = readDouble(data, pos);

    sample.measured_altitude_ft =
        readDouble(data, pos);

    sample.altimeter_fault_active =
        data[pos++] == 1;

    return sample;
}

}

bool FlightDataRecorder::save(
    const std::string& filename,
    const std::vector<FlightSample>& samples
) {
    static_assert(sizeof(double) == 8,
                  "FDR requires 64-bit doubles");
    static_assert(std::numeric_limits<double>::is_iec559,
                  "FDR requires IEEE-754 doubles");

    if (samples.size() > MAX_SAMPLES) {
        return false;
    }

    std::vector<std::uint8_t> data;

    data.reserve(
        HEADER_SIZE + samples.size() * SAMPLE_SIZE + CRC_SIZE
    );

    data.push_back('A');
    data.push_back('E');
    data.push_back('G');
    data.push_back('F');

    writeU32(data, VERSION);
    writeU32(
        data,
        static_cast<std::uint32_t>(samples.size())
    );

    for (const auto& sample : samples) {
        writeSample(data, sample);
    }

    writeU32(data, crc32(data, data.size()));

    std::ofstream file(filename, std::ios::binary);

    if (!file) {
        return false;
    }

    file.write(
        reinterpret_cast<const char*>(data.data()),
        static_cast<std::streamsize>(data.size())
    );

    file.close();
    return file.good();
}

bool FlightDataRecorder::load(
    const std::string& filename,
    std::vector<FlightSample>& samples
) {
    std::ifstream file(filename, std::ios::binary);

    if (!file) {
        return false;
    }

    std::vector<std::uint8_t> data(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>()
    );

    if (file.bad() ||
        data.size() < HEADER_SIZE + CRC_SIZE) {
        return false;
    }

    if (data[0] != 'A' || data[1] != 'E' ||
        data[2] != 'G' || data[3] != 'F') {
        return false;
    }

    std::size_t pos = 4;

    const auto version = readU32(data, pos);
    const auto count = readU32(data, pos);

    if (version != VERSION || count > MAX_SAMPLES) {
        return false;
    }

    const std::size_t expectedSize =
        HEADER_SIZE +
        static_cast<std::size_t>(count) * SAMPLE_SIZE +
        CRC_SIZE;

    if (data.size() != expectedSize) {
        return false;
    }

    std::size_t crcPos = data.size() - CRC_SIZE;
    const auto storedCrc = readU32(data, crcPos);
    const auto calculatedCrc =
        crc32(data, data.size() - CRC_SIZE);

    if (storedCrc != calculatedCrc) {
        return false;
    }

    std::vector<FlightSample> loaded;
    loaded.reserve(count);

    for (std::uint32_t i = 0; i < count; ++i) {
        loaded.push_back(readSample(data, pos));
    }

    for (std::size_t offset = HEADER_SIZE + 64;
         offset < data.size() - CRC_SIZE;
         offset += SAMPLE_SIZE) {
        if (data[offset] > 1) {
            return false;
        }
    }

    samples = std::move(loaded);
    return true;
}
