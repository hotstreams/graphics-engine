#include <limitless/serialization/distribution_serializer.hpp>

#include <limitless/fx/modules/distribution.hpp>
#include <limitless/util/bytebuffer.hpp>
#include <stdexcept>

using namespace Limitless;

template<typename T>
ByteBuffer DistributionSerializer::serialize(const Distribution<T>& distr) {
    ByteBuffer buffer;

    buffer << VERSION;

    buffer << distr.getType();

    switch (distr.getType()) {
        case DistributionType::Const:
            buffer << distr.get();
            break;
        case DistributionType::Range:
            buffer << static_cast<const RangeDistribution<T>&>(distr).getMin()
                   << static_cast<const RangeDistribution<T>&>(distr).getMax();
            break;
        case DistributionType::Curve:
            assert("TODO");
            break;
    }

    return buffer;
}

template<typename T>
std::unique_ptr<Distribution<T>> DistributionSerializer::deserialize(ByteBuffer& buffer) {
    uint8_t version {};

    buffer >> version;

    if (version != VERSION) {
        throw std::runtime_error("Wrong distribution serializer version! " + std::to_string(VERSION) + " vs " + std::to_string(version));
    }

    DistributionType type{};
    buffer >> type;

    std::unique_ptr<Distribution<T>> distribution;
    switch (type) {
        case DistributionType::Const: {
            T value{};
            buffer >> value;
            distribution = std::unique_ptr<Distribution<T>>(new ConstDistribution<T>(value));
            break;
        }
        case DistributionType::Range: {
            T min{};
            T max{};
            buffer >> min >> max;
            distribution = std::unique_ptr<Distribution<T>>(new RangeDistribution<T>(min, max));
            break;
        }
        case DistributionType::Curve:
            assert("TODO");
            break;
    }

    return distribution;
}

template<typename T>
ByteBuffer& Limitless::operator<<(ByteBuffer& buffer, const Distribution<T>& distr) {
    DistributionSerializer serializer;
    buffer << static_cast<const ByteBuffer&>(serializer.serialize<T>(distr));
    return buffer;
}

template<typename T>
ByteBuffer& Limitless::operator>>(ByteBuffer& buffer, std::unique_ptr<Distribution<T>>& distr) {
    DistributionSerializer serializer;
    distr = serializer.deserialize<T>(buffer);
    return buffer;
}

namespace Limitless {
    template ByteBuffer DistributionSerializer::serialize(const Distribution<glm::vec3>& distr);
    template ByteBuffer DistributionSerializer::serialize(const Distribution<glm::vec4>& distr);
    template ByteBuffer DistributionSerializer::serialize(const Distribution<float>& distr);
    template ByteBuffer DistributionSerializer::serialize(const Distribution<uint32_t>& distr);

    template ByteBuffer& operator<<(ByteBuffer& buffer, const Distribution<float>& distr);
    template ByteBuffer& operator<<(ByteBuffer& buffer, const Distribution<uint32_t>& distr);
    template ByteBuffer& operator<<(ByteBuffer& buffer, const Distribution<glm::vec3>& distr);
    template ByteBuffer& operator<<(ByteBuffer& buffer, const Distribution<glm::vec4>& distr);

    template ByteBuffer& operator>>(ByteBuffer& buffer, std::unique_ptr<Distribution<float>>& distr);
    template ByteBuffer& operator>>(ByteBuffer& buffer, std::unique_ptr<Distribution<uint32_t>>& distr);
    template ByteBuffer& operator>>(ByteBuffer& buffer, std::unique_ptr<Distribution<glm::vec3>>& distr);
    template ByteBuffer& operator>>(ByteBuffer& buffer, std::unique_ptr<Distribution<glm::vec4>>& distr);
}
