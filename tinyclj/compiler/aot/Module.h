#include <string>
#include <unordered_set>

class Module {
protected:
    std::string m_Name;
    std::unordered_set<std::string> m_Imports;

    Module(std::string name,
           std::unordered_set<std::string> imports);

public:
    void emitImportsFile();
};
