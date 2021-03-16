#pragma once

namespace jss {

    template <typename Key, typename Value> class ticket_map {
    public:
        bool empty() const noexcept {
            return true;
        }

        std::size_t size() const noexcept {
            return 0;
        }
    };
} // namespace jss
