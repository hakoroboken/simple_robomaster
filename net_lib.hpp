#include <cstddef>
#include <vector>
#include <cstdint>
#include "mbed.h"


#define fn auto

// deserialize func
template <typename T>
T deserialize(std::vector<uint8_t>& bytes) {
    static_assert(std::is_trivially_copyable<T>::value, "Data type is not trivially copyable");

    T data;
    std::memcpy(&data, bytes.data(), sizeof(data));
    return data;
}

struct motor_control_msg{
    std::uint8_t id;
    float motor_1;
    float motor_2;
    float motor_3;
    float motor_4;
};


inline fn set_motor(const float & power) -> int16_t{
    return (int16_t)(power * (float)30000);
}