#include <string>
#include <vector>
#include <stdexcept>
class RegisterAllocator {
    public:
        RegisterAllocator();
        std::string acquire();
        void release(const std::string& reg);

    private:
        std::vector<std::string> available_registers;
};
